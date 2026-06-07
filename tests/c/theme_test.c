#include "unity/unity.h"

#include <stdio.h>   // For printf (still useful for general output)
#include <stdbool.h> // For bool
#include <string.h>  // For strcmp
#include <pebble.h>  // Include pebble.h for GColor, GFont, etc.

#include "../../src/c/theme.h" // Include the header for the functions being tested
#include "../../src/c/theme.c" // Include the C implementation file directly for testing

// Unity setup and teardown functions (required even if empty)
void setUp(void) {
}

void tearDown(void) {
}

// Test function for init_theme with DARK theme
void test_init_theme_dark(void) {
    init_theme(DARK, false);
    Theme *theme = theme_get_theme();

    TEST_ASSERT_EQUAL(DARK, theme->CurrentThemeEnum);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->BackgroundColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->TimeTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->DateTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->BatteryOutlineColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->BatteryFillColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->WeatherTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->StepcountTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->HeartrateTextColor.argb);
}

// Test function for init_theme with LIGHT theme
void test_init_theme_light(void) {
    init_theme(LIGHT, false);
    Theme *theme = theme_get_theme();

    TEST_ASSERT_EQUAL(LIGHT, theme->CurrentThemeEnum);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->BackgroundColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->TimeTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->DateTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->BatteryOutlineColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->BatteryFillColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->WeatherTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->StepcountTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->HeartrateTextColor.argb);
}

// Test function for init_custom_theme
void test_init_custom_theme(void) {
    Theme custom_theme;
    custom_theme.CurrentThemeEnum = CUSTOM;
    custom_theme.BackgroundColor = GColorRed;
    custom_theme.TimeTextColor = GColorGreen;
    custom_theme.DateTextColor = GColorBlue;
    custom_theme.BatteryOutlineColor = GColorYellow;
    custom_theme.BatteryFillColor = GColorCyan;
    custom_theme.WeatherTextColor = GColorMagenta;
    custom_theme.StepcountTextColor = GColorOrange;
    custom_theme.HeartrateTextColor = GColorPurple;
    theme_set_fonts(&custom_theme, false);

    set_custom_theme(&custom_theme);
    Theme *theme = theme_get_theme();

    TEST_ASSERT_EQUAL(CUSTOM, theme->CurrentThemeEnum);
    TEST_ASSERT_EQUAL_HEX(GColorRed.argb, theme->BackgroundColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorGreen.argb, theme->TimeTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlue.argb, theme->DateTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorYellow.argb, theme->BatteryOutlineColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorCyan.argb, theme->BatteryFillColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorMagenta.argb, theme->WeatherTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorOrange.argb, theme->StepcountTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorPurple.argb, theme->HeartrateTextColor.argb);
}

// Test font selection with showSeconds = true
void test_init_theme_show_seconds_true(void) {
    init_theme(DARK, true);
    Theme *theme = theme_get_theme();
    // Assuming a specific font is loaded when showSeconds is true
    // This might require knowing the exact font loaded by the Pebble SDK
    // For now, we can just assert it's not NULL if a font is expected.
    // In a real scenario, you might compare font pointers if possible.
    TEST_ASSERT_NOT_NULL(theme->TimeFont);
}

// Test font selection with showSeconds = false
void test_init_theme_show_seconds_false(void) {
    init_theme(DARK, false);
    Theme *theme = theme_get_theme();
    // Assuming a specific font is loaded when showSeconds is false
    TEST_ASSERT_NOT_NULL(theme->TimeFont);
}


int main() {
    UNITY_BEGIN();
    RUN_TEST(test_init_theme_dark);
    RUN_TEST(test_init_theme_light);
    RUN_TEST(test_init_custom_theme);
    RUN_TEST(test_init_theme_show_seconds_true);
    RUN_TEST(test_init_theme_show_seconds_false);
    return UNITY_END();
}
