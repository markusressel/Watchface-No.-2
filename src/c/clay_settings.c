#include <pebble.h>
#include "clay_settings.h"

// An instance of the struct
static ClaySettings settings;

// Initialize the default settings
static void clay_default_settings() {
  settings.BackgroundColor = GColorWhite;
  settings.ForegroundColor = GColorBlack;
  settings.TextColor = GColorBlack;
  
  // Time Layer
  settings.TimeTextColor = settings.TextColor;
  // Date Layer
  settings.DateTextColor = settings.TextColor;
  
  // Battery Bar Layer
  settings.BatteryFrameColor = settings.ForegroundColor;
  settings.BatteryFillColor = settings.ForegroundColor;
  
  // Weather Layer
  //settings.WeatherIconColor = settings.ForegroundColor;
  settings.WeatherTextColor = settings.TextColor;
  
  /*
  // Heartrate layer
  #ifdef PBL_COLOR
    settings.HeartIconColor = GColorRed;
  #else
    settings.HeartIconColor = settings.ForegroundColor;
  #endif
    settings.HeartrateTextColor = settings.TextColor;
  */
  
  settings.ShowSeconds = false;
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