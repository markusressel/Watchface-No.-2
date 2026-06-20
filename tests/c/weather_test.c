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
ClaySettings *clay_get_settings() { return &s_settings; }

Theme s_theme;
Theme *theme_get_theme() { return &s_theme; }

int ui_state_get_row_count() { return 0; }
WidgetId ui_state_get_widget_id(int row) { return 0; }
Layer *ui_state_get_layer(int row) { return NULL; }

static int s_layer_data_size = 0;

void layer_set_frame(Layer *layer, GRect frame) {
}

int dotted_text_layer_get_content_width(DottedTextLayer *layer) { return 10; }

void dotted_text_layer_set_text(DottedTextLayer *layer, char *text) {
}

void dotted_text_layer_set_color(DottedTextLayer *layer, GColor color) {
}

void dotted_text_layer_set_auto_scale(DottedTextLayer *layer, bool auto_scale) {
}

void dotted_text_layer_set_scale_factor(DottedTextLayer *layer, float scale) {
}

void dotted_text_layer_destroy(DottedTextLayer *layer) {
}

Layer *layer_factory_create_custom_layer_with_data(LayerBuilder builder, LayerUpdateProc update_proc, size_t data_size) {
    s_layer_data_size = (int) data_size;
    return (Layer *) malloc(data_size > 0 ? data_size : 1);
}

DottedTextLayer *layer_factory_create_dotted_text_layer(LayerBuilder builder, GColor color, HorizontalAlignment horizontal_alignment, VerticalAlignment vertical_alignment,
                                                        const char *text) { return (DottedTextLayer *) malloc(1); }

LayerBuilder layer_builder_from_rect(Layer *parent, GRect bounds) {
    return (LayerBuilder)
    {
        .
        parent = parent,
        .
        bounds = bounds
    };
}

LayerBuilder layer_builder(Layer *parent, LayerLayout layout) {
    return (LayerBuilder)
    {
        .
        parent = parent,
        .
        bounds = GRect(layout.x, layout.y, 144, 168)
    };
}

bool phone_connection_is_connected() { return s_mock_connected; }

void update_weather_forecast() {
}

void weather_forecast_tick_update() {
}

// Include the C file to test static functions
#include "../../src/c/ui/layer/weather.c"

void setUp(void) {
    s_mock_time = 1000000; // Arbitrary start time
    s_mock_connected = true;
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.WeatherUpdateIntervalMinutes = 15;
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

void test_weather_persistence_save_restore(void) {
    // 1. Setup data to save
    WeatherData data = {
        .CurrentTemperature = 22,
        .MaxTemperature = 28,
        .MinTemperature = 15,
        .RainNextHourMmX10 = 12,
        .RainPopPercent = 40,
        .ForecastStartTimestamp = s_mock_time - 3600,
        .TemperatureForecastCount = 70, // > 64 to test fragmentation
        .RainForecastMmX10Count = 10,
        .CurrentConditions = "Sunny",
        .is_dirty = true
    };
    int temp_forecast[70];
    for (int i = 0; i < 70; i++) temp_forecast[i] = i;
    data.TemperatureForecast = temp_forecast;

    int rain_forecast[10];
    for (int i = 0; i < 10; i++) rain_forecast[i] = i * 2;
    data.RainForecastMmX10 = rain_forecast;

    // 2. Save
    save_current_weather_data(&data);

    // 3. Reset runtime state
    memset(&s_weather_data, 0, sizeof(WeatherData));
    s_weather_data_initialized = false;
    ensure_runtime_forecast_storage();

    // 4. Restore
    restore_saved_weather_data();

    // 5. Verify
    TEST_ASSERT_EQUAL_INT(22, s_weather_data.CurrentTemperature);
    TEST_ASSERT_EQUAL_INT(28, s_weather_data.MaxTemperature);
    TEST_ASSERT_EQUAL_INT(15, s_weather_data.MinTemperature);
    TEST_ASSERT_EQUAL_INT(12, s_weather_data.RainNextHourMmX10);
    TEST_ASSERT_EQUAL_INT(40, s_weather_data.RainPopPercent);
    TEST_ASSERT_EQUAL_INT(s_mock_time - 3600, s_weather_data.ForecastStartTimestamp);
    TEST_ASSERT_EQUAL_STRING("Sunny", s_weather_data.CurrentConditions);

    TEST_ASSERT_EQUAL_INT(70, s_weather_data.TemperatureForecastCount);
    for (int i = 0; i < 70; i++) {
        TEST_ASSERT_EQUAL_INT_MESSAGE(i, s_weather_data.TemperatureForecast[i], "Temperature mismatch");
    }

    TEST_ASSERT_EQUAL_INT(10, s_weather_data.RainForecastMmX10Count);
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL_INT_MESSAGE(i * 2, s_weather_data.RainForecastMmX10[i], "Rain mismatch");
    }
}

