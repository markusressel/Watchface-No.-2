#pragma once

#include <pebble.h>

typedef enum GraphType {
    GRAPH_TYPE_POINTS = 0,
    GRAPH_TYPE_LINE = 1,
    GRAPH_TYPE_BAR = 2,
} GraphType;

/**
 * @struct GraphColorStop
 * @brief Defines a specific color at a specific value point in a gradient.
 */
typedef struct GraphColorStop {
    int value; /**< The data value at which this color should be applied. */
    GColor color; /**< The color to apply at this value. */
} GraphColorStop;

/**
 * @struct GraphYAxisScalingConfig
 * @brief Configuration for how a series' Y-axis should be scaled.
 */
typedef struct GraphYAxisScalingConfig {
    bool has_y_axis_range; /**< If true, use the manual y_min and y_max values. If false, autoscale based on data. */
    int y_min; /**< The fixed minimum value for the Y-axis. */
    int y_max; /**< The fixed maximum value for the Y-axis. */
    const int *y_axis_max_scale_steps; /**< An array of predefined values to snap the Y-axis maximum to. The smallest step that is >= the data's max value will be used. */
    int y_axis_max_scale_step_count; /**< The number of steps in the y_axis_max_scale_steps array. */
} GraphYAxisScalingConfig;

/**
 * @struct GraphSeriesConfig
 * @brief Configuration for the visual style of a single data series.
 */
typedef struct GraphSeriesConfig {
    GraphType graph_type; /**< The type of graph to draw (points, line, or bar). */
    int dot_size; /**< The size of the dots for point graphs, or the thickness of the line for line graphs. */
    int min_interpolated_dot_distance_px; /**< The minimum pixel distance between interpolated points. Set to 0 to disable interpolation. */
    bool fill_area_under_line; /**< For line graphs, whether to fill the area below the line. */
    bool dither_fill_colors; /**< Whether to apply Bayer dithering to the color gradients in the filled area to reduce banding. (Only applies to fill area) */
    bool suppress_exact_zero_value; /**< If true, values of exactly zero will not be drawn. */
    bool interpolate_color_stops; /**< If true, colors will be smoothly interpolated between the defined color stops. (Applies to both line and fill) */
    GColor default_color; /**< The default color to use if no color stops are defined. */
    const GraphColorStop *color_stops; /**< An array of color stops that define the gradient. */
    int color_stop_count; /**< The number of color stops in the color_stops array. */
    const GraphYAxisScalingConfig *y_axis_scaling; /**< A pointer to the Y-axis scaling configuration for this series. */
} GraphSeriesConfig;

/**
 * @struct GraphAxisConfig
 * @brief Configuration for the shared graph axes.
 */
typedef struct GraphAxisConfig {
    int tick_interval_x; /**< The interval (in data points) at which to draw a vertical tick mark on the X-axis. Set to 0 to disable. */
    GColor tick_color_x; /**< The color of the X-axis tick marks. */
    int tick_length_y; /**< The height (in pixels) of the X-axis tick marks. */
    bool show_indicator_line; /**< If true, draw a horizontal indicator bar from the left up to indicator_line_x_index. */
    bool interpolate_indicator_line; /**< If true, use fractional part of indicator_line_x_index. If false, snap to nearest integer index. */
    float indicator_line_x_index; /**< The fractional X-index up to which to draw the indicator bar. */
    GColor indicator_line_color; /**< The color of the indicator bar. */
} GraphAxisConfig;

/**
 * @struct GraphDrawConfig
 * @brief The main configuration that holds the series and axis configs.
 */
typedef struct GraphDrawConfig {
    const GraphSeriesConfig *series; /**< A pointer to an array of series configurations. */
    int series_count; /**< The number of series configurations in the array. */
    GraphAxisConfig axis; /**< The configuration for the shared axes. */
} GraphDrawConfig;

/**
 * @struct GraphDataSeries
 * @brief Represents a single series of data points to be plotted.
 */
typedef struct GraphDataSeries {
    const int *values; /**< A pointer to an array of integer data points. */
    int value_count; /**< The number of data points in the values array. */
} GraphDataSeries;

/**
 * @struct GraphInstance
 * @brief The main graph object that holds the data and configuration.
 */
typedef struct GraphInstance {
    GraphDataSeries *data; /**< A pointer to an array of data series. */
    int data_count; /**< The number of data series in the array. */
    GraphDrawConfig config; /**< The drawing configuration for the graph. */
} GraphInstance;

/**
 * @brief Initializes a GraphInstance.
 * @param instance A pointer to the GraphInstance to initialize.
 * @param data A pointer to the array of data series.
 * @param data_count The number of data series.
 * @param config A pointer to the drawing configuration.
 */
void graph_instance_init(GraphInstance *instance, GraphDataSeries *data, int data_count, const GraphDrawConfig *config);

/**
 * @brief Draws the entire graph, including axes and all series.
 * @param instance A pointer to the configured GraphInstance to draw.
 * @param ctx The graphics context to draw into.
 * @param bounds The bounding box in which to draw the graph.
 */
void graph_instance_draw(const GraphInstance *instance, GContext *ctx, GRect bounds);