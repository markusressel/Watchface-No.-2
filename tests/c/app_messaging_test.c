#include "unity/unity.h"
#include <time.h>
#include <pebble.h>
#include <stdlib.h>
#include <string.h>

// Mock MESSAGE_KEYs
uint32_t MESSAGE_KEY_AppReady = 1;
uint32_t MESSAGE_KEY_RequestSettings = 2;
uint32_t MESSAGE_KEY_Theme = 3;
uint32_t MESSAGE_KEY_BackgroundColor = 4;
uint32_t MESSAGE_KEY_TimeTextColor = 5;
uint32_t MESSAGE_KEY_DateTextColor = 6;
uint32_t MESSAGE_KEY_WeekdayTextColor = 65;
uint32_t MESSAGE_KEY_DateSeparatorColor = 66;
uint32_t MESSAGE_KEY_BatteryFrameColor = 7;
uint32_t MESSAGE_KEY_BatteryFillColor = 8;
uint32_t MESSAGE_KEY_BatteryLowColor = 9;
uint32_t MESSAGE_KEY_WeatherTextColor = 10;
uint32_t MESSAGE_KEY_WeatherMaxTempColor = 11;
uint32_t MESSAGE_KEY_WeatherCurrentTempColor = 12;
uint32_t MESSAGE_KEY_WeatherMinTempColor = 13;
uint32_t MESSAGE_KEY_WeatherAxisTickColor = 14;
uint32_t MESSAGE_KEY_WeatherIndicatorColor = 15;
uint32_t MESSAGE_KEY_WeatherSeparatorColor = 67;
uint32_t MESSAGE_KEY_ForecastTempColorM10 = 16;
uint32_t MESSAGE_KEY_ForecastTempColor0 = 17;
uint32_t MESSAGE_KEY_ForecastTempColor10 = 18;
uint32_t MESSAGE_KEY_ForecastTempColor20 = 19;
uint32_t MESSAGE_KEY_ForecastTempColor30 = 20;
uint32_t MESSAGE_KEY_ForecastTempColor40 = 21;
uint32_t MESSAGE_KEY_ForecastRainColor0 = 22;
uint32_t MESSAGE_KEY_ForecastRainColor3 = 23;
uint32_t MESSAGE_KEY_ForecastRainColor10 = 24;
uint32_t MESSAGE_KEY_ForecastRainColor50 = 25;
uint32_t MESSAGE_KEY_ForecastRainColor100 = 26;
uint32_t MESSAGE_KEY_StepcountTextColor = 27;
uint32_t MESSAGE_KEY_HeartrateTextColor = 28;
uint32_t MESSAGE_KEY_WeatherSlot1 = 29;
uint32_t MESSAGE_KEY_WeatherSlot2 = 30;
uint32_t MESSAGE_KEY_WeatherSlot3 = 31;
uint32_t MESSAGE_KEY_LowBatteryThreshold = 32;
uint32_t MESSAGE_KEY_SliderDigitWidth = 33;
uint32_t MESSAGE_KEY_SliderDotWidth = 34;
uint32_t MESSAGE_KEY_SliderDotHeight = 35;
uint32_t MESSAGE_KEY_SliderDotHorizontalGap = 36;
uint32_t MESSAGE_KEY_SliderDotVerticalGap = 37;
uint32_t MESSAGE_KEY_ToggleDotAutoScale = 38;
uint32_t MESSAGE_KEY_SliderDotScaleFactorPercent = 39;
uint32_t MESSAGE_KEY_ShowYear = 40;
uint32_t MESSAGE_KEY_DateZeroPadding = 64;
uint32_t MESSAGE_KEY_ShowSeconds = 41;
uint32_t MESSAGE_KEY_ShowWeekdayAbbreviation = 42;
uint32_t MESSAGE_KEY_WeekdayAbbreviationUppercase = 43;
uint32_t MESSAGE_KEY_WeatherUseSimulation = 44;
uint32_t MESSAGE_KEY_SliderWeatherForecastPreviewHoursCount = 45;
uint32_t MESSAGE_KEY_WeatherUpdateIntervalMinutes = 46;
uint32_t MESSAGE_KEY_Row0Widget = 47;
uint32_t MESSAGE_KEY_Row1Widget = 48;
uint32_t MESSAGE_KEY_Row2Widget = 49;
uint32_t MESSAGE_KEY_Row3Widget = 50;
uint32_t MESSAGE_KEY_Row4Widget = 51;
uint32_t MESSAGE_KEY_Row5Widget = 52;
uint32_t MESSAGE_KEY_Row6Widget = 53;
uint32_t MESSAGE_KEY_LayoutRowCount = 54;
uint32_t MESSAGE_KEY_SliderTimeRowRatioPercent = 55;

