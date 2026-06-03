#pragma once

#include <pebble.h>

typedef enum GraphType {
    GRAPH_TYPE_POINTS = 0,
    GRAPH_TYPE_LINE = 1,
    GRAPH_TYPE_BAR = 2,
} GraphType;

typedef GColor (*GraphColorForValueFn)(
    int value,
    int min_value,
    int max_value,
    void *context
);

typedef struct GraphColorStop {
    int value;
    GColor color;
} GraphColorStop;

typedef struct GraphDrawConfig {
    GraphType graph_type;
    int dot_size;
    int min_interpolated_dot_distance_px;
    bool fill_area_under_line;
    bool suppress_exact_zero_value;
    bool interpolate_color_stops;
    GColor default_color;
    const GraphColorStop *color_stops;
    int color_stop_count;
    GraphColorForValueFn color_for_value;
    void *color_context;
} GraphDrawConfig;

void graph_draw_series(
    GContext *ctx,
    GRect bounds,
    const int *values,
    int value_count,
    const GraphDrawConfig *config
);

