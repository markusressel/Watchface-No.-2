#include "watch_layout.h"
#include "clay_settings.h"

// Per-widget layout constants.
// x            - left edge of the layer (pixels from screen left)
// width_margin - total horizontal pixels subtracted from screen width
typedef struct WidgetMetrics {
    int x;
    int width_margin;
} WidgetMetrics;

#define BASELINE_SCREEN_HEIGHT 168

static const WidgetMetrics s_widget_metrics[WIDGET_COUNT] = {
    [WIDGET_WEATHER] = {.x = 0, .width_margin = 3},
    [WIDGET_DATE] = {.x = 0, .width_margin = 3},
    [WIDGET_TIME] = {.x = 0, .width_margin = 3},
    [WIDGET_STEPCOUNT] = {.x = 0, .width_margin = 3},
    [WIDGET_BATTERY_BAR] = {.x = 7, .width_margin = 10},
};

#define EDGE_MARGIN 5
#define ROW_GAP 5

static int scaled_dimension(int value, float scale_factor) {
    int scaled = (int) (value * scale_factor + 0.5f);
    return scaled < 1 ? 1 : scaled;
}

static int pixel_row_height_for_scale(float scale_factor) {
    ClaySettings *settings = clay_get_settings();
    int dot_height = scaled_dimension(settings->DotHeight, scale_factor);
    int gap_size_vertical = scaled_dimension(settings->DotVerticalGap, scale_factor);
    return (5 * dot_height) + (4 * gap_size_vertical);
}

static float auto_scale_from_screen(const int screen_height) {
    return (float) screen_height / (float) BASELINE_SCREEN_HEIGHT;
}

static int widget_height(WidgetId widget, float pixel_scale) {
    return pixel_row_height_for_scale(pixel_scale);
}

static bool widget_is_center_row(WidgetId widget) {
    return widget == WIDGET_TIME;
}

static int find_center_row(const WatchLayout *layout) {
    for (int i = 0; i < layout->row_count; i++) {
        if (widget_is_center_row(layout->rows[i].widget)) {
            return i;
        }
    }

    return -1;
}

static int total_layout_height_for_scale(
    const WatchLayout *layout,
    int center_idx,
    float pixel_scale
) {
    int row_heights_total = 0;
    for (int i = 0; i < layout->row_count; i++) {
        row_heights_total += widget_height(layout->rows[i].widget, pixel_scale);
    }

    if (center_idx < 0) {
        return row_heights_total + ((layout->row_count - 1) * ROW_GAP);
    }

    const int rows_before_center = center_idx;
    const int rows_after_center = layout->row_count - center_idx - 1;
    const int gaps_top = rows_before_center > 0 ? rows_before_center - 1 : 0;
    const int gaps_bottom = rows_after_center > 0 ? rows_after_center - 1 : 0;

    return (2 * EDGE_MARGIN) + row_heights_total + ((gaps_top + gaps_bottom) * ROW_GAP);
}

static float max_pixel_scale_to_fit(
    const WatchLayout *layout,
    int center_idx,
    int screen_height
) {
    float low = 0.1f;
    float high = 4.0f;

    for (int i = 0; i < 16; i++) {
        const float mid = (low + high) / 2.0f;
        const int used_height = total_layout_height_for_scale(layout, center_idx, mid);
        if (used_height <= screen_height) {
            low = mid;
        } else {
            high = mid;
        }
    }

    return low;
}

static int top_group_end_y(const WatchLayout *layout, const int *row_heights, int center_idx) {
    if (center_idx == 0) {
        return EDGE_MARGIN;
    }

    int y = EDGE_MARGIN;
    for (int i = 0; i < center_idx; i++) {
        y += row_heights[i];
        if (i < center_idx - 1) {
            y += ROW_GAP;
        }
    }

    return y;
}

static int bottom_group_start_y(
    const WatchLayout *layout,
    const int *row_heights,
    int center_idx,
    int screen_height
) {
    int rows_after = layout->row_count - 1 - center_idx;
    if (rows_after == 0) {
        return screen_height - EDGE_MARGIN;
    }

    int total_height = 0;
    for (int i = center_idx + 1; i < layout->row_count; i++) {
        total_height += row_heights[i];
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
    const int center_idx = find_center_row(layout);
    ClaySettings *settings = clay_get_settings();
    float pixel_scale = settings->DotAutoScale
                            ? auto_scale_from_screen(screen.size.h)
                            : settings->DotScaleFactor;
    if (pixel_scale <= 0.0f) {
        pixel_scale = 1.0f;
    }

    float max_fit_scale = max_pixel_scale_to_fit(layout, center_idx, screen.size.h);
    if (pixel_scale > max_fit_scale) {
        pixel_scale = max_fit_scale;
    }

    int row_heights[WATCH_LAYOUT_MAX_ROWS] = {0};
    int total_height = 0;
    for (int i = 0; i < layout->row_count; i++) {
        row_heights[i] = widget_height(layout->rows[i].widget, pixel_scale);
        total_height += row_heights[i];
    }

    int y;

    if (center_idx < 0) {
        int remaining = screen.size.h - total_height;
        int gap = remaining / (layout->row_count + 1);

        y = gap;
        for (int i = 0; i < row_index; i++) {
            y += row_heights[i] + gap;
        }
    } else if (row_index < center_idx) {
        y = EDGE_MARGIN;
        for (int i = 0; i < row_index; i++) {
            y += row_heights[i] + ROW_GAP;
        }
    } else if (row_index > center_idx) {
        y = screen.size.h - EDGE_MARGIN - row_heights[row_index];
        for (int i = row_index + 1; i < layout->row_count; i++) {
            y -= row_heights[i] + ROW_GAP;
        }
    } else {
        const int center_height = row_heights[center_idx];

        if (center_idx == 0) {
            y = EDGE_MARGIN;
        } else if (center_idx == layout->row_count - 1) {
            y = screen.size.h - EDGE_MARGIN - center_height;
        } else {
            const int top_end_y = top_group_end_y(layout, row_heights, center_idx);
            const int bottom_start_y = bottom_group_start_y(layout, row_heights, center_idx, screen.size.h);
            y = top_end_y + (bottom_start_y - top_end_y - center_height) / 2;
        }
    }

    const WidgetMetrics *m = &s_widget_metrics[layout->rows[row_index].widget];
    GRect bounds = GRect(m->x, y, screen.size.w - m->width_margin, row_heights[row_index]);
    return layer_builder_from_rect(window_layer, bounds);
}
