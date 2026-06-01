#pragma once

// Persistent storage key
#define SETTINGS_KEY 1
#define SETTINGS_VERSION_KEY 2
#define SETTINGS_VERSION 6

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
    int LayoutRowCount; // total rows including time row (clamped per platform)
    int Row0Widget; // row index 0 (top)
    int Row1Widget; // row index 1
    int Row3Widget; // row index 3
    int Row4Widget; // row index 4 (bottom)
    int Row5Widget; // row index 5 (used when LayoutRowCount >= 6)
    int Row6Widget; // row index 6 (used when LayoutRowCount >= 7)

    // Dotted font scaling
    float DotScaleFactor;
    bool DotAutoScale;
} __attribute__((__packed__)

)
ClaySettings;


// get current settings struct
ClaySettings *clay_get_settings();

// save current settings struct to persistent storage
void clay_save_settings();

// load settings from persistent storage
void clay_load_settings();