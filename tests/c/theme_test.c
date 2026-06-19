#include "unity/unity.h"

#include <stdio.h>   // For printf (still useful for general output)
#include <stdbool.h> // For bool
#include <string.h>  // For strcmp
#include <pebble.h>  // Include pebble.h for GColor, GFont, etc.

#include "../../src/c/ui/theme.h"
#include "../../src/c/ui/theme.c" // Include the C implementation file directly for testing

// Unity setup and teardown functions (required even if empty)
void setUp(void) {
}

void tearDown(void) {
}

// Test function for set_theme with DARK theme
void test_set_theme_dark(void) {
    set_theme(DARK, false);
    Theme *theme = theme_get_theme();

    TEST_ASSERT_EQUAL(DARK, theme->CurrentThemeEnum);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->BackgroundColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->TimeTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->DateTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->WeekdayTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->DateSeparatorColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->BatteryOutlineColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->BatteryFillColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorRed.argb, theme->BatteryLowColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->WeatherTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorDarkGray.argb, theme->WeatherAxisTickColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->WeatherIndicatorColor.argb);

#if defined(PBL_COLOR)
    TEST_ASSERT_EQUAL_HEX(GColorBlueMoon.argb, theme->ForecastTempColorM10.argb);
    TEST_ASSERT_EQUAL_HEX(GColorPictonBlue.argb, theme->ForecastTempColor0.argb);
    TEST_ASSERT_EQUAL_HEX(GColorGreen.argb, theme->ForecastTempColor10.argb);
    TEST_ASSERT_EQUAL_HEX(GColorChromeYellow.argb, theme->ForecastTempColor20.argb);
    TEST_ASSERT_EQUAL_HEX(GColorOrange.argb, theme->ForecastTempColor30.argb);
    TEST_ASSERT_EQUAL_HEX(GColorRed.argb, theme->ForecastTempColor40.argb);

    TEST_ASSERT_EQUAL_HEX(GColorLightGray.argb, theme->ForecastRainColor0.argb);
    TEST_ASSERT_EQUAL_HEX(GColorPictonBlue.argb, theme->ForecastRainColor3.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlueMoon.argb, theme->ForecastRainColor10.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlue.argb, theme->ForecastRainColor50.argb);
    TEST_ASSERT_EQUAL_HEX(GColorDukeBlue.argb, theme->ForecastRainColor100.argb);
#endif

    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->StepcountTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->HeartrateTextColor.argb);
}

// Test function for set_theme with LIGHT theme
void test_set_theme_light(void) {
    set_theme(LIGHT, false);
    Theme *theme = theme_get_theme();

    TEST_ASSERT_EQUAL(LIGHT, theme->CurrentThemeEnum);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->BackgroundColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->TimeTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->DateTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->WeekdayTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->DateSeparatorColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->BatteryOutlineColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->BatteryFillColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorRed.argb, theme->BatteryLowColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->WeatherTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorDarkGray.argb, theme->WeatherAxisTickColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->WeatherIndicatorColor.argb);

#if defined(PBL_COLOR)
    TEST_ASSERT_EQUAL_HEX(GColorBlueMoon.argb, theme->ForecastTempColorM10.argb);
    TEST_ASSERT_EQUAL_HEX(GColorPictonBlue.argb, theme->ForecastTempColor0.argb);
    TEST_ASSERT_EQUAL_HEX(GColorGreen.argb, theme->ForecastTempColor10.argb);
    TEST_ASSERT_EQUAL_HEX(GColorChromeYellow.argb, theme->ForecastTempColor20.argb);
    TEST_ASSERT_EQUAL_HEX(GColorOrange.argb, theme->ForecastTempColor30.argb);
    TEST_ASSERT_EQUAL_HEX(GColorRed.argb, theme->ForecastTempColor40.argb);

    TEST_ASSERT_EQUAL_HEX(GColorLightGray.argb, theme->ForecastRainColor0.argb);
    TEST_ASSERT_EQUAL_HEX(GColorPictonBlue.argb, theme->ForecastRainColor3.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlueMoon.argb, theme->ForecastRainColor10.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlue.argb, theme->ForecastRainColor50.argb);
    TEST_ASSERT_EQUAL_HEX(GColorDukeBlue.argb, theme->ForecastRainColor100.argb);
#endif

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
    custom_theme.WeekdayTextColor = GColorOrange;
    custom_theme.DateSeparatorColor = GColorMagenta;
    custom_theme.BatteryOutlineColor = GColorYellow;
    custom_theme.BatteryFillColor = GColorCyan;
    custom_theme.BatteryLowColor = GColorVividCerulean;
    custom_theme.WeatherTextColor = GColorMagenta;
    custom_theme.WeatherAxisTickColor = GColorDarkGray;
    custom_theme.WeatherIndicatorColor = GColorWhite;

#if defined(PBL_COLOR)
    custom_theme.ForecastTempColorM10 = GColorBlueMoon;
    custom_theme.ForecastTempColor0 = GColorPictonBlue;
    custom_theme.ForecastTempColor10 = GColorGreen;
    custom_theme.ForecastTempColor20 = GColorChromeYellow;
    custom_theme.ForecastTempColor30 = GColorOrange;
    custom_theme.ForecastTempColor40 = GColorRed;

    custom_theme.ForecastRainColor0 = GColorLightGray;
    custom_theme.ForecastRainColor3 = GColorPictonBlue;
    custom_theme.ForecastRainColor10 = GColorBlueMoon;
    custom_theme.ForecastRainColor50 = GColorBlue;
    custom_theme.ForecastRainColor100 = GColorDukeBlue;
