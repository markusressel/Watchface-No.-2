#include "unity/unity.h"
#include <stdbool.h>
#include <string.h>
#include "pebble/pebble.h"

// Mock dependencies
#include "../../src/c/settings/clay_settings.h"
ClaySettings s_settings;
ClaySettings *clay_get_settings() { return &s_settings; }

#include "../../src/c/ui/theme.h"
static Theme s_theme;
Theme *theme_get_theme() { return &s_theme; }

// weather.h mocks
#include "../../src/c/ui/layer/weather.h"
static WeatherData s_weather_data;
WeatherData *weather_get_data() { return &s_weather_data; }

// ui_state.h mocks
#include "../../src/c/ui/ui_state.h"
static Layer* s_mock_layers[5];
static WidgetId s_mock_widgets[5];
static int s_row_count = 0;
int ui_state_get_row_count() { return s_row_count; }
WidgetId ui_state_get_widget_id(int row) { return s_mock_widgets[row]; }
Layer* ui_state_get_layer(int row) { return s_mock_layers[row]; }

// layer_factory.h mocks
#include "../../src/c/ui/layer_factory.h"
static bool s_mock_layer_create_fail = false;
Layer* layer_factory_create_custom_layer_with_data(LayerBuilder builder, LayerUpdateProc update_proc, size_t data_size) {
    if (s_mock_layer_create_fail) {
        return NULL;
    }
    Layer *layer = layer_create_with_data(builder.bounds, data_size);
    layer_set_update_proc(layer, update_proc);
    return layer;
}

// Graph utils - include real ones for better coverage
#include "../../src/c/ui/graphics/graph_utils.h"
#include "../../src/c/ui/graphics/graph_utils.c"

#include "../../src/c/ui/layer/weather_forecast.c"

void setUp(void) {
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.SliderWeatherForecastPreviewHoursCount = 6;
    memset(&s_weather_data, 0, sizeof(WeatherData));
    memset(&s_theme, 0, sizeof(Theme));
    s_row_count = 0;
    s_mock_layer_create_fail = false;
    reset_graphics_fill_rect_calls();
}

void tearDown(void) {}

void test_weather_forecast_layer_create_destroy(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 40) };
    Layer *layer = create_weather_forecast_layer(builder);
    TEST_ASSERT_NOT_NULL(layer);
    destroy_weather_forecast_layer(layer);
}

void test_weather_forecast_update_proc_with_data(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 40) };
    Layer *layer = create_weather_forecast_layer(builder);
    
    int temp_forecast[] = {20, 21, 22, 23};
    int rain_forecast[] = {0, 5, 10, 0};
    s_weather_data.TemperatureForecast = temp_forecast;
    s_weather_data.TemperatureForecastCount = 4;
    s_weather_data.RainForecastMmX10 = rain_forecast;
    s_weather_data.RainForecastMmX10Count = 4;
    s_weather_data.ForecastStartTimestamp = time(NULL) - 1800; // 30 mins ago
    
    // Trigger update proc
    update_proc(layer, NULL);
    
    // Verified it doesn't crash
    
    destroy_weather_forecast_layer(layer);
}

void test_weather_forecast_tick_update(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 40) };
    Layer *layer = create_weather_forecast_layer(builder);
    
    s_row_count = 1;
    s_mock_widgets[0] = WIDGET_WEATHER_FORECAST;
    s_mock_layers[0] = layer;
    
    s_weather_data.ForecastStartTimestamp = time(NULL) - 3600;
    
    weather_forecast_tick_update();
    // Verified it doesn't crash.
    
    destroy_weather_forecast_layer(layer);
}

void test_weather_forecast_layer_update_settings(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 40) };
    Layer *layer = create_weather_forecast_layer(builder);
    
    s_row_count = 1;
    s_mock_widgets[0] = WIDGET_WEATHER_FORECAST;
    s_mock_layers[0] = layer;
    
    weather_forecast_layer_update_settings();
    // Verified it doesn't crash.
    
    destroy_weather_forecast_layer(layer);
}

void test_weather_forecast_destroy_null(void) {
    destroy_weather_forecast_layer(NULL);
}

void test_weather_forecast_layer_create_fail(void) {
    s_mock_layer_create_fail = true;
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 40) };
    Layer *layer = create_weather_forecast_layer(builder);
    TEST_ASSERT_NULL(layer);
}

void test_weather_forecast_update_proc_fallback_paths(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 40) };
    Layer *layer = create_weather_forecast_layer(builder);
    
    // Fallback: temperature and rain counts <= 0
    s_weather_data.TemperatureForecast = NULL;
    s_weather_data.TemperatureForecastCount = 0;
    s_weather_data.RainForecastMmX10 = NULL;
    s_weather_data.RainForecastMmX10Count = 0;
    s_weather_data.ForecastStartTimestamp = 0; // hide indicator line
    
    update_proc(layer, NULL);
    
    // Fallback: counts > max_forecast_points
    int temp_forecast[35];
    int rain_forecast[35];
    s_weather_data.TemperatureForecast = temp_forecast;
    s_weather_data.TemperatureForecastCount = 35;
    s_weather_data.RainForecastMmX10 = rain_forecast;
    s_weather_data.RainForecastMmX10Count = 35;
    
    update_proc(layer, NULL);
    
    destroy_weather_forecast_layer(layer);
}

void test_weather_forecast_update_proc_larger_dot_size(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 40) };
    Layer *layer = create_weather_forecast_layer(builder);
    
    WeatherForecastLayerData *data = layer_get_data(layer);
    data->series_configs[0].dot_size = 3; // > 1
    
    s_weather_data.ForecastStartTimestamp = time(NULL) - 3600;
    
    update_proc(layer, NULL);
    
    s_row_count = 1;
    s_mock_widgets[0] = WIDGET_WEATHER_FORECAST;
    s_mock_layers[0] = layer;
    
    weather_forecast_tick_update();
    
    destroy_weather_forecast_layer(layer);
}

void test_weather_forecast_tick_update_no_timestamp(void) {
    s_weather_data.ForecastStartTimestamp = 0;
    weather_forecast_tick_update();
}

void test_update_weather_forecast(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 40) };
    Layer *layer = create_weather_forecast_layer(builder);
    
    s_row_count = 1;
    s_mock_widgets[0] = WIDGET_WEATHER_FORECAST;
    s_mock_layers[0] = layer;
    
    update_weather_forecast();
    
    // test with NULL layer
    s_mock_layers[0] = NULL;
    update_weather_forecast();
    
    destroy_weather_forecast_layer(layer);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_weather_forecast_layer_create_destroy);
    RUN_TEST(test_weather_forecast_update_proc_with_data);
    RUN_TEST(test_weather_forecast_tick_update);
    RUN_TEST(test_weather_forecast_layer_update_settings);
    RUN_TEST(test_weather_forecast_destroy_null);
    RUN_TEST(test_weather_forecast_layer_create_fail);
    RUN_TEST(test_weather_forecast_update_proc_fallback_paths);
    RUN_TEST(test_weather_forecast_update_proc_larger_dot_size);
    RUN_TEST(test_weather_forecast_tick_update_no_timestamp);
    RUN_TEST(test_update_weather_forecast);
    return UNITY_END();
}
