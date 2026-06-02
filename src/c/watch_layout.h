#pragma once

#include <pebble.h>
#include "layer_factory.h"

// All available widget types. Order here does NOT imply display order.
typedef enum WidgetId {
    WIDGET_WEATHER = 0,
    WIDGET_DATE = 1,
    WIDGET_TIME = 2,
    WIDGET_STEPCOUNT = 3,
    WIDGET_BATTERY_BAR = 4,
    WIDGET_HEARTRATE = 5,
    WIDGET_TEMPERATURE_FORECAST = 6,
    WIDGET_RAIN_FORECAST = 7,
    WIDGET_COUNT = 8
} WidgetId;

// A single row in the layout.
typedef struct WatchRow {
    WidgetId widget;
} WatchRow;

// Maximum number of rows supported. Actual number may be less based on hardware.
#define WATCH_LAYOUT_MAX_ROWS 7

// Full screen layout: an ordered list of rows from top to bottom.
// Rows above the time row are stacked from the top edge,
// rows below it are stacked from the bottom edge, and the time row is centered
// between those two groups when rows exist on both sides. If the time row is
// first or last, it anchors to that edge instead. To reorder, change the
// .rows array.
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