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

typedef struct GraphDrawConfig {
    GraphType graph_type;
    int dot_size;
    int interpolation_steps;
    bool bars_from_zero;
    GColor default_color;
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

