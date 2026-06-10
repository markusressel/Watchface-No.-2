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

typedef struct GraphYAxisScalingConfig {
    bool has_y_axis_range;
    int y_min;
    int y_max;
    const int *y_axis_max_scale_steps;
    int y_axis_max_scale_step_count;
} GraphYAxisScalingConfig;

typedef struct GraphSeriesConfig {
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
    const GraphYAxisScalingConfig *y_axis_scaling;
} GraphSeriesConfig;

typedef struct GraphAxisConfig {
    int tick_interval_x;
    GColor tick_color_x;
    int tick_length_y;
} GraphAxisConfig;

typedef struct GraphDrawConfig {
    const GraphSeriesConfig *series;
    int series_count;
    GraphAxisConfig axis;
} GraphDrawConfig;

typedef struct GraphDataSeries {
    const int *values;
    int value_count;
} GraphDataSeries;

typedef struct GraphInstance {
    GraphDataSeries *data;
    int data_count;
    GraphDrawConfig config;
} GraphInstance;

void graph_instance_init(GraphInstance *instance, GraphDataSeries *data, int data_count, const GraphDrawConfig *config);

void graph_instance_draw(const GraphInstance *instance, GContext *ctx, GRect bounds);
