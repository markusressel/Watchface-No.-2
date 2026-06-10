#include "dotted_text_layer.h"

#include "../graphics/pixel_matrix_drawer.h"
#include "../../settings/clay_settings.h"

static DottedTextLayerData *get_layer_data(const DottedTextLayer *dotted_text_layer) {
    return layer_get_data(dotted_text_layer);
}

static int scaled_dimension(int value, float scale_factor) {
    int scaled = (int) ((float) value * scale_factor + 0.5f);
    return scaled < 1 ? 1 : scaled;
}

static int scaled_non_negative_dimension(int value, float scale_factor) {
    int scaled = (int) ((float) value * scale_factor + 0.5f);
    return scaled < 0 ? 0 : scaled;
}

static int matrix_base_height(int dot_height, int gap_vertical) {
    return (5 * dot_height) + (4 * gap_vertical);
}

static float auto_scale_for_height(int dot_height, int gap_vertical, int available_height) {
    const int base_height = matrix_base_height(dot_height, gap_vertical);
    if (base_height <= 0 || available_height <= 0) {
        return 1.0f;
    }

    float scale = (float) available_height / (float) base_height;
    if (scale <= 0.0f) {
        return 1.0f;
    }

    // Iteratively reduce scale until the text fits.
    // This is to correct for rounding errors that can make the text too tall.
    for (int i = 0; i < 5; i++) {
        // Limit iterations to prevent infinite loops
        int scaled_h = scaled_dimension(dot_height, scale);
        int scaled_g = scaled_non_negative_dimension(gap_vertical, scale);
        int total_h = 5 * scaled_h + 4 * scaled_g;

        if (total_h <= available_height) {
            break; // It fits, we are done.
        }

        // It doesn't fit, reduce scale.
        // The ratio of heights is a good heuristic for reduction.
        scale *= (float) available_height / (float) total_h;
    }

    return scale;
}

// Returns the scale factor to use based on the layer data and available height.
static float get_scale_factor(DottedTextLayerData *data, int base_dot_height, int base_gap_vertical, int available_height) {
    if (!data->auto_scale) {
        return data->scale_factor;
    }

    if (data->cached_bounds_h == available_height &&
        data->cached_base_dot_height == base_dot_height &&
        data->cached_base_gap_vertical == base_gap_vertical) {
        return data->cached_scale;
    }

    float scale_factor = auto_scale_for_height(base_dot_height, base_gap_vertical, available_height);
    data->cached_bounds_h = available_height;
    data->cached_base_dot_height = base_dot_height;
    data->cached_base_gap_vertical = base_gap_vertical;
    data->cached_scale = scale_factor;

    return scale_factor;
}

static int text_width_in_pixels(
    const char *text,
    const unsigned int length,
    int dot_width,
    int gap_size_horizontal,
    int character_offset,
    int digit_size
) {
    if (!text) {
        return 0;
    }

    int width = 0;
    for (unsigned int i = 0; i < length; i++) {
        const int glyph_width = pixel_matrix_drawer_char_width(text[i], digit_size);
        width += glyph_width * dot_width;
        if (glyph_width > 1) {
            width += (glyph_width - 1) * gap_size_horizontal;
        }
        if (i + 1 < length) {
            width += character_offset;
        }
    }

    return width;
}

