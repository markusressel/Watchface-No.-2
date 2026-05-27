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
    [WIDGET_TIME]        = { .x = 0, .width_margin =  0, .height = 55 },
    [WIDGET_STEPCOUNT]   = { .x = 0, .width_margin =  3, .height = 27 },
    [WIDGET_BATTERY_BAR] = { .x = 7, .width_margin = 10, .height = 27 },
};

#define EDGE_MARGIN 5
#define ROW_GAP 5
#define LARGE_ROW_THRESHOLD 30

static int find_large_row(const WatchLayout *layout) {
    for (int i = 0; i < layout->row_count; i++) {
        if (s_widget_metrics[layout->rows[i].widget].height > LARGE_ROW_THRESHOLD) {
            return i;
        }
    }

    return -1;
}

static int top_group_end_y(const WatchLayout *layout, int large_idx) {
    if (large_idx == 0) {
        return EDGE_MARGIN;
    }

    int y = EDGE_MARGIN;
    for (int i = 0; i < large_idx; i++) {
        y += s_widget_metrics[layout->rows[i].widget].height;
        if (i < large_idx - 1) {
            y += ROW_GAP;
        }
    }

    return y;
}

static int bottom_group_start_y(const WatchLayout *layout, int large_idx, int screen_height) {
    int rows_after = layout->row_count - 1 - large_idx;
    if (rows_after == 0) {
        return screen_height - EDGE_MARGIN;
    }

    int total_height = 0;
    for (int i = large_idx + 1; i < layout->row_count; i++) {
        total_height += s_widget_metrics[layout->rows[i].widget].height;
        if (i < layout->row_count - 1) {
            total_height += ROW_GAP;
        }
    }

    return screen_height - EDGE_MARGIN - total_height;
}

LayerBuilder watch_layout_make_builder(
    const WatchLayout *layout,
    Layer *window_layer,
    int row_index
) {
    const GRect screen = layer_get_bounds(window_layer);
    const int large_idx = find_large_row(layout);
    int y;

    if (large_idx < 0) {
        int total_height = 0;
        for (int i = 0; i < layout->row_count; i++) {
            total_height += s_widget_metrics[layout->rows[i].widget].height;
        }

        int remaining = screen.size.h - total_height;
        int gap = remaining / (layout->row_count + 1);

        y = gap;
        for (int i = 0; i < row_index; i++) {
            y += s_widget_metrics[layout->rows[i].widget].height + gap;
        }
    } else if (row_index < large_idx) {
        y = EDGE_MARGIN;
        for (int i = 0; i < row_index; i++) {
            y += s_widget_metrics[layout->rows[i].widget].height + ROW_GAP;
        }
    } else if (row_index > large_idx) {
        y = screen.size.h - EDGE_MARGIN - s_widget_metrics[layout->rows[row_index].widget].height;
        for (int i = row_index + 1; i < layout->row_count; i++) {
            y -= s_widget_metrics[layout->rows[i].widget].height + ROW_GAP;
        }
    } else {
        const int top_end_y = top_group_end_y(layout, large_idx);
        const int bottom_start_y = bottom_group_start_y(layout, large_idx, screen.size.h);
        const int large_height = s_widget_metrics[layout->rows[large_idx].widget].height;
        y = top_end_y + (bottom_start_y - top_end_y - large_height) / 2;
    }

    const WidgetMetrics *m = &s_widget_metrics[layout->rows[row_index].widget];
    GRect bounds = GRect(m->x, y, screen.size.w - m->width_margin, m->height);
    return layer_builder_from_rect(window_layer, bounds);
}