void test_weather_init_data_expired(void) {
    // 1. Setup expired data in mock storage
    s_settings.SliderWeatherForecastPreviewHoursCount = 6;
    time_t start_time = s_mock_time - (7 * 3600); // 7 hours ago (expired)

    PersistedWeatherData persisted = {
        .Version = WEATHER_DATA_PERSIST_VERSION,
        .ForecastStartTimestamp = start_time,
        .CurrentTemperature = 10
    };
    persist_write_data(WEATHER_DATA_KEY, &persisted, sizeof(PersistedWeatherData));

    // 2. Initialize
    s_weather_data_initialized = false;
    s_app_message_outbox_send_count = 0;
    weather_init_data();

    // 3. Verify: should be cleared, but NOT request update (which is done via window load now)
    TEST_ASSERT_EQUAL_INT(0, s_weather_data.CurrentTemperature);
    TEST_ASSERT_EQUAL_INT(0, s_app_message_outbox_send_count);
}

void test_weather_init_data_valid(void) {
    // 1. Setup valid data in mock storage
    s_settings.SliderWeatherForecastPreviewHoursCount = 6;
    time_t start_time = s_mock_time - (1 * 3600); // 1 hour ago (valid)

    PersistedWeatherData persisted = {
        .Version = WEATHER_DATA_PERSIST_VERSION,
        .ForecastStartTimestamp = start_time,
        .CurrentTemperature = 25
    };
    persist_write_data(WEATHER_DATA_KEY, &persisted, sizeof(PersistedWeatherData));

    // 2. Initialize
    s_weather_data_initialized = false;
    s_app_message_outbox_send_count = 0;
    weather_init_data();

    // 3. Verify: should keep data and NOT request update immediately
    TEST_ASSERT_EQUAL_INT(25, s_weather_data.CurrentTemperature);
    TEST_ASSERT_EQUAL_INT(0, s_app_message_outbox_send_count);
}

void test_weather_check_and_request_update_disconnected(void) {
    s_mock_connected = false;
    s_app_message_outbox_send_count = 0;
    weather_check_and_request_update();
    TEST_ASSERT_EQUAL_INT(0, s_app_message_outbox_send_count);
}

void test_weather_check_and_request_update_simulation(void) {
    s_mock_connected = true;
    s_settings.WeatherUseSimulation = true;
    s_app_message_outbox_send_count = 0;
    weather_check_and_request_update();
    TEST_ASSERT_EQUAL_INT(0, s_app_message_outbox_send_count);
    s_settings.WeatherUseSimulation = false;
}

void test_weather_check_and_request_update_no_data(void) {
    s_mock_connected = true;
    s_weather_data.ForecastStartTimestamp = 0;
    s_app_message_outbox_send_count = 0;
    weather_check_and_request_update();
    TEST_ASSERT_EQUAL_INT(1, s_app_message_outbox_send_count);
}

void test_weather_check_and_request_update_expired(void) {
    s_mock_connected = true;
    s_settings.SliderWeatherForecastPreviewHoursCount = 6;
    s_weather_data.ForecastStartTimestamp = s_mock_time - (7 * 3600); // 7 hours ago
    s_weather_data.CurrentTemperature = 10;
    s_app_message_outbox_send_count = 0;
    
    weather_check_and_request_update();
    
    TEST_ASSERT_EQUAL_INT(0, s_weather_data.CurrentTemperature);
    TEST_ASSERT_EQUAL_INT(0, s_weather_data.ForecastStartTimestamp);
    TEST_ASSERT_EQUAL_INT(1, s_app_message_outbox_send_count);
}

