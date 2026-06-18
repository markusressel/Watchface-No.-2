#include "unity/unity.h"
#include <pebble.h>
#include "../../src/c/system/battery_listener.h"

// Mock dependencies
void update_battery_bar() {
}

#include "../../src/c/system/battery_listener.c"

void setUp(void) {
    s_battery_handler = NULL;
    registered = false;
    s_current_battery_level = -1;
}

void tearDown(void) {
}

void test_register_battery_listener(void) {
    register_battery_listener();
    TEST_ASSERT_NOT_NULL(s_battery_handler);
    TEST_ASSERT_TRUE(registered);

    // Register again should ignore
    register_battery_listener();
}

void test_unregister_battery_listener(void) {
    register_battery_listener();
    unregister_battery_listener();
    TEST_ASSERT_NULL(s_battery_handler);
    TEST_ASSERT_FALSE(registered);
}

void test_battery_callback(void) {
    register_battery_listener();

    BatteryChargeState state = {
        .charge_percent = 50,
        .is_charging = true,
        .is_plugged = true
    };

    // Simulate callback
    s_battery_handler(state);

    TEST_ASSERT_EQUAL_INT(50, s_battery_level);
    TEST_ASSERT_TRUE(s_battery_charging);
    TEST_ASSERT_TRUE(s_battery_cable_connected);
    TEST_ASSERT_EQUAL_INT(50, s_current_battery_level);
}

void test_force_battery_update(void) {
    register_battery_listener();

    s_battery_state.charge_percent = 75;
    s_battery_state.is_charging = false;

    force_battery_update();

    TEST_ASSERT_EQUAL_INT(75, s_battery_level);
    TEST_ASSERT_FALSE(s_battery_charging);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_register_battery_listener);
    RUN_TEST(test_unregister_battery_listener);
    RUN_TEST(test_battery_callback);
    RUN_TEST(test_force_battery_update);
    return UNITY_END();
}