static void update_proc(DottedTextLayer *dotted_text_layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(dotted_text_layer);

    // get data associated with current layer
    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    if (!data->text) {
        return;
    }

    ClaySettings *settings = clay_get_settings();
    int base_dot_width = data->use_custom_metrics ? data->custom_dot_width : settings->DotWidth;
    int base_dot_height = data->use_custom_metrics ? data->custom_dot_height : settings->DotHeight;
    int base_gap_horizontal = data->use_custom_metrics ? data->custom_gap_horizontal : settings->DotHorizontalGap;
    int base_gap_vertical = data->use_custom_metrics ? data->custom_gap_vertical : settings->DotVerticalGap;

    float scale_factor = get_scale_factor(data, base_dot_height, base_gap_vertical, bounds.size.h);
    if (scale_factor <= 0.0f) {
        scale_factor = 1.0f;
    }

    int dot_width = scaled_dimension(base_dot_width, scale_factor);
    int dot_height = scaled_dimension(base_dot_height, scale_factor);
    int gap_size_horizontal = scaled_non_negative_dimension(base_gap_horizontal, scale_factor);
    int gap_size_vertical = scaled_non_negative_dimension(base_gap_vertical, scale_factor);
    const int digit_width = data->custom_digit_width > 0
                                ? data->custom_digit_width
                                : clay_get_settings()->DigitWidth;
    int character_offset;
    if (!data->character_offset_overridden) {
        character_offset = 2 * dot_width;
    } else if (data->character_offset_unit == DOTTED_TEXT_OFFSET_BLOCKS) {
        character_offset = data->character_offset_value * dot_width;
    } else {
        character_offset = scaled_non_negative_dimension(data->character_offset_value, scale_factor);
    }

    const int text_height = (5 * dot_height) + (4 * gap_size_vertical);
    int start_y = 0;
    if (data->vertical_alignment == VERTICAL_ALIGN_CENTER) {
        start_y = (bounds.size.h - text_height) / 2;
    } else if (data->vertical_alignment == VERTICAL_ALIGN_BOTTOM) {
        start_y = bounds.size.h - text_height;
    }
    if (start_y < 0) {
        start_y = 0;
    }

    const unsigned int length = strlen(data->text);
    const int text_width = text_width_in_pixels(
        data->text,
        length,
        dot_width,
        gap_size_horizontal,
        character_offset,
        digit_width
    );

    int current_start_x = 0;
    if (data->horizontal_alignment == HORIZONTAL_ALIGN_CENTER) {
        current_start_x = (bounds.size.w - text_width) / 2;
    } else if (data->horizontal_alignment == HORIZONTAL_ALIGN_RIGHT) {
        current_start_x = bounds.size.w - text_width;
    }
    if (current_start_x < 0) {
        current_start_x = 0;
    }

    for (unsigned int i = 0; i < length; i++) {
        char current_character = data->text[i];

        int pixelated_char_width = pixel_matrix_drawer_draw_char(
            ctx,
            GPoint(current_start_x, start_y),
            current_character,
            data->text_color,
            dot_width, dot_height,
            gap_size_horizontal, gap_size_vertical,
            false,
            digit_width
        );

        // APP_LOG(APP_LOG_LEVEL_DEBUG, "pxelated char width: %d", pixelated_char_width);

        current_start_x += pixelated_char_width * dot_width
                + ((pixelated_char_width - 1) * gap_size_horizontal);
        if (i + 1 < length) {
            current_start_x += character_offset;
        }
    }
}

DottedTextLayer *dotted_text_layer_create(GRect bounds) {
    // create the layer with an additional data section
    DottedTextLayer *dotted_text_layer = layer_create_with_data(bounds, sizeof(DottedTextLayerData));
    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    data->text = NULL;
    data->horizontal_alignment = HORIZONTAL_ALIGN_LEFT;
    data->vertical_alignment = VERTICAL_ALIGN_TOP;
    data->character_offset_overridden = false;
    data->character_offset_value = 0;
    data->character_offset_unit = DOTTED_TEXT_OFFSET_PIXELS;
    data->scale_factor = 1.0f;
    data->auto_scale = true;
    data->use_custom_metrics = false;
    data->custom_dot_width = 0;
    data->custom_dot_height = 0;
    data->custom_gap_horizontal = 0;
    data->custom_gap_vertical = 0;
    data->custom_digit_width = 0;
    data->text_color = GColorBlack;
    data->cached_bounds_h = -1;
    data->cached_base_dot_height = -1;
    data->cached_base_gap_vertical = -1;
    data->cached_scale = 1.0f;
    // connect with update method
    layer_set_update_proc(dotted_text_layer, update_proc);

    return dotted_text_layer;
}

void dotted_text_layer_set_text(DottedTextLayer *dotted_text_layer, char *text) {
    if (!dotted_text_layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
        return;
    }

    // get data associated with layer
    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    // free any existing old text
    if (data->text) {
        free(data->text);
    }
    // allocate memory for text
    data->text = malloc(sizeof(char) * strlen(text) + 1);
    // copy passed in text to struct
    strcpy(data->text, text);

    // mark dirty to trigger redraw
    layer_mark_dirty(dotted_text_layer);
}

