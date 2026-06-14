#include "dotted_text_layer.h"

#include "../graphics/pixel_matrix_drawer.h"
#include "../../settings/clay_settings.h"

static DottedTextLayerData *get_layer_data(const DottedTextLayer *dotted_text_layer) {
    return layer_get_data(dotted_text_layer);
}

static int matrix_base_height(int dot_height, int gap_vertical) {
    return (5 * dot_height) + (4 * gap_vertical);
}

static float auto_scale_for_height(
    int dot_height,
    int gap_vertical,
    int available_height,
    DottedTextRenderingMode mode
) {
    const int base_height = matrix_base_height(dot_height, gap_vertical);
    if (base_height <= 0 || available_height <= 0) {
        return 1.0f;
    }

    float scale = (float) available_height / (float) base_height;
    if (scale <= 0.0f) {
        return 1.0f;
    }

    if (mode == DOTTED_TEXT_RENDERING_MODE_PIXEL_PERFECT) {
        // Calculate the total height with current scale using rounding.
        int scaled_h = (int) ((float) dot_height * scale + 0.5f);
        int scaled_g = (int) ((float) gap_vertical * scale + 0.5f);
        int total_h = 5 * scaled_h + 4 * scaled_g;

        if (total_h > available_height) {
            // If rounded values exceed available height, apply a safe algebraic lower bound.
            // The maximum possible rounding error for 9 segments is 4.5 pixels.
            scale = (float) (available_height - 4.5f) / (float) base_height;
            if (scale <= 0.0f) {
                scale = 0.1f; // Minimum fallback scale
            }
        }
    }

    return scale;
}

// Returns the scale factor to use based on the layer data and available height.
static float get_scale_factor(
    DottedTextLayerData *data,
    int base_dot_height,
    int base_gap_vertical,
    int available_height
) {
    if (!data->auto_scale) {
        return data->scale_factor;
    }

    return auto_scale_for_height(
        base_dot_height,
        base_gap_vertical,
        available_height,
        data->rendering_mode
    );
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

    float dot_width = (float) base_dot_width * scale_factor;
    float dot_height = (float) base_dot_height * scale_factor;
    float gap_size_horizontal = (float) base_gap_horizontal * scale_factor;
    float gap_size_vertical = (float) base_gap_vertical * scale_factor;

    if (data->rendering_mode == DOTTED_TEXT_RENDERING_MODE_PIXEL_PERFECT) {
        dot_width = (float) ((int) (dot_width + 0.5f));
        dot_height = (float) ((int) (dot_height + 0.5f));
        gap_size_horizontal = (float) ((int) (gap_size_horizontal + 0.5f));
        gap_size_vertical = (float) ((int) (gap_size_vertical + 0.5f));
    }

    const int digit_width = data->custom_digit_width > 0
                                ? data->custom_digit_width
                                : clay_get_settings()->DigitWidth;

    float character_offset;
    if (!data->character_offset_overridden) {
        character_offset = 2.0f * dot_width;
    } else if (data->character_offset_unit == DOTTED_TEXT_OFFSET_UNIT_BLOCKS) {
        character_offset = (float) data->character_offset_value * dot_width;
    } else {
        character_offset = (float) data->character_offset_value * scale_factor;
        if (data->rendering_mode == DOTTED_TEXT_RENDERING_MODE_PIXEL_PERFECT) {
            character_offset = (float) ((int) (character_offset + 0.5f));
        }
    }

    const float text_height = (5.0f * dot_height) + (4.0f * gap_size_vertical);
    float start_y = 0;
    if (data->vertical_alignment == VERTICAL_ALIGN_CENTER) {
        start_y = ((float) bounds.size.h - text_height) / 2.0f;
    } else if (data->vertical_alignment == VERTICAL_ALIGN_BOTTOM) {
        start_y = (float) bounds.size.h - text_height;
    }
    if (start_y < 0) {
        start_y = 0;
    }

    const unsigned int length = strlen(data->text);

    // Calculate width in pixels using float math
    float text_width = 0;
    for (unsigned int i = 0; i < length; i++) {
        const int glyph_width = pixel_matrix_drawer_char_width(data->text[i], digit_width);
        text_width += (float) glyph_width * dot_width;
        if (glyph_width > 1) {
            text_width += (float) (glyph_width - 1) * gap_size_horizontal;
        }
        if (i + 1 < length) {
            text_width += character_offset;
        }
    }

    float current_start_x = 0;
    if (data->horizontal_alignment == HORIZONTAL_ALIGN_CENTER) {
        current_start_x = ((float) bounds.size.w - text_width) / 2.0f;
    } else if (data->horizontal_alignment == HORIZONTAL_ALIGN_RIGHT) {
        current_start_x = (float) bounds.size.w - text_width;
    }
    if (current_start_x < 0) {
        current_start_x = 0;
    }

    for (unsigned int i = 0; i < length; i++) {
        char current_character = data->text[i];

        int pixelated_char_width = pixel_matrix_drawer_draw_char(
            ctx,
            current_start_x, start_y,
            current_character,
            data->text_color,
            dot_width, dot_height,
            gap_size_horizontal, gap_size_vertical,
            false,
            digit_width,
            data->rendering_mode
        );

        current_start_x += (float) pixelated_char_width * dot_width
                + ((float) (pixelated_char_width - 1) * gap_size_horizontal);
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
    data->rendering_mode = DOTTED_TEXT_RENDERING_MODE_PIXEL_PERFECT;
    data->character_offset_overridden = false;

    data->character_offset_value = 0;
    data->character_offset_unit = DOTTED_TEXT_OFFSET_UNIT_PIXELS;
    data->scale_factor = 1.0f;
    data->auto_scale = true;
    data->use_custom_metrics = false;
    data->custom_dot_width = 0;
    data->custom_dot_height = 0;
    data->custom_gap_horizontal = 0;
    data->custom_gap_vertical = 0;
    data->custom_digit_width = 0;
    data->text_color = GColorBlack;

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
        data->text = NULL;
    }

    if (!text) {
        layer_mark_dirty(dotted_text_layer);
        return;
    }

    // allocate memory for text
    data->text = malloc(sizeof(char) * strlen(text) + 1);
    if (!data->text) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to allocate memory for text!");
        return;
    }
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

