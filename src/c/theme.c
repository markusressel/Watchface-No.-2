#include <pebble.h>
#include "theme.h"

static Theme s_theme;
static enum ThemeEnum currentTheme;

static void set_fonts(bool showSeconds) {
  if (showSeconds) {
    s_theme.TimeFont = fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS);
    //s_theme.TimeFont = fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
  } else {
    s_theme.TimeFont = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
    //s_theme.TimeFont = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  }
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

Theme* theme_get_theme(){
  return &s_theme;
}

void init_theme(enum ThemeEnum themeEnum, bool showSeconds) {
  s_theme.CurrentThemeEnum = themeEnum;
  currentTheme = themeEnum;
  
  set_colors();
  set_fonts(showSeconds);
}

void init_custom_theme(Theme theme, bool showSeconds) {
  s_theme = theme;
  
  s_theme.CurrentThemeEnum = CUSTOM;
  currentTheme = CUSTOM;
  
  set_fonts(showSeconds);
}