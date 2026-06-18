#include "unity/unity.h"
#include <stdbool.h>
#include <time.h>
#include "pebble/pebble.h"

// Mock dependencies
#include "../../src/c/settings/clay_settings.h"
ClaySettings s_settings;
ClaySettings *clay_get_settings() { return &s_settings; }

static int s_update_date_count = 0;
void update_date() { s_update_date_count++; }

static int s_update_time_count = 0;
void update_time_layer() { s_update_time_count++; }

static int s_weather_tick_count = 0;
void weather_tick_update() { s_weather_tick_count++; }

static int s_weather_forecast_tick_count = 0;
void weather_forecast_tick_update() { s_weather_forecast_tick_count++; }

#include "../../src/c/system/tick_listener.c"

void setUp(void) {
    registered = false;
    s_tick_handler = NULL;
    s_tick_units = 0;
    s_update_date_count = 0;
    s_update_time_count = 0;
    s_weather_tick_count = 0;
    s_weather_forecast_tick_count = 0;
    memset(&s_settings, 0, sizeof(ClaySettings));
}

void tearDown(void) {
}

void test_register_tick_listener_minutes(void) {
    s_settings.ShowSeconds = false;
    register_tick_listener();

    TEST_ASSERT_TRUE(registered);
    TEST_ASSERT_NOT_NULL(s_tick_handler);
    TEST_ASSERT_EQUAL(MINUTE_UNIT, s_tick_units);
}

void test_register_tick_listener_seconds(void) {
    s_settings.ShowSeconds = true;
    register_tick_listener();

    TEST_ASSERT_TRUE(registered);
    TEST_ASSERT_NOT_NULL(s_tick_handler);
    TEST_ASSERT_EQUAL(SECOND_UNIT, s_tick_units);
}

void test_unregister_tick_listener(void) {
    register_tick_listener();
    TEST_ASSERT_TRUE(registered);

    unregister_tick_listener();
    TEST_ASSERT_FALSE(registered);
    TEST_ASSERT_NULL(s_tick_handler);
}

void test_tick_handler_callback_minute(void) {
    register_tick_listener();

    struct tm dummy_time = {0};
    s_tick_handler(&dummy_time, MINUTE_UNIT);

    TEST_ASSERT_EQUAL_INT(0, s_update_date_count);
    TEST_ASSERT_EQUAL_INT(1, s_update_time_count);
    TEST_ASSERT_EQUAL_INT(1, s_weather_tick_count);
    TEST_ASSERT_EQUAL_INT(1, s_weather_forecast_tick_count);
}

void test_tick_handler_callback_day(void) {
    register_tick_listener();

    struct tm dummy_time = {0};
    s_tick_handler(&dummy_time, DAY_UNIT | MINUTE_UNIT);

    TEST_ASSERT_EQUAL_INT(1, s_update_date_count);
    TEST_ASSERT_EQUAL_INT(1, s_update_time_count);
    TEST_ASSERT_EQUAL_INT(1, s_weather_tick_count);
    TEST_ASSERT_EQUAL_INT(1, s_weather_forecast_tick_count);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_register_tick_listener_minutes);
    RUN_TEST(test_register_tick_listener_seconds);
    RUN_TEST(test_unregister_tick_listener);
    RUN_TEST(test_tick_handler_callback_minute);
    RUN_TEST(test_tick_handler_callback_day);
    return UNITY_END();
}