#pragma once

#include <pebble.h>
#include "layer_factory.h"

// All available widget types. Order here does NOT imply display order.
typedef enum WidgetId {
    WIDGET_WEATHER,
    WIDGET_DATE,
    WIDGET_TIME,
    WIDGET_STEPCOUNT,
    WIDGET_BATTERY_BAR,
    WIDGET_COUNT,
} WidgetId;

// A single row in the layout.
typedef struct WatchRow {
    WidgetId widget;
} WatchRow;

#define WATCH_LAYOUT_MAX_ROWS WIDGET_COUNT

// Full screen layout: an ordered list of rows from top to bottom.
// Small rows above the first large row are stacked from the top edge, small
// rows below it are stacked from the bottom edge, and the large row is centered
// between those two groups. To reorder, change the .rows array.
typedef struct WatchLayout {
    WatchRow rows[WATCH_LAYOUT_MAX_ROWS];
    int row_count;
} WatchLayout;

// Returns a LayerBuilder for row `row_index` in `layout`, sized and positioned
// according to built-in per-widget metrics and the current row grouping rules.
LayerBuilder watch_layout_make_builder(
    const WatchLayout *layout,
    Layer *window_layer,
    int row_index
);

