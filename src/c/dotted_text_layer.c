#include "dotted_text_layer.h"
#include "pixel_matrix_drawer.h"


static DottedTextLayerData* get_layer_data(DottedTextLayer* dotted_text_layer) {
  return (DottedTextLayerData*) layer_get_data(dotted_text_layer);
}

static void update_proc(DottedTextLayer *dotted_text_layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(dotted_text_layer);
  
  // get data associated with current layer
  DottedTextLayerData *data = get_layer_data(dotted_text_layer);
  
  // set the fill color
  graphics_context_set_fill_color(ctx, data->text_color);
  
  // offset in pixel between two characters
  int character_offset = 2;
  
  // dot size
  int dot_width = 3;
  int dot_height = 3;
  
  int gap_size_horizontal = 3;
  int gap_size_vertical = 3;
  
  int current_start_x;
  if (data->align_right) {
    current_start_x = bounds.size.w;
  } else {
    current_start_x = 0;
  }
  unsigned int length = strlen(data->text);
  for(unsigned int i = 0; i < length; i++) {
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
      data->align_right);
    
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

DottedTextLayer* dotted_text_layer_create(GRect bounds) {
  // create the layer with an additional data section
  DottedTextLayer* dotted_text_layer = layer_create_with_data(bounds, sizeof(DottedTextLayerData));
  // connect with update method
  layer_set_update_proc(dotted_text_layer, update_proc);
  
  return dotted_text_layer;
}

void dotted_text_layer_set_text(DottedTextLayer* dotted_text_layer, char* text) {
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

void dotted_text_layer_set_align_right(DottedTextLayer* dotted_text_layer, bool align_right) {
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

void dotted_text_layer_set_color(DottedTextLayer* dotted_text_layer, GColor color) {
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

void dotted_text_layer_set_scale_factor(DottedTextLayer* dotted_text_layer, int scale_factor) {
  if (!dotted_text_layer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
    return;
  }
  
  if (scale_factor < 1) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Scale factor needs to be >= 1!");
    return;
  }
  
  // get data associated with layer
  DottedTextLayerData *data = get_layer_data(dotted_text_layer);
  
  // set the scale factor in struct
  data->scale_factor = scale_factor;
  
  // mark dirty to trigger redraw
  layer_mark_dirty(dotted_text_layer);
}

void dotted_text_layer_destroy(DottedTextLayer* dotted_text_layer) {
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