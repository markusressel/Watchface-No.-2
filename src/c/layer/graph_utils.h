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
    bool has_y_axis_range;
    int y_min;
    int y_max;
} GraphDrawConfig;

typedef struct GraphInstance {
    const int *values;
    int value_count;
    GraphDrawConfig config;
} GraphInstance;

void graph_draw_series(
    GContext *ctx,
    GRect bounds,
    const int *values,
    int value_count,
    const GraphDrawConfig *config
);

void graph_instance_init(GraphInstance *instance, const int *values, int value_count);

void graph_instance_set_config(GraphInstance *instance, const GraphDrawConfig *config);

void graph_instance_set_y_axis_range(GraphInstance *instance, int y_min, int y_max);

void graph_instance_draw(const GraphInstance *instance, GContext *ctx, GRect bounds);
