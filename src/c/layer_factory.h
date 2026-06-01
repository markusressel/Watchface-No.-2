#pragma once

#include <pebble.h>
#include "dotted_text_layer.h"

typedef struct TextLayerStyle {
    GColor background_color;
    GColor text_color;
    GFont font;
    GTextAlignment alignment;
} TextLayerStyle;

typedef struct LayerLayout {
    int x;
    int y;
    int width_margin;
    int height;
} LayerLayout;

typedef struct LayerBuilder {
    Layer *parent;
    GRect bounds;
} LayerBuilder;

LayerBuilder layer_builder(Layer *parent, LayerLayout layout);

// Construct a LayerBuilder directly from a pre-computed GRect (e.g. from the
// layout system which already knows the exact screen position).
LayerBuilder layer_builder_from_rect(Layer *parent, GRect bounds);

TextLayer *layer_factory_create_text_layer(
    LayerBuilder builder,
    TextLayerStyle style
);

DottedTextLayer *layer_factory_create_dotted_text_layer(
    LayerBuilder builder,
    GColor text_color,
    bool align_right,
    const char *initial_text
);

Layer *layer_factory_create_custom_layer(
    LayerBuilder builder,
    LayerUpdateProc update_proc
);