#endif

    custom_theme.StepcountTextColor = GColorOrange;
    custom_theme.HeartrateTextColor = GColorPurple;
    theme_set_fonts(&custom_theme, false);

    set_custom_theme(&custom_theme);
    Theme *theme = theme_get_theme();

    TEST_ASSERT_EQUAL(CUSTOM, theme->CurrentThemeEnum);
    TEST_ASSERT_EQUAL_HEX(GColorRed.argb, theme->BackgroundColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorGreen.argb, theme->TimeTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlue.argb, theme->DateTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorOrange.argb, theme->WeekdayTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorMagenta.argb, theme->DateSeparatorColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorYellow.argb, theme->BatteryOutlineColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorCyan.argb, theme->BatteryFillColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorMagenta.argb, theme->WeatherTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorOrange.argb, theme->StepcountTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorPurple.argb, theme->HeartrateTextColor.argb);
}

// Test font selection with showSeconds = true
void test_set_theme_show_seconds_true(void) {
    set_theme(DARK, true);
    Theme *theme = theme_get_theme();
    // Assuming a specific font is loaded when showSeconds is true
    // This might require knowing the exact font loaded by the Pebble SDK
    // For now, we can just assert it's not NULL if a font is expected.
    // In a real scenario, you might compare font pointers if possible.
    TEST_ASSERT_NOT_NULL(theme->TimeFont);
}

// Test font selection with showSeconds = false
void test_set_theme_show_seconds_false(void) {
    set_theme(DARK, false);
    Theme *theme = theme_get_theme();
    // Assuming a specific font is loaded when showSeconds is false
    TEST_ASSERT_NOT_NULL(theme->TimeFont);
}

// Test apply_theme_from_settings with LIGHT theme
void test_apply_theme_from_settings_light(void) {
    ClaySettings settings;
    memset(&settings, 0, sizeof(ClaySettings));
    strcpy(settings.ThemeValue, THEME_LIGHT_STR);
    settings.ShowSeconds = false;

    apply_theme_from_settings(&settings, NULL);
    Theme *theme = theme_get_theme();

    TEST_ASSERT_EQUAL(LIGHT, theme->CurrentThemeEnum);
    TEST_ASSERT_EQUAL_HEX(GColorWhite.argb, theme->BackgroundColor.argb);
}

// Test apply_theme_from_settings with DARK theme
void test_apply_theme_from_settings_dark(void) {
    ClaySettings settings;
    memset(&settings, 0, sizeof(ClaySettings));
    strcpy(settings.ThemeValue, THEME_DARK_STR);
    settings.ShowSeconds = true;

    apply_theme_from_settings(&settings, NULL);
    Theme *theme = theme_get_theme();

    TEST_ASSERT_EQUAL(DARK, theme->CurrentThemeEnum);
    TEST_ASSERT_EQUAL_HEX(GColorBlack.argb, theme->BackgroundColor.argb);
}

// Test apply_theme_from_settings with CUSTOM theme
void test_apply_theme_from_settings_custom(void) {
    ClaySettings settings;
    memset(&settings, 0, sizeof(ClaySettings));
    strcpy(settings.ThemeValue, THEME_CUSTOM_STR);
    settings.BackgroundColor = GColorRed;
    settings.TimeTextColor = GColorGreen;
    settings.WeekdayTextColor = GColorBlue;
    settings.DateSeparatorColor = GColorOrange;
    settings.ShowSeconds = false;

    apply_theme_from_settings(&settings, NULL);
    Theme *theme = theme_get_theme();

    TEST_ASSERT_EQUAL(CUSTOM, theme->CurrentThemeEnum);
    TEST_ASSERT_EQUAL_HEX(GColorRed.argb, theme->BackgroundColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorGreen.argb, theme->TimeTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorBlue.argb, theme->WeekdayTextColor.argb);
    TEST_ASSERT_EQUAL_HEX(GColorOrange.argb, theme->DateSeparatorColor.argb);
}

// Test apply_theme_from_settings with default (empty) theme value
void test_apply_theme_from_settings_default(void) {
    ClaySettings settings;
    memset(&settings, 0, sizeof(ClaySettings));
    settings.ThemeValue[0] = '\0';

    apply_theme_from_settings(&settings, NULL);
    Theme *theme = theme_get_theme();

    // Should fallback to LIGHT according to theme.c logic
    TEST_ASSERT_EQUAL(LIGHT, theme->CurrentThemeEnum);
    TEST_ASSERT_EQUAL_STRING(THEME_LIGHT_STR, settings.ThemeValue);
}


int main() {
    UNITY_BEGIN();
    RUN_TEST(test_set_theme_dark);
    RUN_TEST(test_set_theme_light);
    RUN_TEST(test_init_custom_theme);
    RUN_TEST(test_set_theme_show_seconds_true);
    RUN_TEST(test_set_theme_show_seconds_false);
    RUN_TEST(test_apply_theme_from_settings_light);
    RUN_TEST(test_apply_theme_from_settings_dark);
    RUN_TEST(test_apply_theme_from_settings_custom);
    RUN_TEST(test_apply_theme_from_settings_default);
    return UNITY_END();
}