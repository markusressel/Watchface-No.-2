#include "unity/unity.h"
#include <pebble.h>

// Define variables that would otherwise be extern
int s_heartrate_bpm;
int s_step_count;

#include "../../src/c/system/health_listener.h"

#include "../../src/c/settings/clay_settings.h"
#include "../../src/c/ui/layer/widget.h"
ClaySettings s_settings;
ClaySettings *clay_get_settings() { return &s_settings; }

// Mock dependencies
void update_stepcount() {
}

void update_heartrate() {
}

#include "../../src/c/system/health_listener.c"

void setUp(void) {
    mock_storage_reset();
    s_health_handler = NULL;
    registered = false;
    s_mock_step_count = 0;
    s_mock_heart_rate = 0;
    s_heartrate_bpm = 0;
    s_step_count = 0;

    // By default, enable a health widget so tests register correctly
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.LayoutRowCount = 1;
    s_settings.Row0Widget = WIDGET_HEARTRATE;
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

void test_health_restore_from_persistence(void) {
    // 1. Write key to storage
    persist_write_int(PERSIST_KEY_LAST_HEARTRATE, 72);

    // 2. Register (should restore)
    register_health_event_listener();

    TEST_ASSERT_EQUAL_INT(72, s_heartrate_bpm);
}

void test_health_ignore_zero_heartrate_update(void) {
    register_health_event_listener();
    s_heartrate_bpm = 75;

    // 1. Trigger update with 0
    s_mock_heart_rate = 0;
    s_health_handler(HealthEventHeartRateUpdate, NULL);

    // 2. Value should remain 75 (not overwritten by 0)
    TEST_ASSERT_EQUAL_INT(75, s_heartrate_bpm);
}

void test_health_ignore_zero_heartrate_significant(void) {
    register_health_event_listener();
    s_heartrate_bpm = 75;

    // 1. Trigger update with 0
    s_mock_heart_rate = 0;
    s_health_handler(HealthEventSignificantUpdate, NULL);

    // 2. Value should remain 75 (not overwritten by 0)
    TEST_ASSERT_EQUAL_INT(75, s_heartrate_bpm);
}

void test_health_save_to_persistence_on_unregister(void) {
    register_health_event_listener();
    s_heartrate_bpm = 82;

    // Unregister should save to persistence
    unregister_health_event_listener();

    TEST_ASSERT_TRUE(persist_exists(PERSIST_KEY_LAST_HEARTRATE));
    TEST_ASSERT_EQUAL_INT(82, persist_read_int(PERSIST_KEY_LAST_HEARTRATE));
}

void test_health_does_not_save_zero_to_persistence_on_unregister(void) {
    register_health_event_listener();
    s_heartrate_bpm = 0;

    // Unregister should NOT save 0 to persistence
    unregister_health_event_listener();

    TEST_ASSERT_FALSE(persist_exists(PERSIST_KEY_LAST_HEARTRATE));
}

void test_health_listener_not_registered_if_inactive(void) {
    // 1. Configure settings to NOT have active health widgets
    s_settings.LayoutRowCount = 1;
    s_settings.Row0Widget = WIDGET_WEATHER;

    // 2. Register
    register_health_event_listener();

    // 3. Verify it was NOT registered
    TEST_ASSERT_NULL(s_health_handler);
    TEST_ASSERT_FALSE(registered);
}

void test_health_already_registered(void) {
    register_health_event_listener();
    TEST_ASSERT_TRUE(registered);
    // Call again
    register_health_event_listener();
    TEST_ASSERT_TRUE(registered);
}

void test_health_unregister_not_registered(void) {
    TEST_ASSERT_FALSE(registered);
    unregister_health_event_listener();
    TEST_ASSERT_FALSE(registered);
}

void test_health_subscribe_failure(void) {
    s_mock_health_subscribe_success = false;
    register_health_event_listener();
    TEST_ASSERT_NULL(s_health_handler);
    // Clean up
    s_mock_health_subscribe_success = true;
}

void test_health_handler_unhandled_events(void) {
    register_health_event_listener();
    s_health_handler(HealthEventSleepUpdate, NULL);
    s_health_handler(HealthEventMetricAlert, NULL);
    s_health_handler((HealthEventType)999, NULL); // default case
}

void test_health_widget_active_other_rows(void) {
    // Test row 1
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.LayoutRowCount = 2;
    s_settings.Row0Widget = WIDGET_WEATHER;
    s_settings.Row1Widget = WIDGET_STEPCOUNT;
    register_health_event_listener();
    TEST_ASSERT_TRUE(registered);
    unregister_health_event_listener();

    // Test row 2
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.LayoutRowCount = 3;
    s_settings.Row2Widget = WIDGET_HEARTRATE;
    register_health_event_listener();
    TEST_ASSERT_TRUE(registered);
    unregister_health_event_listener();

    // Test row 3
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.LayoutRowCount = 4;
    s_settings.Row3Widget = WIDGET_HEARTRATE;
    register_health_event_listener();
    TEST_ASSERT_TRUE(registered);
    unregister_health_event_listener();

    // Test row 4
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.LayoutRowCount = 5;
    s_settings.Row4Widget = WIDGET_HEARTRATE;
    register_health_event_listener();
    TEST_ASSERT_TRUE(registered);
    unregister_health_event_listener();

    // Test row 5
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.LayoutRowCount = 6;
    s_settings.Row5Widget = WIDGET_HEARTRATE;
    register_health_event_listener();
    TEST_ASSERT_TRUE(registered);
    unregister_health_event_listener();

    // Test row 6
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.LayoutRowCount = 7;
    s_settings.Row6Widget = WIDGET_HEARTRATE;
    register_health_event_listener();
    TEST_ASSERT_TRUE(registered);
    unregister_health_event_listener();

    // Test out-of-bounds row (default case)
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.LayoutRowCount = 8;
    register_health_event_listener();
    TEST_ASSERT_FALSE(registered);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_register_health_event_listener);
    RUN_TEST(test_unregister_health_event_listener);
    RUN_TEST(test_health_handler_heartrate);
    RUN_TEST(test_health_handler_movement);
    RUN_TEST(test_health_handler_significant_update);
    RUN_TEST(test_health_restore_from_persistence);
    RUN_TEST(test_health_ignore_zero_heartrate_update);
    RUN_TEST(test_health_ignore_zero_heartrate_significant);
    RUN_TEST(test_health_save_to_persistence_on_unregister);
    RUN_TEST(test_health_does_not_save_zero_to_persistence_on_unregister);
    RUN_TEST(test_health_listener_not_registered_if_inactive);
    RUN_TEST(test_health_already_registered);
    RUN_TEST(test_health_unregister_not_registered);
    RUN_TEST(test_health_subscribe_failure);
    RUN_TEST(test_health_handler_unhandled_events);
    RUN_TEST(test_health_widget_active_other_rows);
    return UNITY_END();
}