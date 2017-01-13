#pragma once
#include <pebble.h>

// A text layer that renders characters individually using the pebble graphics api
// in a dotted style
typedef Layer DottedTextLayer;


typedef struct DottedTextLayerData {
  char* text;
  int scale_factor;
  GColor text_color;
} __attribute__((__packed__)) DottedTextLayerData;


// Use this method to create a DottedTextLayer
//
// @param bounds  The boundaries for the created layer
DottedTextLayer* dotted_text_layer_create(GRect bounds);


// Use this method to get the base layer of the given DottedTextLayer
//
// @param dotted_text_layer  The DottedTextLayer to get the base layer from
// Layer dotted_text_layer_get_layer(DottedTextLayer* dotted_text_layer);


// Use this method to set the text for the layer
// 
// Keep in mind that this layer only supports a small subset of all characters
// and they are drawn manually using graphics api, not like a font.
//
// @param dotted_text_layer  the layer you want to set the text to
// @param text               the text to set
void dotted_text_layer_set_text(DottedTextLayer* dotted_text_layer, char* text);


// Use this method to set the text color for the layer
// 
// @param dotted_text_layer  the layer you want to set the color to
// @param color              the color to set
void dotted_text_layer_set_color(DottedTextLayer* dotted_text_layer, GColor color);


// Use this method to set the scale factor for the layer
// A scale factor of 1 means 1x1 pixel drawn, 1x1 pixel not drawn and so on
// A scale factor of 2 means 2x2 pixel drawn, 2x2 pixel not drwan and so on
//
// So the scale factor applies to both width and height. Seperate scaling might possible in a future release.
// 
// @param dotted_text_layer  the layer you want to set the color to
// @param scale              the scale factor to set (>= 1)
void dotted_text_layer_set_scale_factor(DottedTextLayer* dotted_text_layer, int scale);


// Use this method to destroy a DottedTextLayer
//
// @param dotted_text_layer  The layer to destroy
void dotted_text_layer_destroy(DottedTextLayer* dotted_text_layer);