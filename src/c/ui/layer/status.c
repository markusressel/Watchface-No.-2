#include "status.h"

#include "../layer_factory.h"
#include "../../settings/clay_settings.h"
#include "../../system/phone_connection.h"

#define STATUS_SEGMENTS 3
#define STATUS_PADDING 2

#define STATUS_SEGMENT_PHONE_CONNECTION 1

static Layer *s_status_layer = NULL;

static GColor status_layer_determine_segment_color(int i) {
    // Center segment (index 1) is used for "disconnected from phone" status.
    if (i == STATUS_SEGMENT_PHONE_CONNECTION) {
        bool connected = phone_connection_is_connected();
        if (!connected) {
            return GColorRed;
        }
    }

    return GColorClear;
}

void status_layer_draw_segment(GContext *ctx, int i, int dot_width, int segment_height, GColor color) {
    graphics_context_set_fill_color(ctx, color);
    const int y = i * (segment_height + STATUS_PADDING);
    // Draw a solid line with dot_width
    GRect segment_bounds = GRect(0, y, dot_width, segment_height);
    graphics_fill_rect(ctx, segment_bounds, 0, GCornerNone);
}

static void status_layer_update_proc(Layer *layer, GContext *ctx) {
    ClaySettings *settings = clay_get_settings();
    float scale = settings->DotAutoScale ? settings->DotScaleFactor : 1.0f;
    int dot_width = (int) (settings->DotWidth * scale + 0.5f);
    if (dot_width < 1) dot_width = 1;

    GRect bounds = layer_get_bounds(layer);

    // Total height available for segments after accounting for padding
    const int available_height = bounds.size.h - (STATUS_PADDING * (STATUS_SEGMENTS - 1));
    const int segment_height = available_height / STATUS_SEGMENTS;

    for (int i = 0; i < STATUS_SEGMENTS; i++) {
        GColor color = status_layer_determine_segment_color(i);
        if (!gcolor_equal(color, GColorClear)) {
            status_layer_draw_segment(ctx, i, dot_width, segment_height, color);
        }
    }
}

Layer *status_layer_create(Layer *parent_layer) {
    GRect parent_bounds = layer_get_bounds(parent_layer);

    ClaySettings *settings = clay_get_settings();
    float scale = settings->DotAutoScale ? settings->DotScaleFactor : 1.0f;
    int dot_width = (int) (settings->DotWidth * scale + 0.5f);
    if (dot_width < 1) dot_width = 1;

    // Optimized Bounds: only as wide as the dot_width
    GRect bounds = GRect(0, 0, dot_width, parent_bounds.size.h);

    LayerBuilder builder = (LayerBuilder){
        .parent = parent_layer,
        .bounds = bounds,
    };

    s_status_layer = layer_factory_create_custom_layer(builder, status_layer_update_proc);
    return s_status_layer;
}

void status_layer_destroy() {
    if (s_status_layer) {
        layer_destroy(s_status_layer);
        s_status_layer = NULL;
    }
}

void status_layer_update() {
    if (s_status_layer) {
        layer_mark_dirty(s_status_layer);
    }
}
