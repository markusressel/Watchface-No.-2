#include "unity/unity.h"
#include <pebble.h>

// Define variables that would otherwise be extern
int s_heartrate_bpm;
int s_step_count;

#include "../../src/c/system/health_listener.h"

// Mock dependencies
void update_stepcount() {
}

void update_heartrate() {
}

#include "../../src/c/system/health_listener.c"

void setUp(void) {
    s_health_handler = NULL;
    registered = false;
    s_mock_step_count = 0;
    s_mock_heart_rate = 0;
    s_heartrate_bpm = 0;
    s_step_count = 0;
}

void tearDown(void) {
}

void test_register_health_event_listener(void) {
    s_mock_heart_rate = 70;
    s_mock_step_count = 1000;

    register_health_event_listener();

    TEST_ASSERT_NOT_NULL(s_health_handler);
    TEST_ASSERT_TRUE(registered);
    TEST_ASSERT_EQUAL_INT(70, s_heartrate_bpm);
    TEST_ASSERT_EQUAL_INT(1000, s_step_count);
}

void test_unregister_health_event_listener(void) {
    register_health_event_listener();
    unregister_health_event_listener();

    TEST_ASSERT_NULL(s_health_handler);
    TEST_ASSERT_FALSE(registered);
}

void test_health_handler_heartrate(void) {
    register_health_event_listener();

    s_mock_heart_rate = 85;
    s_health_handler(HealthEventHeartRateUpdate, NULL);

    TEST_ASSERT_EQUAL_INT(85, s_heartrate_bpm);
}

void test_health_handler_movement(void) {
    register_health_event_listener();

    s_mock_step_count = 5000;
    s_health_handler(HealthEventMovementUpdate, NULL);

    TEST_ASSERT_EQUAL_INT(5000, s_step_count);
}

void test_health_handler_significant_update(void) {
    register_health_event_listener();

    s_mock_heart_rate = 90;
    s_mock_step_count = 6000;
    s_health_handler(HealthEventSignificantUpdate, NULL);

    TEST_ASSERT_EQUAL_INT(90, s_heartrate_bpm);
    TEST_ASSERT_EQUAL_INT(6000, s_step_count);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_register_health_event_listener);
    RUN_TEST(test_unregister_health_event_listener);
    RUN_TEST(test_health_handler_heartrate);
    RUN_TEST(test_health_handler_movement);
    RUN_TEST(test_health_handler_significant_update);
    return UNITY_END();
}