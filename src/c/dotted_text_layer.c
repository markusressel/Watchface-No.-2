#include "dotted_text_layer.h"
#include "pixel_matrix_drawer.h"
#include "clay_settings.h"

static DottedTextLayerData *get_layer_data(const DottedTextLayer *dotted_text_layer) {
    return layer_get_data(dotted_text_layer);
}

static int scaled_dimension(int value, float scale_factor) {
    int scaled = (int) (value * scale_factor + 0.5f);
    return scaled < 1 ? 1 : scaled;
}

static int required_content_height(float scale_factor) {
    ClaySettings *settings = clay_get_settings();
    int dot_height = scaled_dimension(settings->DotHeight, scale_factor);
    int gap_size_vertical = scaled_dimension(settings->DotVerticalGap, scale_factor);
    return (5 * dot_height) + (4 * gap_size_vertical);
}

static void apply_scaled_frame(DottedTextLayer *dotted_text_layer, float scale_factor) {
    GRect frame = layer_get_frame(dotted_text_layer);
    int required_height = required_content_height(scale_factor);
    if (frame.size.h == required_height) {
        return;
    }

    int center_y = frame.origin.y + frame.size.h / 2;
    frame.size.h = required_height;
    frame.origin.y = center_y - frame.size.h / 2;
    layer_set_frame(dotted_text_layer, frame);
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

    // offset in pixel between two characters
    int character_offset = 2;

    float scale_factor = data->scale_factor;
    if (scale_factor <= 0.0f) {
        scale_factor = 1.0f;
    }

    ClaySettings *settings = clay_get_settings();
    int dot_width = scaled_dimension(settings->DotWidth, scale_factor);
    int dot_height = scaled_dimension(settings->DotHeight, scale_factor);
    int gap_size_horizontal = scaled_dimension(settings->DotHorizontalGap, scale_factor);
    int gap_size_vertical = scaled_dimension(settings->DotVerticalGap, scale_factor);

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
            clay_get_settings()->DigitWidth
        );

        // APP_LOG(APP_LOG_LEVEL_DEBUG, "pxelated char width: %d", pixelated_char_width);

        if (data->align_right) {
            current_start_x -= pixelated_char_width * dot_width
                    + ((pixelated_char_width - 1) * gap_size_horizontal)
                    + (character_offset * gap_size_horizontal);
        } else {
            current_start_x += pixelated_char_width * dot_width
                    + ((pixelated_char_width - 1) * gap_size_horizontal)
                    + (character_offset * gap_size_horizontal);
        }
    }
}

DottedTextLayer *dotted_text_layer_create(GRect bounds) {
    // create the layer with an additional data section
    DottedTextLayer *dotted_text_layer = layer_create_with_data(bounds, sizeof(DottedTextLayerData));
    DottedTextLayerData *data = get_layer_data(dotted_text_layer);
    data->text = NULL;
    data->align_right = false;
    data->scale_factor = 1.0f;
    data->text_color = GColorBlack;
    // connect with update method
    layer_set_update_proc(dotted_text_layer, update_proc);
    apply_scaled_frame(dotted_text_layer, data->scale_factor);

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
    apply_scaled_frame(dotted_text_layer, data->scale_factor);

    // mark dirty to trigger redraw
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
