#pragma once

#include <pebble.h>
#include "layer/dotted_text_layer.h"

typedef struct {
    int x;
    int y;
    int width_margin;
    int height;
} LayerLayout;

typedef struct {
    Layer *parent;
    GRect bounds;
} LayerBuilder;

typedef struct {
    GColor background_color;
    GColor text_color;
    GFont font;
    GTextAlignment alignment;
} TextLayerStyle;

LayerBuilder layer_builder(Layer *parent, LayerLayout layout);

LayerBuilder layer_builder_from_rect(Layer *parent, GRect bounds);

TextLayer *layer_factory_create_text_layer(
    LayerBuilder builder,
    TextLayerStyle style
);

DottedTextLayer *layer_factory_create_dotted_text_layer(
    LayerBuilder builder,
    GColor text_color,
    HorizontalAlignment horizontal_alignment,
    VerticalAlignment vertical_alignment,
    const char *initial_text
);

Layer *layer_factory_create_custom_layer(
    LayerBuilder builder,
    LayerUpdateProc update_proc
);

Layer *layer_factory_create_custom_layer_with_data(
    LayerBuilder builder,
    LayerUpdateProc update_proc,
    size_t data_size
);