uint32_t MESSAGE_KEY_WEATHER_TEMPERATURE_CURRENT = 100;
uint32_t MESSAGE_KEY_WEATHER_TEMPERATURE_MIN = 101;
uint32_t MESSAGE_KEY_WEATHER_TEMPERATURE_MAX = 102;
uint32_t MESSAGE_KEY_WEATHER_CONDITION = 103;
uint32_t MESSAGE_KEY_WEATHER_RAIN_NEXT_HOUR_MM_X10 = 104;
uint32_t MESSAGE_KEY_WEATHER_RAIN_POP_PERCENT = 105;
uint32_t MESSAGE_KEY_WEATHER_TEMP_FORECAST_ENCODED = 106;
uint32_t MESSAGE_KEY_WEATHER_RAIN_FORECAST_MM_X10_ENCODED = 107;
uint32_t MESSAGE_KEY_WEATHER_FORECAST_START_TS = 108;

#include "../../src/c/app_messaging/app_messaging.h"

// Mock dependencies
#include "../../src/c/settings/clay_settings.h"
#include "../../src/c/ui/layer/weather.h"
#include "../../src/c/ui/watch_layout.h"

// Mock weather data
static WeatherData s_test_weather_data;
WeatherData *weather_get_data() { return &s_test_weather_data; }

// Mock settings
static ClaySettings s_test_settings;
ClaySettings *clay_get_settings() { return &s_test_settings; }
ClaySettings *clay_save_settings(ClaySettings *s) { return s; }

// Mock main functions
void main_reload_layout(ClaySettings *settings, Window *window) {
}

Window *main_get_window() { return (Window *) 1; }

// Mock weather functions
void weather_init_data() {
}

void update_weather() {
}

void update_weather_forecast() {
}

void clay_log_settings_debug(const char *label, ClaySettings *s) {
}

// Mock Dictionary Iterator and Tuples
#define MAX_TUPLES 100
static Tuple s_mock_tuples[MAX_TUPLES];
static int s_mock_tuple_count = 0;

void reset_mock_dict() {
    s_mock_tuple_count = 0;
    memset(s_mock_tuples, 0, sizeof(s_mock_tuples));
}

void add_mock_int_tuple(uint32_t key, int32_t value) {
    if (s_mock_tuple_count < MAX_TUPLES) {
        s_mock_tuples[s_mock_tuple_count].message_key = key;
        s_mock_tuples[s_mock_tuple_count].type = TUPLE_INT;
        s_mock_tuples[s_mock_tuple_count].value[0].int32 = value;
        s_mock_tuple_count++;
    }
}

void add_mock_string_tuple(uint32_t key, const char *value) {
    if (s_mock_tuple_count < MAX_TUPLES) {
        s_mock_tuples[s_mock_tuple_count].message_key = key;
        s_mock_tuples[s_mock_tuple_count].type = TUPLE_CSTRING;
        strncpy(s_mock_tuples[s_mock_tuple_count].value[0].cstring, value, sizeof(s_mock_tuples[s_mock_tuple_count].value[0].cstring));
        s_mock_tuples[s_mock_tuple_count].length = strlen(value) + 1;
        s_mock_tuple_count++;
    }
}

// Custom dict_find for the test
Tuple *my_dict_find(const DictionaryIterator *iter, uint32_t key) {
    for (int i = 0; i < s_mock_tuple_count; i++) {
        if (s_mock_tuples[i].message_key == key) {
            return &s_mock_tuples[i];
        }
    }
    return NULL;
}

