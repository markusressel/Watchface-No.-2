#include "../ui/watch_layout.h"
#include "../settings/clay_settings.h"
#include "../ui/layer/widget.h"
#include "../ui/theme.h"
#include "../ui/window.h"

// Layout is built at runtime from settings (see build_layout_from_settings).
static WatchLayout s_layout;

// Per-widget layout constants.
// x            - left edge of the layer (pixels from screen left)
// width_margin - total horizontal pixels subtracted from screen width
typedef struct WidgetMetrics {
    int x;
    int width_margin;
} WidgetMetrics;

#define TIME_ROW_RATIO 1.2f
#define DEFAULT_ROW_RATIO 1.0f

static const WidgetMetrics s_widget_metrics[WIDGET_COUNT] = {
    [WIDGET_WEATHER] = {.x = 0, .width_margin = 3},
    [WIDGET_DATE] = {.x = 0, .width_margin = 3},
    [WIDGET_TIME] = {.x = 0, .width_margin = 3},
    [WIDGET_STEPCOUNT] = {.x = 0, .width_margin = 3},
    [WIDGET_BATTERY_BAR] = {.x = 7, .width_margin = 10},
    [WIDGET_HEARTRATE] = {.x = 0, .width_margin = 3},
    [WIDGET_WEATHER_FORECAST] = {.x = 3, .width_margin = 6},
};

#define EDGE_MARGIN 5
#define ROW_GAP 8

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

static int layout_row_gap_count(const WatchLayout *layout) {
    return layout->row_count > 1 ? layout->row_count - 1 : 0;
}

static float row_ratio(WidgetId widget) {
    return widget == WIDGET_TIME ? TIME_ROW_RATIO : DEFAULT_ROW_RATIO;
}

static void compute_auto_row_heights(
    const WatchLayout *layout,
    int center_idx,
    int screen_height,
    int *row_heights
) {
    const int gap_count = layout_row_gap_count(layout);
    int available = screen_height - (2 * EDGE_MARGIN) - (gap_count * ROW_GAP);
    if (available < layout->row_count) {
        available = layout->row_count;
    }

    float ratio_sum = 0.0f;
    float raw_heights[WATCH_LAYOUT_MAX_ROWS] = {0.0f};
    int used = 0;
    for (int i = 0; i < layout->row_count; i++) {
        ratio_sum += row_ratio(layout->rows[i].widget);
    }
    if (ratio_sum <= 0.0f) {
        ratio_sum = 1.0f;
    }

    for (int i = 0; i < layout->row_count; i++) {
        raw_heights[i] = ((float) available * row_ratio(layout->rows[i].widget)) / ratio_sum;
        row_heights[i] = (int) raw_heights[i];
        if (row_heights[i] < 1) {
            row_heights[i] = 1;
        }
        used += row_heights[i];
    }

    int remaining = available - used;
    while (remaining > 0) {
        int best_idx = -1;
        float best_frac = -2.0f;
        int best_dist_to_center = 1000000;
        for (int i = 0; i < layout->row_count; i++) {
            const int base_height = (int) raw_heights[i];
            const int assigned_extra = row_heights[i] - base_height;
            const float frac = raw_heights[i] - (float) base_height - (float) assigned_extra;
            int dist_to_center = center_idx >= 0 ? i - center_idx : 0;
            if (dist_to_center < 0) {
                dist_to_center = -dist_to_center;
            }

            if (
                frac > best_frac ||
                (frac == best_frac && dist_to_center < best_dist_to_center)
            ) {
                best_frac = frac;
                best_idx = i;
                best_dist_to_center = dist_to_center;
            }
        }
        if (best_idx < 0) {
            break;
        }
        row_heights[best_idx]++;
        remaining--;
    }

    while (remaining < 0) {
        int best_idx = -1;
        int best_height = 0;
        for (int i = 0; i < layout->row_count; i++) {
            if (row_heights[i] > best_height && row_heights[i] > 1) {
                best_height = row_heights[i];
                best_idx = i;
            }
        }
        if (best_idx < 0) {
            break;
        }
        row_heights[best_idx]--;
        remaining++;
    }
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

    int row_heights[WATCH_LAYOUT_MAX_ROWS] = {0};
    int total_height = 0;
    if (settings->DotAutoScale) {
        compute_auto_row_heights(layout, center_idx, screen.size.h, row_heights);
    } else {
        float pixel_scale = settings->DotScaleFactor;
        if (pixel_scale <= 0.0f) {
            pixel_scale = 1.0f;
        }
        for (int i = 0; i < layout->row_count; i++) {
            row_heights[i] = widget_height(layout->rows[i].widget, pixel_scale);
        }
    }

    for (int i = 0; i < layout->row_count; i++) {
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
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "watch_layout: building row %d, widget %d, bounds=(%d,%d,%d,%d)", row_index, layout->rows[row_index].widget, bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h);
    return layer_builder_from_rect(window_layer, bounds);
}

void build_layout_from_settings(ClaySettings *settings) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "build_layout_from_settings");

    int row_count = settings->LayoutRowCount;
    if (row_count < WATCH_LAYOUT_MIN_ROWS) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Layout row count too low, setting to minimum of %d", WATCH_LAYOUT_MIN_ROWS);
        row_count = WATCH_LAYOUT_MIN_ROWS;
    } else if (row_count > WATCH_LAYOUT_MAX_ROWS) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Layout row count too high, setting to maximum of %d", WATCH_LAYOUT_MAX_ROWS);
        row_count = WATCH_LAYOUT_MAX_ROWS;
    }

    s_layout = (WatchLayout){
        .row_count = row_count,
        .rows = {
            [0] = {.widget = (WidgetId) settings->Row0Widget},
            [1] = {.widget = (WidgetId) settings->Row1Widget},
            [2] = {.widget = (WidgetId) settings->Row2Widget},
            [3] = {.widget = (WidgetId) settings->Row3Widget},
            [4] = {.widget = (WidgetId) settings->Row4Widget},
            [5] = {.widget = (WidgetId) settings->Row5Widget},
            [6] = {.widget = (WidgetId) settings->Row6Widget},
        },
    };
}

void main_reload_layout(ClaySettings *settings, Window *window) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "main_reload_layout");

    window_unload(window);
    apply_theme_from_settings(settings, window);
    build_layout_from_settings(settings);
    window_load(window);
}

WatchLayout *watch_layout_get_layout() {
    return &s_layout;
}
