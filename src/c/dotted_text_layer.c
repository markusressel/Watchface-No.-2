#include "dotted_text_layer.h"


static DottedTextLayerData* get_layer_data(DottedTextLayer* dotted_text_layer) {
  return (DottedTextLayerData*) layer_get_data(dotted_text_layer);
}

static void update_proc(DottedTextLayer *dotted_text_layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(dotted_text_layer);
  
  // get data associated with current layer
  DottedTextLayerData *data = get_layer_data(dotted_text_layer);
  
  // set the fill color
  graphics_context_set_fill_color(ctx, data->text_color);
  // fill layer completely (for now)
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

DottedTextLayer* dotted_text_layer_create(GRect bounds) {
  // create the layer with an additional data section
  DottedTextLayer* dotted_text_layer = layer_create_with_data(bounds, sizeof(DottedTextLayerData));
  layer_set_update_proc(dotted_text_layer, update_proc);
  
  return dotted_text_layer;
}

// Is included by type definition
/*
Layer dotted_text_layer_get_layer(DottedTextLayer* dotted_text_layer) {
  DottedTextLayerData ProgressData *data = (ProgressData *)layer_get_data(bar);
  
  if (!dotted_text_layer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
    return NULL;
  }
  
  return dotted_text_layer->layer;
}
*/

void dotted_text_layer_set_text(DottedTextLayer* dotted_text_layer, char* text) {
  if (!dotted_text_layer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "DottedTextLayer is NULL!");
    return;
  }
  
  /*
  // free any existing old text
  if (dotted_text_layer->text) {
    free(dotted_text_layer->text);
  }
  
  // allocate memory for text
  dotted_text_layer->text = malloc(sizeof(char) * strlen(text) + 1);
  // copy passed in text to struct
  strcpy(dotted_text_layer->text, text);
  */
  
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