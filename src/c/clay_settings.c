#include <pebble.h>
#include "clay_settings.h"

// An instance of the struct
static ClaySettings settings;

static bool is_row_widget_valid(int widget) {
  return widget == 0 || widget == 1 || widget == 3 || widget == 4;
}

static void clay_sanitize_settings() {
  if (settings.TimeFontSize < 20 || settings.TimeFontSize > 60) {
    settings.TimeFontSize = 42;
  }
  settings.TimeFontAutoSize = settings.TimeFontAutoSize ? true : false;

  if (settings.DigitWidth < 3 || settings.DigitWidth > 5) {
    settings.DigitWidth = 4;
  }

  if (settings.DotWidth < 1 || settings.DotWidth > 5) settings.DotWidth = 3;
  if (settings.DotHeight < 1 || settings.DotHeight > 5) settings.DotHeight = 3;
  if (settings.DotHorizontalGap < 1 || settings.DotHorizontalGap > 5) settings.DotHorizontalGap = 3;
  if (settings.DotVerticalGap < 1 || settings.DotVerticalGap > 5) settings.DotVerticalGap = 3;

  if (settings.DotScaleFactor <= 0.0f || settings.DotScaleFactor > 4.0f) {
    settings.DotScaleFactor = 1.0f;
  }
  settings.DotAutoScale = settings.DotAutoScale ? true : false;

  if (!is_row_widget_valid(settings.Row0Widget)) settings.Row0Widget = 0;
  if (!is_row_widget_valid(settings.Row1Widget)) settings.Row1Widget = 1;
  if (!is_row_widget_valid(settings.Row3Widget)) settings.Row3Widget = 3;
  if (!is_row_widget_valid(settings.Row4Widget)) settings.Row4Widget = 4;
}

// Initialize the default settings
// Note: Defaults are also set in the configPage.json, keep them in sync!
static void clay_default_settings() {
  settings.BackgroundColor = GColorWhite;
  GColor foregroundColor = GColorBlack;
  GColor textColor = GColorBlack;

  // Time Layer
  settings.TimeTextColor = textColor;
  settings.TimeFontSize = 42;
  settings.TimeFontAutoSize = true;
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

  // Migrate/reset settings when struct layout changes across versions.
  if (!persist_exists(SETTINGS_VERSION_KEY) ||
      persist_read_int(SETTINGS_VERSION_KEY) != SETTINGS_VERSION) {
    clay_save_settings();
    return;
  }

  if (persist_exists(SETTINGS_KEY)) {
    const int bytes = persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
    if (bytes != sizeof(settings)) {
      clay_default_settings();
      clay_save_settings();
      return;
    }
  }

  clay_sanitize_settings();
}

void clay_save_settings() {
  // save ClaySettings struct to persistent storage
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  persist_write_int(SETTINGS_VERSION_KEY, SETTINGS_VERSION);
}