#include "../../src/c/app_messaging/forecast_series.c"
#include "../../src/c/app_messaging/app_messaging.c"

void setUp(void) {
    memset(&s_test_settings, 0, sizeof(ClaySettings));
    memset(&s_test_weather_data, 0, sizeof(WeatherData));
    reset_mock_dict();
    s_app_message_outbox_send_count = 0;
    s_dict_find_func = my_dict_find;
}

void tearDown(void) {
}

void test_app_messaging_send_app_ready(void) {
    app_messaging_send_app_ready();
    TEST_ASSERT_EQUAL_INT(1, s_app_message_outbox_send_count);
}

void test_inbox_received_settings_update(void) {
    add_mock_string_tuple(MESSAGE_KEY_Theme, "DARK");
    add_mock_string_tuple(MESSAGE_KEY_BackgroundColor, "0xFFFFFF");
    add_mock_string_tuple(MESSAGE_KEY_WeekdayTextColor, "0x0000AA");
    add_mock_string_tuple(MESSAGE_KEY_DateSeparatorColor, "0x000055");
    add_mock_string_tuple(MESSAGE_KEY_WeatherSeparatorColor, "0x0000FF");
    add_mock_int_tuple(MESSAGE_KEY_DateZeroPadding, 1);

    inbox_received_callback(NULL, NULL);

    TEST_ASSERT_EQUAL_STRING("DARK", s_test_settings.ThemeValue);
    TEST_ASSERT_EQUAL_HEX(0xFF, s_test_settings.BackgroundColor.argb);
    TEST_ASSERT_EQUAL_HEX(0xAA, s_test_settings.WeekdayTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(0x55, s_test_settings.DateSeparatorColor.argb);
    TEST_ASSERT_EQUAL_HEX(0xFF, s_test_settings.WeatherSeparatorColor.argb);
    TEST_ASSERT_TRUE(s_test_settings.DateZeroPadding);
    TEST_ASSERT_TRUE(s_test_settings.InitialSyncDone);
}

void test_inbox_received_weather_data(void) {
    add_mock_int_tuple(MESSAGE_KEY_WEATHER_TEMPERATURE_CURRENT, 22);
    add_mock_int_tuple(MESSAGE_KEY_WEATHER_TEMPERATURE_MIN, 15);
    add_mock_int_tuple(MESSAGE_KEY_WEATHER_TEMPERATURE_MAX, 28);
    add_mock_string_tuple(MESSAGE_KEY_WEATHER_CONDITION, "Clear");
    add_mock_string_tuple(MESSAGE_KEY_WEATHER_TEMP_FORECAST_ENCODED, "20,21,22");

    inbox_received_callback(NULL, NULL);

    TEST_ASSERT_EQUAL_INT(22, s_test_weather_data.CurrentTemperature);
    TEST_ASSERT_EQUAL_INT(15, s_test_weather_data.MinTemperature);
    TEST_ASSERT_EQUAL_INT(28, s_test_weather_data.MaxTemperature);
    TEST_ASSERT_EQUAL_STRING("Clear", s_test_weather_data.CurrentConditions);

    TEST_ASSERT_EQUAL_INT(3, s_test_weather_data.TemperatureForecastCount);
    TEST_ASSERT_NOT_NULL(s_test_weather_data.TemperatureForecast);
    TEST_ASSERT_EQUAL_INT(20, s_test_weather_data.TemperatureForecast[0]);
    TEST_ASSERT_EQUAL_INT(21, s_test_weather_data.TemperatureForecast[1]);
    TEST_ASSERT_EQUAL_INT(22, s_test_weather_data.TemperatureForecast[2]);

    // Cleanup dynamic alloc from mock malloc
    if (s_test_weather_data.is_temp_forecast_dynamic_alloc) {
        free(s_test_weather_data.TemperatureForecast);
    }
}

void test_app_messaging_initialize(void) {
    // Should call app_message_open and return success if mock returns OK
    app_messaging_initialize();
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_app_messaging_send_app_ready);
    RUN_TEST(test_inbox_received_settings_update);
    RUN_TEST(test_inbox_received_weather_data);
    RUN_TEST(test_app_messaging_initialize);
    return UNITY_END();
}
