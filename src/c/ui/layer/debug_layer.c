#include "debug_layer.h"

#define MAX_DEBUG_BORDERS 20
static Layer* s_debug_borders[MAX_DEBUG_BORDERS];
static int s_debug_border_count = 0;

static void debug_border_update_proc(Layer *layer, GContext *ctx) {
    graphics_context_set_stroke_color(ctx, GColorRed);
    graphics_draw_rect(ctx, layer_get_bounds(layer));
}

void debug_layer_add_border(Layer *layer) {
    if (s_debug_border_count >= MAX_DEBUG_BORDERS) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "Max debug borders reached.");
        return;
    }

    // A border layer that draws exactly over the target layer's bounds
    Layer *border = layer_create(layer_get_bounds(layer));
    layer_set_update_proc(border, debug_border_update_proc);

    // Add the border as a child of the target layer so it draws on top
    layer_add_child(layer, border);

    s_debug_borders[s_debug_border_count++] = border;
}

void debug_layer_destroy_all_borders(void) {
    for (int i = 0; i < s_debug_border_count; i++) {
        if (s_debug_borders[i] != NULL) {
            layer_destroy(s_debug_borders[i]);
            s_debug_borders[i] = NULL;
        }
    }
    s_debug_border_count = 0;
}
