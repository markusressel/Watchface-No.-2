#include "watch_layout.h"

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

#define PIXEL_ROW_HEIGHT 27
#define FONT_ROW_HEIGHT 55

static const WidgetMetrics s_widget_metrics[WIDGET_COUNT] = {
    [WIDGET_WEATHER]     = { .x = 0, .width_margin =  3, .row_style = WIDGET_ROW_STYLE_PIXEL },
    [WIDGET_DATE]        = { .x = 0, .width_margin =  3, .row_style = WIDGET_ROW_STYLE_PIXEL },
    [WIDGET_TIME]        = { .x = 0, .width_margin =  0, .row_style = WIDGET_ROW_STYLE_FONT },
    [WIDGET_STEPCOUNT]   = { .x = 0, .width_margin =  3, .row_style = WIDGET_ROW_STYLE_PIXEL },
    [WIDGET_BATTERY_BAR] = { .x = 7, .width_margin = 10, .row_style = WIDGET_ROW_STYLE_PIXEL },
};

#define EDGE_MARGIN 5
#define ROW_GAP 5

static int widget_height(WidgetId widget) {
    return s_widget_metrics[widget].row_style == WIDGET_ROW_STYLE_FONT
        ? FONT_ROW_HEIGHT
        : PIXEL_ROW_HEIGHT;
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

static int top_group_end_y(const WatchLayout *layout, int center_idx) {
    if (center_idx == 0) {
        return EDGE_MARGIN;
    }

    int y = EDGE_MARGIN;
    for (int i = 0; i < center_idx; i++) {
        y += widget_height(layout->rows[i].widget);
        if (i < center_idx - 1) {
            y += ROW_GAP;
        }
    }

    return y;
}

static int bottom_group_start_y(const WatchLayout *layout, int center_idx, int screen_height) {
    int rows_after = layout->row_count - 1 - center_idx;
    if (rows_after == 0) {
        return screen_height - EDGE_MARGIN;
    }

    int total_height = 0;
    for (int i = center_idx + 1; i < layout->row_count; i++) {
        total_height += widget_height(layout->rows[i].widget);
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
    int y;

    if (center_idx < 0) {
        int total_height = 0;
        for (int i = 0; i < layout->row_count; i++) {
            total_height += widget_height(layout->rows[i].widget);
        }

        int remaining = screen.size.h - total_height;
        int gap = remaining / (layout->row_count + 1);

        y = gap;
        for (int i = 0; i < row_index; i++) {
            y += widget_height(layout->rows[i].widget) + gap;
        }
    } else if (row_index < center_idx) {
        y = EDGE_MARGIN;
        for (int i = 0; i < row_index; i++) {
            y += widget_height(layout->rows[i].widget) + ROW_GAP;
        }
    } else if (row_index > center_idx) {
        y = screen.size.h - EDGE_MARGIN - widget_height(layout->rows[row_index].widget);
        for (int i = row_index + 1; i < layout->row_count; i++) {
            y -= widget_height(layout->rows[i].widget) + ROW_GAP;
        }
    } else {
        const int center_height = widget_height(layout->rows[center_idx].widget);

        if (center_idx == 0) {
            y = EDGE_MARGIN;
        } else if (center_idx == layout->row_count - 1) {
            y = screen.size.h - EDGE_MARGIN - center_height;
        } else {
            const int top_end_y = top_group_end_y(layout, center_idx);
            const int bottom_start_y = bottom_group_start_y(layout, center_idx, screen.size.h);
            y = top_end_y + (bottom_start_y - top_end_y - center_height) / 2;
        }
    }

    const WidgetMetrics *m = &s_widget_metrics[layout->rows[row_index].widget];
    GRect bounds = GRect(m->x, y, screen.size.w - m->width_margin, widget_height(layout->rows[row_index].widget));
    return layer_builder_from_rect(window_layer, bounds);
}

