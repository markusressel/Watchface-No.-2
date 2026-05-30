#include "watch_layout.h"
#include "clay_settings.h"

typedef enum WidgetRowStyle {
    WIDGET_ROW_STYLE_PIXEL,
    WIDGET_ROW_STYLE_FONT,
} WidgetRowStyle;

// Per-widget layout constants.
// x            - left edge of the layer (pixels from screen left)
// width_margin - total horizontal pixels subtracted from screen width
// row_style    - determines the shared row height and group placement behavior
typedef struct WidgetMetrics {
    int x;
    int width_margin;
    WidgetRowStyle row_style;
} WidgetMetrics;

#define FONT_ROW_HEIGHT 55
#define BASELINE_SCREEN_HEIGHT 168

static const WidgetMetrics s_widget_metrics[WIDGET_COUNT] = {
    [WIDGET_WEATHER]     = { .x = 0, .width_margin =  3, .row_style = WIDGET_ROW_STYLE_PIXEL },
    [WIDGET_DATE]        = { .x = 0, .width_margin =  3, .row_style = WIDGET_ROW_STYLE_PIXEL },
    [WIDGET_TIME]        = { .x = 0, .width_margin =  0, .row_style = WIDGET_ROW_STYLE_FONT },
    [WIDGET_STEPCOUNT]   = { .x = 0, .width_margin =  3, .row_style = WIDGET_ROW_STYLE_PIXEL },
    [WIDGET_BATTERY_BAR] = { .x = 7, .width_margin = 10, .row_style = WIDGET_ROW_STYLE_PIXEL },
};

#define EDGE_MARGIN 5
#define ROW_GAP 5

static int scaled_dimension(int value, float scale_factor) {
    int scaled = (int) (value * scale_factor + 0.5f);
    return scaled < 1 ? 1 : scaled;
}

static int pixel_row_base_height(void) {
    ClaySettings *settings = clay_get_settings();
    return (5 * settings->DotHeight) + (4 * settings->DotVerticalGap);
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

static float max_pixel_scale_to_fit(
    const WatchLayout *layout,
    int center_idx,
    int screen_height
) {
    int pixel_row_count = 0;
    int rows_before_center = 0;
    int rows_after_center = 0;
    for (int i = 0; i < layout->row_count; i++) {
        WidgetId widget = layout->rows[i].widget;
        if (s_widget_metrics[widget].row_style == WIDGET_ROW_STYLE_PIXEL) {
            pixel_row_count++;
        }
        if (center_idx >= 0 && i < center_idx) {
            rows_before_center++;
        } else if (center_idx >= 0 && i > center_idx) {
            rows_after_center++;
        }
    }

    if (pixel_row_count == 0) {
        return 1.0f;
    }

    int fixed_heights = 2 * EDGE_MARGIN;
    if (center_idx >= 0) {
        fixed_heights += FONT_ROW_HEIGHT;
        fixed_heights += ((rows_before_center > 0) ? (rows_before_center - 1) * ROW_GAP : 0);
        fixed_heights += ((rows_after_center > 0) ? (rows_after_center - 1) * ROW_GAP : 0);
    } else {
        fixed_heights += (layout->row_count - 1) * ROW_GAP;
    }

    int available_for_pixels = screen_height - fixed_heights;
    if (available_for_pixels <= 0) {
        return 1.0f;
    }

    int base_pixel_height = pixel_row_base_height();
    if (base_pixel_height <= 0) {
        return 1.0f;
    }

    float max_pixel_height = (float) available_for_pixels / (float) pixel_row_count;
    float max_scale = max_pixel_height / (float) base_pixel_height;
    return max_scale > 0.0f ? max_scale : 1.0f;
}

static int widget_height(WidgetId widget, float pixel_scale) {
    return s_widget_metrics[widget].row_style == WIDGET_ROW_STYLE_FONT
        ? FONT_ROW_HEIGHT
        : pixel_row_height_for_scale(pixel_scale);
}

static bool widget_is_center_row(WidgetId widget) {
    return s_widget_metrics[widget].row_style == WIDGET_ROW_STYLE_FONT;
}

static int find_center_row(const WatchLayout *layout) {
    for (int i = 0; i < layout->row_count; i++) {
        if (widget_is_center_row(layout->rows[i].widget)) {
            return i;
        }
    }

    return -1;
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