void test_weather_check_and_request_update_older_than_threshold(void) {
    s_mock_connected = true;
    s_settings.SliderWeatherForecastPreviewHoursCount = 6;
    s_settings.WeatherUpdateIntervalMinutes = 15;
    s_weather_data.ForecastStartTimestamp = s_mock_time - (20 * 60); // 20 mins ago (older than 15 min threshold)
    s_weather_data.CurrentTemperature = 25;
    s_app_message_outbox_send_count = 0;
    
    weather_check_and_request_update();
    
    // Should NOT clear the current weather data, but SHOULD request update
    TEST_ASSERT_EQUAL_INT(25, s_weather_data.CurrentTemperature);
    TEST_ASSERT_EQUAL_INT(1, s_app_message_outbox_send_count);
}

void test_weather_check_and_request_update_fresh(void) {
    s_mock_connected = true;
    s_settings.SliderWeatherForecastPreviewHoursCount = 6;
    s_settings.WeatherUpdateIntervalMinutes = 15;
    s_weather_data.ForecastStartTimestamp = s_mock_time - (10 * 60); // 10 mins ago (fresher than 15 min threshold)
    s_weather_data.CurrentTemperature = 25;
    s_app_message_outbox_send_count = 0;
    
    weather_check_and_request_update();
    
    // Should NOT clear and should NOT request update
    TEST_ASSERT_EQUAL_INT(25, s_weather_data.CurrentTemperature);
    TEST_ASSERT_EQUAL_INT(0, s_app_message_outbox_send_count);
}

void test_weather_request_update(void) {
    s_app_message_outbox_send_count = 0;
    weather_request_update();
    TEST_ASSERT_EQUAL_INT(1, s_app_message_outbox_send_count);
}

void test_weather_tick_update_expired(void) {
    // Setup data that is expired
    s_weather_data.ForecastStartTimestamp = s_mock_time - (24 * 3600);
    s_settings.SliderWeatherForecastPreviewHoursCount = 6;
    s_weather_data.CurrentTemperature = 25;

    weather_tick_update();

    // Should be cleared
    TEST_ASSERT_EQUAL_INT(0, s_weather_data.CurrentTemperature);
    TEST_ASSERT_EQUAL_INT(0, s_weather_data.ForecastStartTimestamp);
}

void test_weather_tick_update_valid(void) {
    // Setup data that is valid
    s_weather_data.ForecastStartTimestamp = s_mock_time - (1 * 3600);
    s_settings.SliderWeatherForecastPreviewHoursCount = 6;
    s_weather_data.CurrentTemperature = 25;

    weather_tick_update();

    // Should NOT be cleared
    TEST_ASSERT_EQUAL_INT(25, s_weather_data.CurrentTemperature);
    TEST_ASSERT_NOT_EQUAL(0, s_weather_data.ForecastStartTimestamp);
}

void test_update_weather(void) {
    s_weather_data.CurrentTemperature = 30;
    s_weather_data.ForecastStartTimestamp = s_mock_time;

    // update_weather NO LONGER saves data to persist immediately
    update_weather();

    // Verify it was NOT saved
    PersistedWeatherData persisted = {0};
    persist_read_data(WEATHER_DATA_KEY, &persisted, sizeof(PersistedWeatherData));
    TEST_ASSERT_NOT_EQUAL(30, persisted.CurrentTemperature);
}

void test_weather_delete_persisted_data(void) {
    // Write some data
    PersistedWeatherData persisted = {.CurrentTemperature = 25};
    persist_write_data(WEATHER_DATA_KEY, &persisted, sizeof(PersistedWeatherData));
    TEST_ASSERT_TRUE(persist_exists(WEATHER_DATA_KEY));

    weather_delete_persisted_data();

    TEST_ASSERT_FALSE(persist_exists(WEATHER_DATA_KEY));
}