void dotted_text_layer_set_horizontal_alignment(
    DottedTextLayer *dotted_text_layer,
    HorizontalAlignment alignment
) {
    if (!dotted_text_layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
        return;
    }

    if (alignment != HORIZONTAL_ALIGN_LEFT &&
        alignment != HORIZONTAL_ALIGN_CENTER &&
        alignment != HORIZONTAL_ALIGN_RIGHT) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid horizontal alignment!");
        return;
    }

    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    data->horizontal_alignment = alignment;

    layer_mark_dirty(dotted_text_layer);
}

void dotted_text_layer_set_vertical_alignment(
    DottedTextLayer *dotted_text_layer,
    VerticalAlignment alignment
) {
    if (!dotted_text_layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
        return;
    }

    if (alignment != VERTICAL_ALIGN_TOP &&
        alignment != VERTICAL_ALIGN_CENTER &&
        alignment != VERTICAL_ALIGN_BOTTOM) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid vertical alignment!");
        return;
    }

    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    data->vertical_alignment = alignment;
    layer_mark_dirty(dotted_text_layer);
}

void dotted_text_layer_set_color(DottedTextLayer *dotted_text_layer, GColor color) {
    if (!dotted_text_layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
        return;
    }

    // get data associated with layer
    DottedTextLayerData *data = get_layer_data(dotted_text_layer);

    // set the text color in struct
    data->text_color = color;

    // mark dirty to trigger redraw
    layer_mark_dirty(dotted_text_layer);
}

void dotted_text_layer_set_scale_factor(DottedTextLayer *dotted_text_layer, float scale_factor) {
    if (!dotted_text_layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
        return;
    }

    if (scale_factor <= 0.0f) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Scale factor needs to be > 0!");
        return;
    }

    // get data associated with layer
    DottedTextLayerData *data = get_layer_data(dotted_text_layer);

    // set the scale factor in struct
    data->scale_factor = scale_factor;
    data->auto_scale = false;

    // mark dirty to trigger redraw
    layer_mark_dirty(dotted_text_layer);
}

void dotted_text_layer_set_character_offset(
    DottedTextLayer *dotted_text_layer,
    const int value,
    const DottedTextOffsetUnit unit
) {
    if (!dotted_text_layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
        return;
    }

    if (value < 0) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Character offset needs to be >= 0!");
        return;
    }

    if (unit != DOTTED_TEXT_OFFSET_PIXELS && unit != DOTTED_TEXT_OFFSET_BLOCKS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid character offset unit!");
        return;
    }

    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    data->character_offset_overridden = true;
    data->character_offset_value = value;
    data->character_offset_unit = unit;
    layer_mark_dirty(dotted_text_layer);
}

void dotted_text_layer_set_auto_scale(DottedTextLayer *dotted_text_layer, bool enabled) {
    if (!dotted_text_layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
        return;
    }

    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    data->auto_scale = enabled;
    layer_mark_dirty(dotted_text_layer);
}

void dotted_text_layer_set_custom_metrics(
    DottedTextLayer *dotted_text_layer,
    const int dot_width,
    const int dot_height,
    const int gap_horizontal,
    const int gap_vertical
) {
    if (!dotted_text_layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
        return;
    }

    if (dot_width < 1 || dot_height < 1 || gap_horizontal < 0 || gap_vertical < 0) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Dot size must be >= 1 and gaps must be >= 0");
        return;
    }

    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    data->use_custom_metrics = true;
    data->custom_dot_width = dot_width;
    data->custom_dot_height = dot_height;
    data->custom_gap_horizontal = gap_horizontal;
    data->custom_gap_vertical = gap_vertical;
    layer_mark_dirty(dotted_text_layer);
}

void dotted_text_layer_set_digit_width(DottedTextLayer *dotted_text_layer, const int digit_width) {
    if (!dotted_text_layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
        return;
    }
    if (digit_width < 1) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Digit width must be >= 1");
        return;
    }

    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    data->custom_digit_width = digit_width;
    layer_mark_dirty(dotted_text_layer);
}

void dotted_text_layer_destroy(DottedTextLayer *dotted_text_layer) {
    if (!dotted_text_layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
        return;
    }

    layer_destroy(dotted_text_layer);
}