#include "watch_layout.h"

// Per-widget layout constants.
// x           - left edge of the layer (pixels from screen left)
// width_margin - total horizontal pixels subtracted from screen width
// height       - row height in pixels
typedef struct WidgetMetrics {
    int x;
    int width_margin;
    int height;
} WidgetMetrics;

static const WidgetMetrics s_widget_metrics[WIDGET_COUNT] = {
    [WIDGET_WEATHER]     = { .x = 0, .width_margin =  3, .height = 27 },
    [WIDGET_DATE]        = { .x = 0, .width_margin =  3, .height = 27 },
    [WIDGET_TIME]        = { .x = 0, .width_margin =  0, .height = 50 },
    [WIDGET_STEPCOUNT]   = { .x = 0, .width_margin =  3, .height = 27 },
    [WIDGET_BATTERY_BAR] = { .x = 7, .width_margin = 10, .height = 27 },
};

LayerBuilder watch_layout_make_builder(
    const WatchLayout *layout,
    Layer *window_layer,
    int row_index
) {
    const GRect screen = layer_get_bounds(window_layer);

    // Sum the height of every row.
    int total_height = 0;
    for (int i = 0; i < layout->row_count; i++) {
        total_height += s_widget_metrics[layout->rows[i].widget].height;
    }

    // Distribute remaining space as equal gaps:
    //   gap  |  row 0  |  gap  |  row 1  |  gap  | ...  |  gap
    // That is row_count+1 equal gaps filling the leftover pixels.
    int remaining = screen.size.h - total_height;
    int gap = remaining / (layout->row_count + 1);

    // Walk down to the requested row.
    int y = gap;
    for (int i = 0; i < row_index; i++) {
        y += s_widget_metrics[layout->rows[i].widget].height + gap;
    }

    const WidgetMetrics *m = &s_widget_metrics[layout->rows[row_index].widget];
    GRect bounds = GRect(m->x, y, screen.size.w - m->width_margin, m->height);
    return layer_builder_from_rect(window_layer, bounds);
}

