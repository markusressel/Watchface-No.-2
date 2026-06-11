#include "layer_factory.h"
#include "../developer_options.h"
#include "layer/debug_layer.h"
#include "layer/dotted_text_layer.h"

LayerBuilder layer_builder(Layer *parent, LayerLayout layout) {
    GRect parent_bounds = layer_get_bounds(parent);
    return (LayerBuilder){
        .parent = parent,
        .bounds = GRect(
            layout.x,
            layout.y,
            parent_bounds.size.w - layout.width_margin,
            layout.height
        ),
    };
}

LayerBuilder layer_builder_from_rect(Layer *parent, GRect bounds) {
    return (LayerBuilder){
        .parent = parent,
        .bounds = bounds,
    };
}

TextLayer *layer_factory_create_text_layer(
    LayerBuilder builder,
    TextLayerStyle style
) {
    TextLayer *layer = text_layer_create(builder.bounds);

    text_layer_set_background_color(layer, style.background_color);
    text_layer_set_text_color(layer, style.text_color);
    text_layer_set_font(layer, style.font);
    text_layer_set_text_alignment(layer, style.alignment);

    layer_add_child(builder.parent, text_layer_get_layer(layer));

    if (DEV_OPTIONS.ShowLayerBounds) {
        debug_layer_add_border(text_layer_get_layer(layer));
    }

    return layer;
}

DottedTextLayer *layer_factory_create_dotted_text_layer(
    LayerBuilder builder,
    GColor text_color,
    HorizontalAlignment horizontal_alignment,
    VerticalAlignment vertical_alignment,
    const char *initial_text
) {
    DottedTextLayer *layer = dotted_text_layer_create(builder.bounds);

    dotted_text_layer_set_auto_scale(layer, true);
    dotted_text_layer_set_color(layer, text_color);
    dotted_text_layer_set_horizontal_alignment(layer, horizontal_alignment);
    dotted_text_layer_set_vertical_alignment(layer, vertical_alignment);
    if (initial_text) {
        dotted_text_layer_set_text(layer, (char *) initial_text);
    }

    layer_add_child(builder.parent, layer);

    if (DEV_OPTIONS.ShowLayerBounds) {
        debug_layer_add_border(layer);
    }

    return layer;
}

Layer *layer_factory_create_custom_layer(
    LayerBuilder builder,
    LayerUpdateProc update_proc
) {
    return layer_factory_create_custom_layer_with_data(builder, update_proc, 0);
}

Layer *layer_factory_create_custom_layer_with_data(
    LayerBuilder builder,
    LayerUpdateProc update_proc,
    size_t data_size
) {
    Layer *layer = data_size > 0 ? layer_create_with_data(builder.bounds, data_size) : layer_create(builder.bounds);
    layer_set_update_proc(layer, update_proc);
    layer_add_child(builder.parent, layer);

    if (DEV_OPTIONS.ShowLayerBounds) {
        debug_layer_add_border(layer);
    }

    return layer;
}
