#include "unity/unity.h"
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// 1. Include the mock pebble.h provided by your test environment.
#include <pebble.h>

// 2. Include the headers and source file to be tested.
#include "../../src/c/clay_settings.h"
#include "../../src/c/clay_settings.c"

// --- Mocks ---
void app_messaging_request_settings() {
    // Mock implementation
}

// --- Test Setup ---
void setUp(void) {
    // The mock_storage_reset() function will be defined in the mock pebble.h
    mock_storage_reset();
}

void tearDown(void) {
}


// --- Tests ---

void test_clay_load_settings_migrates_old_version(void) {
    // Arrange: Simulate old version data exists. These setters will be in pebble.h
    mock_storage_set_version(9);

    ClaySettings old_settings;
    memset(&old_settings, 0, sizeof(ClaySettings));
    old_settings.LayoutRowCount = 99; // Invalid value
    mock_storage_set_data(&old_settings, sizeof(ClaySettings));

    // Act
    ClaySettings *loaded_settings = clay_load_settings();

    // Assert: Check that settings were reset to defaults and version was updated
    TEST_ASSERT_EQUAL_INT(layout_row_count_max_for_platform(), loaded_settings->LayoutRowCount);
    TEST_ASSERT_EQUAL_INT(SETTINGS_VERSION, mock_storage_get_version());
}

void test_clay_load_settings_with_valid_version(void) {
    // Arrange: Simulate current version data exists
    mock_storage_set_version(SETTINGS_VERSION);

    clay_default_settings();
    ClaySettings *saved_settings = clay_get_settings();
    saved_settings->LayoutRowCount = layout_row_count_max_for_platform();
    saved_settings->ShowSeconds = true;
    clay_save_settings();
    // mock_storage_set_data(&saved_settings, sizeof(ClaySettings));

    // Act
    ClaySettings *loaded_settings = clay_load_settings();

    // Assert: Check that settings were loaded from storage
    TEST_ASSERT_EQUAL_INT(layout_row_count_max_for_platform(), loaded_settings->LayoutRowCount);
    TEST_ASSERT_TRUE(loaded_settings->ShowSeconds);
}


int main() {
    UNITY_BEGIN();
    RUN_TEST(test_clay_load_settings_migrates_old_version);
    RUN_TEST(test_clay_load_settings_with_valid_version);
    return UNITY_END();
}
