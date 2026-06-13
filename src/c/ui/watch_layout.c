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
#define ROW_GAP 1

static int scaled_dimension(int value, float scale_factor) {
    int scaled = (int) (value * scale_factor + 0.5f);
    return scaled < 1 ? 1 : scaled;
}

static int pixel_row_height_for_scale(WidgetId widget, float scale_factor) {
    ClaySettings *settings = clay_get_settings();
    int base_dot_height = settings->DotHeight;
    int base_gap_vertical = settings->DotVerticalGap;

    if (widget == WIDGET_TIME) {
        base_dot_height = 10;
        base_gap_vertical = 0;
    }

    int dot_height = scaled_dimension(base_dot_height, scale_factor);
    int gap_size_vertical = scaled_dimension(base_gap_vertical, scale_factor);
    return (5 * dot_height) + (4 * gap_size_vertical);
}

static int widget_height(WidgetId widget, float pixel_scale) {
    return pixel_row_height_for_scale(widget, pixel_scale);
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

static void compute_auto_row_heights(
    const WatchLayout *layout,
    int center_idx,
    int screen_height,
    int *row_heights,
    int *gap_padding
) {
    const int gap_count = layout_row_gap_count(layout);
    int available = screen_height - (2 * EDGE_MARGIN) - (gap_count * ROW_GAP);
    if (available < layout->row_count) {
        available = layout->row_count;
    }

    ClaySettings *settings = clay_get_settings();
    const int base_h = settings->DotHeight;
    const int base_g = settings->DotVerticalGap;
    const int base_std_total_h = (5 * base_h) + (4 * base_g);
    const float time_ratio = settings->TimeRowRatio > 0.01f ? settings->TimeRowRatio : 1.2f;

    int time_row_idx = -1;
    for (int i = 0; i < layout->row_count; i++) {
        if (layout->rows[i].widget == WIDGET_TIME) {
            time_row_idx = i;
            break;
        }
    }

    int standard_row_count = layout->row_count;
    if (time_row_idx >= 0) {
        standard_row_count--;
    }

    float divisor = (time_row_idx >= 0) ? ((float) standard_row_count + time_ratio) : (float) standard_row_count;
    float max_std_budget = (float) available / divisor;

    // Find the largest pixel-perfect height that fits in the budget
    float scale = max_std_budget / (float) base_std_total_h;
    int h_std = 0;
    if (scale > 0.0f) {
        int scaled_h = (int) ((float) base_h * scale + 0.5f);
        int scaled_g = (int) ((float) base_g * scale + 0.5f);
        int total_h = 5 * scaled_h + 4 * scaled_g;

        if (total_h > (int) max_std_budget && max_std_budget > 4.5f) {
            scale = (max_std_budget - 4.5f) / (float) base_std_total_h;
            if (scale <= 0.0f) scale = 0.1f;
            scaled_h = (int) ((float) base_h * scale + 0.5f);
            scaled_g = (int) ((float) base_g * scale + 0.5f);
            total_h = 5 * scaled_h + 4 * scaled_g;
        }
        h_std = total_h;
    }
    if (h_std < 1) h_std = 1;

    int h_time = 0;
    if (time_row_idx >= 0) {
        h_time = (int) (max_std_budget * time_ratio + 0.5f);
        if (h_time < 1) h_time = 1;
    }

    int total_assigned = (standard_row_count * h_std) + h_time;
    if (total_assigned > available) {
        // Fallback: If combined height overflows, use simple remainder for Time to ensure fit.
        h_time = available - (standard_row_count * h_std);
        if (h_time < 1) h_time = 1;
        total_assigned = (standard_row_count * h_std) + h_time;
    }

    *gap_padding = available - total_assigned;

    for (int i = 0; i < layout->row_count; i++) {
        row_heights[i] = (i == time_row_idx) ? h_time : h_std;
    }
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
    int gap_padding = 0;

    if (settings->DotAutoScale) {
        compute_auto_row_heights(layout, center_idx, screen.size.h, row_heights, &gap_padding);
    } else {
        float pixel_scale = settings->DotScaleFactor > 0.0f ? settings->DotScaleFactor : 1.0f;
        for (int i = 0; i < layout->row_count; i++) {
            row_heights[i] = widget_height(layout->rows[i].widget, pixel_scale);
        }
        int total_content_h = 0;
        for (int i = 0; i < layout->row_count; i++) total_content_h += row_heights[i];
        const int gap_count = layout_row_gap_count(layout);
        int available = screen.size.h - (2 * EDGE_MARGIN) - (gap_count * ROW_GAP);
        gap_padding = available - total_content_h;
        if (gap_padding < 0) gap_padding = 0;
    }

    int total_height_before = 0;
    for (int i = 0; i < row_index; i++) {
        total_height_before += row_heights[i];
    }

    const int gap_count = layout->row_count - 1;
    const int total_gap_space = (gap_count > 0) ? (gap_count * ROW_GAP + gap_padding) : 0;

    // Linear interpolation to distribute gaps/padding perfectly across the screen
    int accumulated_gap_space = (gap_count > 0) ? (row_index * total_gap_space) / gap_count : 0;
    int y = EDGE_MARGIN + total_height_before + accumulated_gap_space;

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

    s_layout = (WatchLayout)
    {
        .
        row_count = row_count,
        .
        rows = {
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