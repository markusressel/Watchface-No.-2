#include <pebble.h>
#include "clay_settings.h"

// An instance of the struct
static ClaySettings settings;

// Initialize the default settings
// Note: Defaults are also set in the configPage.json, keep them in sync!
static void clay_default_settings() {
  settings.BackgroundColor = GColorWhite;
  GColor foregroundColor = GColorBlack;
  GColor textColor = GColorBlack;

  // Time Layer
  settings.TimeTextColor = textColor;
  // Date Layer
  settings.DateTextColor = textColor;

  // Battery Bar Layer
  settings.BatteryFrameColor = foregroundColor;
  settings.BatteryFillColor = foregroundColor;

  // Weather Layer
  settings.WeatherTextColor = textColor;

  // Stepcount layer
  settings.StepcountTextColor = textColor;

  settings.ShowYear = false;
  settings.ShowSeconds = false;
  settings.ShowWeekdayAbbreviation = false;
  settings.WeekdayAbbreviationUppercase = false;
  strcpy(settings.ThemeValue, "LIGHT");

  settings.DigitWidth = 4;

  settings.DotWidth = 3;
  settings.DotHeight = 3;
  settings.DotHorizontalGap = 3;
  settings.DotVerticalGap = 3;
  settings.DotScaleFactor = 1.0f;
  settings.DotAutoScale = true;

  // Row layout defaults: Weather, Date, [TIME fixed], Stepcount, Battery
  settings.Row0Widget = 0; // WIDGET_WEATHER
  settings.Row1Widget = 1; // WIDGET_DATE
  settings.Row3Widget = 3; // WIDGET_STEPCOUNT
  settings.Row4Widget = 4; // WIDGET_BATTERY_BAR
}

ClaySettings *clay_get_settings() {
  return &settings;
}

// Read settings from persistent storage
void clay_load_settings() {
  // Load the default settings
  clay_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

void clay_save_settings() {
  // save ClaySettings struct to persistent storage
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}
