#include "unity/unity.h"
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// Mock time for testing
static time_t s_mock_time = 0;
time_t mock_time(time_t *t) {
    if (t) *t = s_mock_time;
    return s_mock_time;
}
#define time mock_time

// Mock dependencies for weather.c
#include "pebble/pebble.h"

// Define Pebble message keys that are normally generated
uint32_t MESSAGE_KEY_RequestWeatherData = 1001;
uint32_t MESSAGE_KEY_WEATHER_TEMPERATURE_CURRENT = 1002;
uint32_t MESSAGE_KEY_WEATHER_CONDITION = 1003;
uint32_t MESSAGE_KEY_WEATHER_TEMPERATURE_MAX = 1004;
uint32_t MESSAGE_KEY_WEATHER_TEMPERATURE_MIN = 1005;
uint32_t MESSAGE_KEY_WEATHER_RAIN_NEXT_HOUR_MM_X10 = 1006;
uint32_t MESSAGE_KEY_WEATHER_RAIN_POP_PERCENT = 1007;
uint32_t MESSAGE_KEY_WEATHER_TEMP_FORECAST_ENCODED = 1008;
uint32_t MESSAGE_KEY_WEATHER_RAIN_FORECAST_MM_X10_ENCODED = 1009;
uint32_t MESSAGE_KEY_WEATHER_FORECAST_START_TS = 1010;

#include "../../src/c/settings/clay_settings.h"
#include "../../src/c/ui/theme.h"
#include "../../src/c/ui/ui_state.h"
#include "../../src/c/ui/layer_factory.h"
#include "../../src/c/ui/layer/dotted_text_layer.h"
#include "../../src/c/ui/layer/weather_forecast.h"
#include "../../src/c/system/phone_connection.h"

// Mock functions that are not in headers or need simple implementations
ClaySettings s_settings;
ClaySettings* clay_get_settings() { return &s_settings; }

Theme s_theme;
Theme* theme_get_theme() { return &s_theme; }

int ui_state_get_row_count() { return 0; }
WidgetId ui_state_get_widget_id(int row) { return 0; }
Layer* ui_state_get_layer(int row) { return NULL; }

void* layer_get_data(const Layer* layer) { return NULL; }
void layer_set_frame(Layer* layer, GRect frame) {}

int dotted_text_layer_get_content_width(DottedTextLayer* layer) { return 0; }
void dotted_text_layer_set_text(DottedTextLayer* layer, char* text) {}
void dotted_text_layer_set_color(DottedTextLayer* layer, GColor color) {}
void dotted_text_layer_set_auto_scale(DottedTextLayer* layer, bool auto_scale) {}
void dotted_text_layer_set_scale_factor(DottedTextLayer* layer, float scale) {}
void dotted_text_layer_destroy(DottedTextLayer* layer) {}

Layer* layer_factory_create_custom_layer_with_data(LayerBuilder builder, LayerUpdateProc update_proc, size_t data_size) { return NULL; }
DottedTextLayer* layer_factory_create_dotted_text_layer(LayerBuilder builder, GColor color, HorizontalAlignment horizontal_alignment, VerticalAlignment vertical_alignment, const char* text) { return NULL; }

LayerBuilder layer_builder_from_rect(Layer *parent, GRect bounds) {
    return (LayerBuilder){.parent = parent, .bounds = bounds};
}

LayerBuilder layer_builder(Layer *parent, LayerLayout layout) {
    return (LayerBuilder){.parent = parent, .bounds = GRect(layout.x, layout.y, 144, 168)};
}

bool phone_connection_is_connected() { return true; }

void update_weather_forecast() {}
void weather_forecast_tick_update() {}

// Include the C file to test static functions
#include "../../src/c/ui/layer/weather.c"

void setUp(void) {
    s_mock_time = 1000000; // Arbitrary start time
    memset(&s_settings, 0, sizeof(ClaySettings));
    memset(&s_theme, 0, sizeof(Theme));
    mock_storage_reset();
}

void tearDown(void) {
}

void test_weather_get_current_temp_no_forecast(void) {
    WeatherData data = {
        .CurrentTemperature = 20,
        .ForecastStartTimestamp = 0,
        .TemperatureForecastCount = 0
    };

    TEST_ASSERT_EQUAL_INT(20, weather_get_current_temp(&data));
}

void test_weather_get_current_temp_before_forecast(void) {
    int forecast[] = {25, 26, 27};
    WeatherData data = {
        .CurrentTemperature = 20,
        .ForecastStartTimestamp = s_mock_time + 100, // Forecast starts in future
        .TemperatureForecastCount = 3,
        .TemperatureForecast = forecast
    };

    TEST_ASSERT_EQUAL_INT(20, weather_get_current_temp(&data));
}

void test_weather_get_current_temp_at_start(void) {
    int forecast[] = {25, 26, 27};
    WeatherData data = {
        .CurrentTemperature = 20,
        .ForecastStartTimestamp = s_mock_time,
        .TemperatureForecastCount = 3,
        .TemperatureForecast = forecast
    };

    // At exactly the start, it should still return CurrentTemperature (index 0)
    TEST_ASSERT_EQUAL_INT(20, weather_get_current_temp(&data));
}

void test_weather_get_current_temp_closest_to_first_forecast(void) {
    int forecast[] = {25, 26, 27};
    WeatherData data = {
        .CurrentTemperature = 20,
        .ForecastStartTimestamp = s_mock_time - (7 * 60), // 7 mins ago
        .TemperatureForecastCount = 3,
        .TemperatureForecast = forecast
    };

    // 7 mins is < 7.5 mins (half of 15), so index 0
    TEST_ASSERT_EQUAL_INT(20, weather_get_current_temp(&data));
}

void test_weather_get_current_temp_closest_to_second_forecast(void) {
    int forecast[] = {25, 26, 27};
    WeatherData data = {
        .CurrentTemperature = 20,
        .ForecastStartTimestamp = s_mock_time - (8 * 60), // 8 mins ago
        .TemperatureForecastCount = 3,
        .TemperatureForecast = forecast
    };

    // 8 mins is > 7.5 mins, so index 1
    TEST_ASSERT_EQUAL_INT(26, weather_get_current_temp(&data));
}

void test_weather_get_current_temp_later_forecast(void) {
    int forecast[] = {25, 26, 27};
    WeatherData data = {
        .CurrentTemperature = 20,
        .ForecastStartTimestamp = s_mock_time - (30 * 60), // 30 mins ago
        .TemperatureForecastCount = 3,
        .TemperatureForecast = forecast
    };

    // 30 mins is exactly index 2
    TEST_ASSERT_EQUAL_INT(27, weather_get_current_temp(&data));
}

void test_weather_get_current_temp_clamped(void) {
    int forecast[] = {25, 26, 27};
    WeatherData data = {
        .CurrentTemperature = 20,
        .ForecastStartTimestamp = s_mock_time - (100 * 60), // 100 mins ago
        .TemperatureForecastCount = 3,
        .TemperatureForecast = forecast
    };

    // Should be clamped to last index
    TEST_ASSERT_EQUAL_INT(27, weather_get_current_temp(&data));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_weather_get_current_temp_no_forecast);
    RUN_TEST(test_weather_get_current_temp_before_forecast);
    RUN_TEST(test_weather_get_current_temp_at_start);
    RUN_TEST(test_weather_get_current_temp_closest_to_first_forecast);
    RUN_TEST(test_weather_get_current_temp_closest_to_second_forecast);
    RUN_TEST(test_weather_get_current_temp_later_forecast);
    RUN_TEST(test_weather_get_current_temp_clamped);
    return UNITY_END();
}
