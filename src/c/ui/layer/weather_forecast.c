#include "weather_forecast.h"

#if defined(PBL_PLATFORM_APLITE)

void update_weather_forecast() {
}

void weather_forecast_layer_update_settings() {
}

Layer *create_weather_forecast_layer(LayerBuilder builder) {
    (void) builder;
    return NULL;
}

void destroy_weather_forecast_layer(Layer *layer) {
    (void) layer;
}

#else

#include <pebble.h>
#include <string.h>

#include "../../ui/theme.h"
#include "weather.h"
#include "../graphics/graph_utils.h"
#include "../../settings/clay_settings.h"
#include "../../ui/layer_factory.h"

#define MAX_TEMPERATURE_FORECAST_LAYERS 7
#define FORECAST_POINTS_PER_HOUR 4
#define MAX_FORECAST_POINTS (FORECAST_POINTS_PER_HOUR * 24)
#define NUM_GRAPH_SERIES 2

#if defined(PBL_COLOR)
static const GraphColorStop s_temperature_color_stops[] = {
    {.value = -25, .color = GColorMagenta},
    {.value = -12, .color = GColorBlueMoon},
    {.value = -1, .color = GColorPictonBlue},
    {.value = 0, .color = GColorGreen},
    {.value = 15, .color = GColorChromeYellow},
    {.value = 30, .color = GColorRed},
};

static const GraphColorStop s_rain_color_stops[] = {
    {.value = -50, .color = GColorVividCerulean},
    {.value = -10, .color = GColorPictonBlue},
    {.value = 0, .color = GColorLightGray},
    {.value = 3, .color = GColorPictonBlue},
    {.value = 10, .color = GColorBlueMoon},
    {.value = 50, .color = GColorBlue},
    {.value = 100, .color = GColorDukeBlue},
};

static const int s_rain_scale_steps[] = {100, 250, 500, 1000};
#endif

typedef struct {
    GraphDataSeries data[NUM_GRAPH_SERIES];
    GraphSeriesConfig series_configs[NUM_GRAPH_SERIES];
    GraphYAxisScalingConfig y_axis_scaling_configs[NUM_GRAPH_SERIES];
    GraphInstance forecast_graph;
} WeatherForecastLayerData;

static Layer *s_layers[MAX_TEMPERATURE_FORECAST_LAYERS];
static int s_layer_count = 0;

static void update_proc(Layer *layer, GContext *ctx) {
    WeatherForecastLayerData *data = layer_get_data(layer);
    const GRect bounds = layer_get_bounds(layer);
    WeatherData *weather_data = weather_get_data();
    ClaySettings *settings = clay_get_settings();

    int max_forecast_points = settings->SliderWeatherForecastPreviewHoursCount * FORECAST_POINTS_PER_HOUR;

    // Update rain data
    int rain_value_count = weather_data->RainForecastMmX10Count;
    const int *rain_render_values = weather_data->RainForecastMmX10;
    int rain_fallback_value[1];
    if (rain_value_count <= 0 || !rain_render_values) {
        rain_fallback_value[0] = weather_data->RainNextHourMmX10;
        rain_render_values = rain_fallback_value;
        rain_value_count = 1;
    } else if (rain_value_count > max_forecast_points) {
        rain_value_count = max_forecast_points;
    }
    data->data[0].values = rain_render_values;
    data->data[0].value_count = rain_value_count;

    // Update temperature data
    int temp_value_count = weather_data->TemperatureForecastCount;
    const int *temp_render_values = weather_data->TemperatureForecast;
    int temp_fallback_value[1];
    if (temp_value_count <= 0 || !temp_render_values) {
        temp_fallback_value[0] = weather_data->CurrentTemperature;
        temp_render_values = temp_fallback_value;
        temp_value_count = 1;
    } else if (temp_value_count > max_forecast_points) {
        temp_value_count = max_forecast_points;
    }
    data->data[1].values = temp_render_values;
    data->data[1].value_count = temp_value_count;

    graph_instance_draw(&data->forecast_graph, ctx, bounds);
}

void update_weather_forecast() {
    for (int i = 0; i < s_layer_count; i++) {
        if (s_layers[i] != NULL) {
            layer_mark_dirty(s_layers[i]);
        }
    }
}

