#pragma once

#include <pebble.h>
#include "layer_factory.h"
#include "layer/widget.h"

// A single row in the layout.
typedef struct WatchRow {
    WidgetId widget;
} WatchRow;

// Maximum number of rows supported. The actual number may be lower based on hardware.
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
