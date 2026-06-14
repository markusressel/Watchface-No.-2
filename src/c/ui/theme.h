#pragma once
#include <pebble.h>
#include "../settings/clay_settings.h"

// Themes
enum ThemeEnum {
    DARK,
    LIGHT,
    CUSTOM
};

typedef struct Theme {
    enum ThemeEnum CurrentThemeEnum;

    // General Colors
    GColor BackgroundColor;

    // Time Layer
    GColor TimeTextColor;
    // Date Layer
    GColor DateTextColor;

    // Battery Bar Layer
    GColor BatteryOutlineColor;
    GColor BatteryFillColor;

    // Weather Layer
    GColor WeatherTextColor;
    GColor WeatherMaxTempColor;
    GColor WeatherCurrentTempColor;
    GColor WeatherMinTempColor;
    GColor WeatherAxisTickColor;
    GColor WeatherIndicatorColor;

    // Stepcount layer
    GColor StepcountTextColor;

    // Heartrate layer
    GColor HeartrateTextColor;

    // Fonts
    GFont TimeFont;
} __attribute__((__packed__)) Theme;

// Theme *theme_create_custom(Theme theme);

Theme *theme_set_fonts(Theme *theme, bool showSeconds);

// Get current Theme struct
Theme *theme_get_theme();

// method to set application theme
// @param theme        one of the ThemeEnum values DARK, LIGHT
//                     if you want to set a custom theme use init_custom_theme();
// @param showSeconds  specifies if the time shows seconds (to properly select font size)
void set_theme(enum ThemeEnum theme, bool showSeconds);

// method to initialize custom theme with custom colors
// @param theme        the custom theme
void set_custom_theme(Theme *theme);

void apply_theme_from_settings(ClaySettings *settings, Window *window);
