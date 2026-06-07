#pragma once

#include "persist_keys.h"

// Persistent storage key
#define SETTINGS_KEY PERSIST_KEY_SETTINGS
#define SETTINGS_VERSION_KEY PERSIST_KEY_SETTINGS_VERSION
#define SETTINGS_VERSION 10

extern const char *THEME_LIGHT;
extern const char *THEME_DARK;
extern const char *THEME_CUSTOM;

// Define our settings struct
typedef struct ClaySettings {
    GColor BackgroundColor;

    // Time Layer
    GColor TimeTextColor;

    // Date Layer
    GColor DateTextColor;

    // Battery Bar Layer
    GColor BatteryFrameColor;
    GColor BatteryFillColor;

    // Weather Layer
    GColor WeatherTextColor;

    // Stepcount Layer
    GColor StepcountTextColor;

    // Heartrate Layer
    GColor HeartrateTextColor;

    char ThemeValue[10];

    bool ShowYear;
    bool ShowSeconds;
    bool ShowWeekdayAbbreviation;

    // Dotted Font
    int DigitWidth;

    int DotWidth;
    int DotHeight;
    int DotHorizontalGap;
    int DotVerticalGap;

    bool WeekdayAbbreviationUppercase;

    // Row layout (WidgetId values)
    int LayoutRowCount; // total rows including time row
    int Row0Widget; // row index 0
    int Row1Widget; // row index 1
    int Row2Widget; // row index 2
    int Row3Widget; // row index 3
    int Row4Widget; // row index 4
    int Row5Widget; // row index 5 (used when LayoutRowCount >= 6)
    int Row6Widget; // row index 6 (used when LayoutRowCount >= 7)

    // Dotted font scaling
    float DotScaleFactor;
    bool DotAutoScale;

    bool WeatherUseSimulation;
} __attribute__((__packed__)) ClaySettings;

// get current settings struct
ClaySettings *clay_get_settings();

// save current settings struct to persistent storage
ClaySettings *clay_save_settings(ClaySettings *settings);

// load settings from persistent storage
ClaySettings *clay_load_settings();

// print current settings values for debugging
void clay_log_settings_debug(const char *context_label, ClaySettings *settings);
