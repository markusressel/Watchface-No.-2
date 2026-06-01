#include "dotted_text_layer.h"
#include "pixel_matrix_drawer.h"
#include "clay_settings.h"

static DottedTextLayerData *get_layer_data(const DottedTextLayer *dotted_text_layer) {
    return layer_get_data(dotted_text_layer);
}

static int scaled_dimension(int value, float scale_factor) {
    int scaled = (int) ((float) value * scale_factor + 0.5f);
    return scaled < 1 ? 1 : scaled;
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
    return scale;
}

static void update_proc(DottedTextLayer *dotted_text_layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(dotted_text_layer);

    // get data associated with current layer
    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    if (!data->text) {
        return;
    }

    // set the fill color
    graphics_context_set_fill_color(ctx, data->text_color);

    ClaySettings *settings = clay_get_settings();
    int base_dot_width = data->use_custom_metrics ? data->custom_dot_width : settings->DotWidth;
    int base_dot_height = data->use_custom_metrics ? data->custom_dot_height : settings->DotHeight;
    int base_gap_horizontal = data->use_custom_metrics ? data->custom_gap_horizontal : settings->DotHorizontalGap;
    int base_gap_vertical = data->use_custom_metrics ? data->custom_gap_vertical : settings->DotVerticalGap;

    float scale_factor = data->auto_scale
                             ? auto_scale_for_height(base_dot_height, base_gap_vertical, bounds.size.h)
                             : data->scale_factor;
    if (scale_factor <= 0.0f) {
        scale_factor = 1.0f;
    }

    int dot_width = scaled_dimension(base_dot_width, scale_factor);
    int dot_height = scaled_dimension(base_dot_height, scale_factor);
    int gap_size_horizontal = scaled_dimension(base_gap_horizontal, scale_factor);
    int gap_size_vertical = scaled_dimension(base_gap_vertical, scale_factor);
    int character_offset;
    if (!data->character_offset_overridden) {
        character_offset = 2 * dot_width;
    } else if (data->character_offset_unit == DOTTED_TEXT_OFFSET_BLOCKS) {
        character_offset = data->character_offset_value * dot_width;
    } else {
        character_offset = scaled_dimension(data->character_offset_value, scale_factor);
    }

    int current_start_x;
    if (data->align_right) {
        current_start_x = bounds.size.w;
    } else {
        current_start_x = 0;
    }
    unsigned int length = strlen(data->text);
    for (unsigned int i = 0; i < length; i++) {
        char current_character;
        if (data->align_right) {
            current_character = data->text[length - 1 - i];
        } else {
            current_character = data->text[i];
        }

        // APP_LOG(APP_LOG_LEVEL_DEBUG, "drawing char: %c", current_character);

        int pixelated_char_width = pixel_matrix_drawer_draw_char(
            ctx,
            GPoint(current_start_x, 0),
            current_character,
            dot_width, dot_height,
            gap_size_horizontal, gap_size_vertical,
            data->align_right,
            data->solid_blocks,
            clay_get_settings()->DigitWidth
        );

        // APP_LOG(APP_LOG_LEVEL_DEBUG, "pxelated char width: %d", pixelated_char_width);

        if (data->align_right) {
            current_start_x -= pixelated_char_width * dot_width
                    + ((pixelated_char_width - 1) * gap_size_horizontal)
                    + character_offset;
        } else {
            current_start_x += pixelated_char_width * dot_width
                    + ((pixelated_char_width - 1) * gap_size_horizontal)
                    + character_offset;
        }
    }
}

DottedTextLayer *dotted_text_layer_create(GRect bounds) {
    // create the layer with an additional data section
    DottedTextLayer *dotted_text_layer = layer_create_with_data(bounds, sizeof(DottedTextLayerData));
    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    data->text = NULL;
    data->align_right = false;
    data->character_offset_overridden = false;
    data->character_offset_value = 0;
    data->character_offset_unit = DOTTED_TEXT_OFFSET_PIXELS;
    data->scale_factor = 1.0f;
    data->auto_scale = true;
    data->solid_blocks = false;
    data->use_custom_metrics = false;
    data->custom_dot_width = 0;
    data->custom_dot_height = 0;
    data->custom_gap_horizontal = 0;
    data->custom_gap_vertical = 0;
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
    }
    // allocate memory for text
    data->text = malloc(sizeof(char) * strlen(text) + 1);
    // copy passed in text to struct
    strcpy(data->text, text);

    // mark dirty to trigger redraw
    layer_mark_dirty(dotted_text_layer);
}

void dotted_text_layer_set_align_right(DottedTextLayer *dotted_text_layer, bool align_right) {
    if (!dotted_text_layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
        return;
    }

    // get data associated with layer
    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    data->align_right = align_right;

    // mark dirty to trigger redraw
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

void dotted_text_layer_set_solid_blocks(DottedTextLayer *dotted_text_layer, bool enabled) {
    if (!dotted_text_layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
        return;
    }

    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    data->solid_blocks = enabled;
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

    if (dot_width < 1 || dot_height < 1 || gap_horizontal < 1 || gap_vertical < 1) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Custom metrics must be >= 1");
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

void dotted_text_layer_destroy(DottedTextLayer *dotted_text_layer) {
    if (!dotted_text_layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
        return;
    }

    // Should be not needed if data is embedded in the layer itslef
    /*
  // free stored text
  if (dotted_text_layer->text) {
    free(dotted_text_layer->text);
  }
  // destroy base layer
  layer_destroy(dotted_text_layer->layer);
  */

    layer_destroy(dotted_text_layer);
}