void dotted_text_layer_set_rendering_mode(
    DottedTextLayer *dotted_text_layer,
    DottedTextRenderingMode mode
) {
    if (!dotted_text_layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
        return;
    }

    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    data->rendering_mode = mode;
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

    if (unit != DOTTED_TEXT_OFFSET_UNIT_PIXELS && unit != DOTTED_TEXT_OFFSET_UNIT_BLOCKS) {
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

int dotted_text_layer_get_content_width(DottedTextLayer *dotted_text_layer) {
    if (!dotted_text_layer) {
        return 0;
    }

    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    if (!data->text) {
        return 0;
    }

    GRect bounds = layer_get_bounds(dotted_text_layer);
    ClaySettings *settings = clay_get_settings();
    int base_dot_width = data->use_custom_metrics ? data->custom_dot_width : settings->DotWidth;
    int base_dot_height = data->use_custom_metrics ? data->custom_dot_height : settings->DotHeight;
    int base_gap_horizontal = data->use_custom_metrics ? data->custom_gap_horizontal : settings->DotHorizontalGap;
    int base_gap_vertical = data->use_custom_metrics ? data->custom_gap_vertical : settings->DotVerticalGap;

    float scale_factor = get_scale_factor(data, base_dot_height, base_gap_vertical, bounds.size.h);
    if (scale_factor <= 0.0f) {
        scale_factor = 1.0f;
    }

    float dot_width = (float) base_dot_width * scale_factor;
    float dot_height = (float) base_dot_height * scale_factor;
    float gap_size_horizontal = (float) base_gap_horizontal * scale_factor;
    float gap_size_vertical = (float) base_gap_vertical * scale_factor;

    if (data->rendering_mode == DOTTED_TEXT_RENDERING_MODE_PIXEL_PERFECT) {
        dot_width = (float) ((int) (dot_width + 0.5f));
        dot_height = (float) ((int) (dot_height + 0.5f));
        gap_size_horizontal = (float) ((int) (gap_size_horizontal + 0.5f));
        gap_size_vertical = (float) ((int) (gap_size_vertical + 0.5f));
    }

    const int digit_width = data->custom_digit_width > 0
                                ? data->custom_digit_width
                                : clay_get_settings()->DigitWidth;

    float character_offset;
    if (!data->character_offset_overridden) {
        character_offset = 2.0f * dot_width;
    } else if (data->character_offset_unit == DOTTED_TEXT_OFFSET_UNIT_BLOCKS) {
        character_offset = (float) data->character_offset_value * dot_width;
    } else {
        character_offset = (float) data->character_offset_value * scale_factor;
        if (data->rendering_mode == DOTTED_TEXT_RENDERING_MODE_PIXEL_PERFECT) {
            character_offset = (float) ((int) (character_offset + 0.5f));
        }
    }


    float width = 0;
    const unsigned int length = strlen(data->text);
    for (unsigned int i = 0; i < length; i++) {
        const int glyph_width = pixel_matrix_drawer_char_width(data->text[i], digit_width);
        width += (float) glyph_width * dot_width;
        if (glyph_width > 1) {
            width += (float) (glyph_width - 1) * gap_size_horizontal;
        }
        if (i + 1 < length) {
            width += character_offset;
        }
    }

    return (int) (width + 0.5f);
}

void dotted_text_layer_destroy(DottedTextLayer *dotted_text_layer) {
    if (!dotted_text_layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
        return;
    }

    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    if (data->text) {
        free(data->text);
        data->text = NULL;
    }

    layer_destroy(dotted_text_layer);
}
