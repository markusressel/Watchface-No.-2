#pragma once

// Persistent storage key
#define SETTINGS_KEY 1

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

    // Row layout (WidgetId values; row 2 is always WIDGET_TIME)
    int Row0Widget; // row index 0 (top)
    int Row1Widget; // row index 1
    int Row3Widget; // row index 3
    int Row4Widget; // row index 4 (bottom)
} __attribute__((__packed__)) ClaySettings;


// get current settings struct
ClaySettings *clay_get_settings();

// save current settings struct to persistent storage
void clay_save_settings();

// load settings from persistent storage
void clay_load_settings();
