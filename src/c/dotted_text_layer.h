#pragma once
#include <pebble.h>

// A text layer that renders characters individually using the pebble graphics api
// in a dotted style
typedef Layer DottedTextLayer;


typedef struct DottedTextLayerData {
    char *text;
    bool align_right;
    float scale_factor;
    bool auto_scale;
    GColor text_color;
} __attribute__((__packed__)) DottedTextLayerData;


// Use this method to create a DottedTextLayer
//
// @param bounds  The boundaries for the created layer
DottedTextLayer *dotted_text_layer_create(GRect bounds);


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
void dotted_text_layer_set_text(DottedTextLayer *dotted_text_layer, char *text);


// Use this method to set the text color for the layer
// 
// @param dotted_text_layer  the layer you want to set the color to
// @param color              the color to set
void dotted_text_layer_set_color(DottedTextLayer *dotted_text_layer, GColor color);

// Use this method to draw characters from the right instead of the left
//
// @param align_right  align to right if true, false otherwise
void dotted_text_layer_set_align_right(DottedTextLayer *dotted_text_layer, bool align_right);

// Use this method to set the scale factor for the layer.
// This scales dot size, dot gaps, and spacing between characters.
//
// @param dotted_text_layer  the layer you want to set the color to
// @param scale              the scale factor to set (> 0.0f)
void dotted_text_layer_set_scale_factor(DottedTextLayer *dotted_text_layer, float scale);

// Enable/disable automatic scale fitting to the layer height.
// When enabled, scale is computed from available vertical space.
void dotted_text_layer_set_auto_scale(DottedTextLayer *dotted_text_layer, bool enabled);


// Use this method to destroy a DottedTextLayer
//
// @param dotted_text_layer  The layer to destroy
void dotted_text_layer_destroy(DottedTextLayer *dotted_text_layer);
