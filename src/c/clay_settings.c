#include <pebble.h>
#include "clay_settings.h"

// An instance of the struct
static ClaySettings settings;

// Initialize the default settings
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
  settings.SmallerDigits = false;
  strcpy(settings.ThemeValue, "LIGHT");
}

ClaySettings* clay_get_settings() {
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