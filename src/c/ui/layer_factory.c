#include "layer_factory.h"
#include "../developer_options.h"

static void debug_border_update_proc(Layer *layer, GContext *ctx) {
    graphics_context_set_stroke_color(ctx, GColorRed);
    graphics_draw_rect(ctx, layer_get_bounds(layer));
}

static void maybe_add_debug_border(Layer *parent_layer) {
    if (!DEV_OPTIONS.ShowLayerBounds) {
        return;
    }

    Layer *border = layer_create(layer_get_bounds(parent_layer));
    layer_set_update_proc(border, debug_border_update_proc);
    layer_add_child(parent_layer, border);
}

LayerBuilder layer_builder(Layer *parent, LayerLayout layout) {
    GRect parent_bounds = layer_get_bounds(parent);
    return (LayerBuilder)
    {
        .
        parent = parent,
        .
        bounds = GRect(
            layout.x,
            layout.y,
            parent_bounds.size.w - layout.width_margin,
            layout.height
        ),
    };
}

LayerBuilder layer_builder_from_rect(Layer *parent, GRect bounds) {
    return (LayerBuilder)
    {
        .
        parent = parent,
        .
        bounds = bounds,
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

    maybe_add_debug_border(text_layer_get_layer(layer));
    layer_add_child(builder.parent, text_layer_get_layer(layer));

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

    maybe_add_debug_border(layer);
    layer_add_child(builder.parent, layer);

    return layer;
}

Layer *layer_factory_create_custom_layer(
    LayerBuilder builder,
    LayerUpdateProc update_proc
) {
    Layer *layer = layer_create(builder.bounds);
    layer_set_update_proc(layer, update_proc);
    maybe_add_debug_border(layer);
    layer_add_child(builder.parent, layer);

    return layer;
}