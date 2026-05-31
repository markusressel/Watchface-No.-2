#include <pebble.h>
#include <stdlib.h>
#include "theme.h"

static Theme s_theme;
static enum ThemeEnum currentTheme;

typedef struct FontCandidate {
  int size;
  const char *key;
} FontCandidate;

static int default_time_font_size_for_screen(bool showSeconds, GSize screenSize) {
  if (showSeconds) {
    if (screenSize.h >= 220 || screenSize.w >= 180) return 49;
    if (screenSize.h >= 180) return 42;
    return 36;
  }

  if (screenSize.h >= 220 || screenSize.w >= 180) return 60;
  if (screenSize.h >= 180) return 49;
  return 42;
}

static int pick_closest_index(const FontCandidate *candidates, int candidate_count, int requested_size) {
  int closest = 0;
  int min_diff = abs(candidates[0].size - requested_size);
  for (int i = 1; i < candidate_count; i++) {
    int diff = abs(candidates[i].size - requested_size);
    if (diff < min_diff) {
      min_diff = diff;
      closest = i;
    }
  }
  return closest;
}

static bool font_fits_width(GFont font, const char *sample, int width) {
  GSize text_size = graphics_text_layout_get_content_size(
      sample,
      font,
      GRect(0, 0, width, 200),
      GTextOverflowModeTrailingEllipsis,
      GTextAlignmentRight
  );
  return text_size.w <= width;
}

static GFont pick_font_for_size(
    const FontCandidate *candidates,
    int candidate_count,
    int requested_size,
    const char *sample,
    int max_width
) {
  int idx = pick_closest_index(candidates, candidate_count, requested_size);

  for (int i = idx; i >= 0; i--) {
    GFont font = fonts_get_system_font(candidates[i].key);
    if (font_fits_width(font, sample, max_width)) {
      return font;
    }
  }

  return fonts_get_system_font(candidates[0].key);
}

static void set_fonts(bool showSeconds, int timeFontSize, bool autoTimeFontSize, GSize screenSize) {
  static const FontCandidate time_fonts[] = {
      {20, FONT_KEY_LECO_20_BOLD_NUMBERS},
      {26, FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM},
      {32, FONT_KEY_LECO_32_BOLD_NUMBERS},
      {36, FONT_KEY_LECO_36_BOLD_NUMBERS},
      {38, FONT_KEY_LECO_38_BOLD_NUMBERS},
      {42, FONT_KEY_LECO_42_NUMBERS},
      {44, FONT_KEY_BITHAM_42_MEDIUM_NUMBERS},
      {49, FONT_KEY_ROBOTO_BOLD_SUBSET_49},
#ifdef FONT_KEY_LECO_60_BOLD_NUMBERS_AM_PM
      {60, FONT_KEY_LECO_60_BOLD_NUMBERS_AM_PM},
#endif
  };

  int requested_size = autoTimeFontSize
                           ? default_time_font_size_for_screen(showSeconds, screenSize)
                           : timeFontSize;
  const char *sample = showSeconds ? "00:00:00" : "00:00";
  s_theme.TimeFont = pick_font_for_size(
      time_fonts,
      sizeof(time_fonts) / sizeof(time_fonts[0]),
      requested_size,
      sample,
      screenSize.w
  );
}

static void set_colors() {
  GColor mainTextColor;
  GColor foregroundColor;

  switch (currentTheme) {
    case LIGHT:
      mainTextColor = GColorBlack;
      foregroundColor = GColorBlack;

      s_theme.BackgroundColor = GColorWhite;
      break;
    case DARK:
    default:
      mainTextColor = GColorWhite;
      foregroundColor = GColorWhite;

      s_theme.BackgroundColor = GColorBlack;
      break;
  }

  // Time Layer
  s_theme.TimeTextColor = mainTextColor;
  // Date Layer
  s_theme.DateTextColor = mainTextColor;

  // Battery Bar Layer
  s_theme.BatteryOutlineColor = foregroundColor;
  s_theme.BatteryFillColor = foregroundColor;

  // Weather Layer
  s_theme.WeatherTextColor = mainTextColor;

  // Stepcount Layer
  s_theme.StepcountTextColor = mainTextColor;
}

Theme *theme_get_theme() {
  return &s_theme;
}

void init_theme(
    enum ThemeEnum themeEnum,
    bool showSeconds,
    int timeFontSize,
    bool autoTimeFontSize,
    GSize screenSize
) {
  s_theme.CurrentThemeEnum = themeEnum;
  currentTheme = themeEnum;

  set_colors();
  set_fonts(showSeconds, timeFontSize, autoTimeFontSize, screenSize);
}

void init_custom_theme(
    Theme theme,
    bool showSeconds,
    int timeFontSize,
    bool autoTimeFontSize,
    GSize screenSize
) {
  s_theme = theme;

  s_theme.CurrentThemeEnum = CUSTOM;
  currentTheme = CUSTOM;

  set_fonts(showSeconds, timeFontSize, autoTimeFontSize, screenSize);
}
