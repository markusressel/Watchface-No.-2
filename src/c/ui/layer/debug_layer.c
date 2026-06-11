#include "debug_layer.h"

static void debug_border_update_proc(Layer *layer, GContext *ctx) {
    graphics_context_set_stroke_color(ctx, GColorRed);
    graphics_draw_rect(ctx, layer_get_bounds(layer));
}

void debug_layer_add_border(Layer *layer) {
    // A border layer that draws exactly over the target layer's bounds
    Layer *border = layer_create(layer_get_bounds(layer));
    layer_set_update_proc(border, debug_border_update_proc);

    // Add the border as a child of the target layer so it draws on top
    layer_add_child(layer, border);
}

void debug_layer_destroy_all_borders(void) {
    // No-op, borders are automatically destroyed when their parent layers are destroyed.
}