void weather_forecast_layer_update_settings() {
    for (int i = 0; i < s_layer_count; i++) {
        if (s_layers[i] != NULL) {
            WeatherForecastLayerData *data = layer_get_data(s_layers[i]);
            data->series_configs[1].default_color = theme_get_theme()->WeatherTextColor;
            layer_mark_dirty(s_layers[i]);
        }
    }
}

Layer *create_weather_forecast_layer(LayerBuilder builder) {
    if (s_layer_count >= MAX_TEMPERATURE_FORECAST_LAYERS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Max temperature forecast layers exceeded!");
        return NULL;
    }

    Layer *layer = layer_factory_create_custom_layer_with_data(builder, update_proc, sizeof(WeatherForecastLayerData));
    if (layer == NULL) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to create temperature forecast layer");
        return NULL;
    }

    WeatherForecastLayerData *data = layer_get_data(layer);

    // Rain Y-axis scaling
    data->y_axis_scaling_configs[0] = (GraphYAxisScalingConfig){
        .has_y_axis_range = true,
        .y_min = 0,
        .y_max = 0,
#if defined(PBL_COLOR)
        .y_axis_max_scale_steps = s_rain_scale_steps,
        .y_axis_max_scale_step_count = (int) (sizeof(s_rain_scale_steps) / sizeof(s_rain_scale_steps[0])),
#else
        .y_axis_max_scale_steps = NULL,
        .y_axis_max_scale_step_count = 0,
#endif
    };

    // Temperature Y-axis scaling (auto)
    data->y_axis_scaling_configs[1] = (GraphYAxisScalingConfig){
        .has_y_axis_range = false,
    };

    // Rain series config
    data->series_configs[0] = (GraphSeriesConfig){
        .graph_type = GRAPH_TYPE_LINE,
        .dot_size = 1,
        .min_interpolated_dot_distance_px = 0,
        .fill_area_under_line = true,
        .dither_fill_colors = true,
        .suppress_exact_zero_value = true,
        .interpolate_color_stops = true,
        .default_color = GColorBlue,
#if defined(PBL_COLOR)
        .color_stops = s_rain_color_stops,
        .color_stop_count = (int) (sizeof(s_rain_color_stops) / sizeof(s_rain_color_stops[0])),
#else
        .color_stops = NULL,
        .color_stop_count = 0,
#endif
        .y_axis_scaling = &data->y_axis_scaling_configs[0],
    };

    // Temperature series config
    data->series_configs[1] = (GraphSeriesConfig){
        .graph_type = GRAPH_TYPE_LINE,
        .dot_size = 1,
        .min_interpolated_dot_distance_px = 0,
        .fill_area_under_line = false,
        .dither_fill_colors = false,
        .suppress_exact_zero_value = false,
        .interpolate_color_stops = true,
        .default_color = theme_get_theme()->WeatherTextColor,
#if defined(PBL_COLOR)
        .color_stops = s_temperature_color_stops,
        .color_stop_count = (int) (sizeof(s_temperature_color_stops) / sizeof(s_temperature_color_stops[0])),
#else
        .color_stops = NULL,
        .color_stop_count = 0,
#endif
        .y_axis_scaling = &data->y_axis_scaling_configs[1],
    };

    GraphDrawConfig draw_config = {
        .series = data->series_configs,
        .series_count = NUM_GRAPH_SERIES,
        .axis = {
            .tick_interval_x = FORECAST_POINTS_PER_HOUR,
            .tick_color_x = GColorDarkGray,
            .tick_length_y = 3,
        }
    };

    graph_instance_init(&data->forecast_graph, data->data, NUM_GRAPH_SERIES, &draw_config);

    s_layers[s_layer_count++] = layer;
    layer_mark_dirty(layer);

    return layer;
}

void destroy_weather_forecast_layer(Layer *layer) {
    if (layer == NULL) {
        return;
    }

    for (int i = 0; i < s_layer_count; i++) {
        if (s_layers[i] == layer) {
            for (int j = i; j < s_layer_count - 1; j++) {
                s_layers[j] = s_layers[j + 1];
            }
            s_layer_count--;
            break;
        }
    }

    layer_destroy(layer);
}

#endif