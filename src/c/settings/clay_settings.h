#pragma once

#include <pebble.h>
#include <stdbool.h>
#include "persist_keys.h"

// Persistent storage key
#define SETTINGS_KEY PERSIST_KEY_SETTINGS
#define SETTINGS_VERSION_KEY PERSIST_KEY_SETTINGS_VERSION
#define SETTINGS_VERSION 20

// Theme Values
#define THEME_LIGHT_STR "LIGHT"
#define THEME_DARK_STR "DARK"
#define THEME_CUSTOM_STR "CUSTOM"

// Define our settings struct
typedef struct ClaySettings {
    GColor BackgroundColor;
    GColor TimeTextColor;
    GColor DateTextColor;
    GColor BatteryFrameColor;
    GColor BatteryFillColor;
    GColor BatteryLowColor;
    GColor WeatherTextColor;
    GColor WeatherMaxTempColor;
    GColor WeatherCurrentTempColor;
    GColor WeatherMinTempColor;
    GColor WeatherAxisTickColor;
    GColor WeatherIndicatorColor;
#if defined(PBL_COLOR)
    GColor ForecastTempColorM10;
    GColor ForecastTempColor0;
    GColor ForecastTempColor10;
    GColor ForecastTempColor20;
    GColor ForecastTempColor30;
    GColor ForecastTempColor40;
    GColor ForecastRainColor0;
    GColor ForecastRainColor3;
    GColor ForecastRainColor10;
    GColor ForecastRainColor50;
    GColor ForecastRainColor100;
#endif
    GColor StepcountTextColor;
    GColor HeartrateTextColor;
    float DotScaleFactor;
    float TimeRowRatio;
    uint8_t WeatherSlot1;
    uint8_t WeatherSlot2;
    uint8_t WeatherSlot3;
    uint8_t LowBatteryThreshold;
    uint8_t DigitWidth;
    uint8_t DotWidth;
    uint8_t DotHeight;
    uint8_t DotHorizontalGap;
    uint8_t DotVerticalGap;
    uint8_t LayoutRowCount;
    uint8_t Row0Widget;
    uint8_t Row1Widget;
    uint8_t Row2Widget;
    uint8_t Row3Widget;
    uint8_t Row4Widget;
    uint8_t Row5Widget;
    uint8_t Row6Widget;
    uint8_t SliderWeatherForecastPreviewHoursCount;
    uint8_t WeatherUpdateIntervalMinutes;
    char ThemeValue[10];
    bool ShowYear: 1;
    bool ShowSeconds: 1;
    bool ShowWeekdayAbbreviation: 1;
    bool WeekdayAbbreviationUppercase: 1;
    bool DotAutoScale: 1;
    bool WeatherUseSimulation: 1;
    bool InitialSyncDone: 1;
} __attribute__((__packed__)) ClaySettings;

// get current settings struct
ClaySettings *clay_get_settings();

// save current settings struct to persistent storage
ClaySettings *clay_save_settings(ClaySettings *settings);

// load settings from persistent storage
ClaySettings *clay_load_settings();

// print current settings values for debugging
void clay_log_settings_debug(const char *context_label, ClaySettings *settings);