void test_weather_layer_create_destroy(void) {
    LayerBuilder builder = {0};
    Layer *layer = create_weather_layer(builder);
    TEST_ASSERT_NOT_NULL(layer);
    TEST_ASSERT_EQUAL_INT(sizeof(WeatherLayerData), s_layer_data_size);

    destroy_weather_layer(layer);
}

void test_weather_deinit_data(void) {
    s_weather_data.CurrentTemperature = 35;
    s_weather_data.ForecastStartTimestamp = s_mock_time;
    s_weather_data.is_dirty = true;
    s_weather_data.TemperatureForecast = malloc(10 * sizeof(int));
    s_weather_data.is_temp_forecast_dynamic_alloc = true;
    s_weather_data.RainForecastMmX10 = malloc(10 * sizeof(int));
    s_weather_data.is_rain_forecast_dynamic_alloc = true;

    // deinit_weather_data should now save to persist because is_dirty is true
    deinit_weather_data();

    PersistedWeatherData persisted = {0};
    persist_read_data(WEATHER_DATA_KEY, &persisted, sizeof(PersistedWeatherData));
    TEST_ASSERT_EQUAL_INT(35, persisted.CurrentTemperature);

    TEST_ASSERT_NULL(s_weather_data.TemperatureForecast);
    TEST_ASSERT_NULL(s_weather_data.RainForecastMmX10);
    TEST_ASSERT_FALSE(s_weather_data.is_temp_forecast_dynamic_alloc);
    TEST_ASSERT_FALSE(s_weather_data.is_rain_forecast_dynamic_alloc);
    TEST_ASSERT_FALSE(s_weather_data.is_dirty);
}

void test_weather_deinit_data_not_dirty(void) {
    // 1. Setup mock storage with some data
    PersistedWeatherData persisted_initial = {.CurrentTemperature = 20, .Version = WEATHER_DATA_PERSIST_VERSION, .ForecastStartTimestamp = s_mock_time};
    persist_write_data(WEATHER_DATA_KEY, &persisted_initial, sizeof(PersistedWeatherData));

    // 2. Setup runtime data but NOT dirty
    s_weather_data.CurrentTemperature = 35; // Different from storage
    s_weather_data.ForecastStartTimestamp = s_mock_time;
    s_weather_data.is_dirty = false;

    // 3. Deinit should NOT save because it's not dirty
    deinit_weather_data();

    // 4. Verify storage still has the old data
    PersistedWeatherData persisted_after = {0};
    persist_read_data(WEATHER_DATA_KEY, &persisted_after, sizeof(PersistedWeatherData));
    TEST_ASSERT_EQUAL_INT(20, persisted_after.CurrentTemperature);
}

void test_weather_simulation_mode(void) {
    s_settings.WeatherUseSimulation = true;
    WeatherData *data = weather_get_data();
    TEST_ASSERT_NOT_NULL(data);
    TEST_ASSERT_EQUAL_INT(26, data->CurrentTemperature);
    TEST_ASSERT_EQUAL_STRING("Mock", data->CurrentConditions);
    s_settings.WeatherUseSimulation = false;
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
    RUN_TEST(test_weather_persistence_save_restore);
    RUN_TEST(test_weather_init_data_expired);
    RUN_TEST(test_weather_init_data_valid);
    RUN_TEST(test_weather_check_and_request_update_disconnected);
    RUN_TEST(test_weather_check_and_request_update_simulation);
    RUN_TEST(test_weather_check_and_request_update_no_data);
    RUN_TEST(test_weather_check_and_request_update_expired);
    RUN_TEST(test_weather_check_and_request_update_older_than_threshold);
    RUN_TEST(test_weather_check_and_request_update_fresh);
    RUN_TEST(test_weather_request_update);
    RUN_TEST(test_weather_tick_update_expired);
    RUN_TEST(test_weather_tick_update_valid);
    RUN_TEST(test_update_weather);
    RUN_TEST(test_weather_delete_persisted_data);
    RUN_TEST(test_weather_layer_create_destroy);
    RUN_TEST(test_weather_deinit_data);
    RUN_TEST(test_weather_deinit_data_not_dirty);
    RUN_TEST(test_weather_simulation_mode);
    return UNITY_END();
}