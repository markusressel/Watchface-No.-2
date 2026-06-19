#include "unity/unity.h"
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// 1. Include the mock pebble.h provided by your test environment.
#include <pebble.h>

// 2. Include the headers and source file to be tested.
#include "../../src/c/settings/clay_settings.h"
#include "../../src/c/settings/clay_settings.c"

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

    ClaySettings *saved_settings = clay_reset_to_default_settings();
    saved_settings->LayoutRowCount = layout_row_count_max_for_platform();
    saved_settings->ShowSeconds = true;
    clay_save_settings(saved_settings);

    // Act
    ClaySettings *loaded_settings = clay_load_settings();

    // Assert: Check that settings were loaded from storage
    TEST_ASSERT_EQUAL_INT(layout_row_count_max_for_platform(), loaded_settings->LayoutRowCount);
    TEST_ASSERT_TRUE(loaded_settings->ShowSeconds);
}

void test_clay_battery_settings_defaults(void) {
    // Act
    ClaySettings *settings = clay_reset_to_default_settings();

    // Assert
    TEST_ASSERT_EQUAL_INT(30, settings->LowBatteryThreshold);
    TEST_ASSERT_EQUAL_UINT8(GColorRed.argb, settings->BatteryLowColor.argb);

#if defined(PBL_COLOR)
    TEST_ASSERT_EQUAL_UINT8(GColorBlueMoon.argb, settings->ForecastTempColorM10.argb);
    TEST_ASSERT_EQUAL_UINT8(GColorPictonBlue.argb, settings->ForecastTempColor0.argb);
    TEST_ASSERT_EQUAL_UINT8(GColorGreen.argb, settings->ForecastTempColor10.argb);
    TEST_ASSERT_EQUAL_UINT8(GColorChromeYellow.argb, settings->ForecastTempColor20.argb);
    TEST_ASSERT_EQUAL_UINT8(GColorOrange.argb, settings->ForecastTempColor30.argb);
    TEST_ASSERT_EQUAL_UINT8(GColorRed.argb, settings->ForecastTempColor40.argb);

    TEST_ASSERT_EQUAL_UINT8(GColorLightGray.argb, settings->ForecastRainColor0.argb);
    TEST_ASSERT_EQUAL_UINT8(GColorPictonBlue.argb, settings->ForecastRainColor3.argb);
    TEST_ASSERT_EQUAL_UINT8(GColorBlueMoon.argb, settings->ForecastRainColor10.argb);
    TEST_ASSERT_EQUAL_UINT8(GColorBlue.argb, settings->ForecastRainColor50.argb);
    TEST_ASSERT_EQUAL_UINT8(GColorDukeBlue.argb, settings->ForecastRainColor100.argb);
#endif
}

void test_clay_load_settings_detects_version_change_and_resets_sync(void) {
    mock_storage_set_version(SETTINGS_VERSION);

    ClaySettings *saved_settings = clay_reset_to_default_settings();
    saved_settings->InitialSyncDone = true;
    clay_save_settings(saved_settings);

    // Set stored version to a different value
    persist_write_string(PERSIST_KEY_APP_VERSION, "1.0.0-old");

    // Act
    ClaySettings *loaded_settings = clay_load_settings();

    // Assert
    TEST_ASSERT_FALSE(loaded_settings->InitialSyncDone);

    char stored_version[64] = {0};
    persist_read_string(PERSIST_KEY_APP_VERSION, stored_version, sizeof(stored_version));
    TEST_ASSERT_EQUAL_STRING(WF_APP_VERSION, stored_version);
}

void test_clay_load_settings_retains_sync_when_version_is_same(void) {
    mock_storage_set_version(SETTINGS_VERSION);

    ClaySettings *saved_settings = clay_reset_to_default_settings();
    saved_settings->InitialSyncDone = true;
    clay_save_settings(saved_settings);

    // Set stored version to the current version
    persist_write_string(PERSIST_KEY_APP_VERSION, WF_APP_VERSION);

    // Act
    ClaySettings *loaded_settings = clay_load_settings();

    // Assert
    TEST_ASSERT_TRUE(loaded_settings->InitialSyncDone);
}


int main() {
    UNITY_BEGIN();
    // RUN_TEST(test_clay_load_settings_migrates_old_version);
    RUN_TEST(test_clay_load_settings_with_valid_version);
    RUN_TEST(test_clay_battery_settings_defaults);
    RUN_TEST(test_clay_load_settings_detects_version_change_and_resets_sync);
    RUN_TEST(test_clay_load_settings_retains_sync_when_version_is_same);
    return UNITY_END();
}