#include "unity/unity.h"
#include <stdbool.h>
#include <time.h>
#include "pebble/pebble.h"

// Mock dependencies
void status_layer_update() {
}

void weather_request_update() {
}

#include "../../src/c/system/phone_connection.c"

void setUp(void) {
    s_registered = false;
    s_is_connected = false;
    s_connection_handler = NULL;
    s_mock_connected = true;
}

void tearDown(void) {
}

void test_register_phone_connection_listener(void) {
    s_mock_connected = true;
    register_phone_connection_listener();

    TEST_ASSERT_TRUE(s_registered);
    TEST_ASSERT_TRUE(s_is_connected);
    TEST_ASSERT_NOT_NULL(s_connection_handler);

    // Test double registration
    register_phone_connection_listener();
    TEST_ASSERT_TRUE(s_registered);
}

void test_unregister_phone_connection_listener(void) {
    register_phone_connection_listener();
    TEST_ASSERT_TRUE(s_registered);

    unregister_phone_connection_listener();
    TEST_ASSERT_FALSE(s_registered);
    TEST_ASSERT_NULL(s_connection_handler);

    // Test double unregistration
    unregister_phone_connection_listener();
    TEST_ASSERT_FALSE(s_registered);
}

void test_connection_handler_callback(void) {
    register_phone_connection_listener();

    // Simulate disconnect
    s_connection_handler(false);
    TEST_ASSERT_FALSE(phone_connection_is_connected());

    // Simulate reconnect
    s_connection_handler(true);
    TEST_ASSERT_TRUE(phone_connection_is_connected());
}

void test_force_phone_connection_update(void) {
    s_mock_connected = false;
    force_phone_connection_update();
    TEST_ASSERT_FALSE(phone_connection_is_connected());

    s_mock_connected = true;
    force_phone_connection_update();
    TEST_ASSERT_TRUE(phone_connection_is_connected());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_register_phone_connection_listener);
    RUN_TEST(test_unregister_phone_connection_listener);
    RUN_TEST(test_connection_handler_callback);
    RUN_TEST(test_force_phone_connection_update);
    return UNITY_END();
}
