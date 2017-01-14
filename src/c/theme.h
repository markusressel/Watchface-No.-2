#pragma once
#include <pebble.h>

// Themes
enum ThemeEnum {
  DARK,
  LIGHT,
  CUSTOM
};

typedef struct Theme {
  enum ThemeEnum CurrentThemeEnum;
  
  // General Colors
  GColor BackgroundColor;
  
  // Time Layer
  GColor TimeTextColor;
  // Date Layer
  GColor DateTextColor;
    
  // Battery Bar Layer
  GColor BatteryOutlineColor;
  GColor BatteryFillColor;
  
  // Weather Layer
  GColor WeatherTextColor;
  
  // Stepcount layer
  GColor StepcountTextColor;
  
  // Fonts
  GFont TimeFont;
} __attribute__((__packed__)) Theme;

// Get current Theme struct
Theme* theme_get_theme();

// method to set application theme
// @param theme        one of the ThemeEnum values DARK, LIGHT
//                     if you want to set a custom theme use init_custom_theme();
// @param showSeconds  specifies if the time shows seconds (to properly select font size)
void init_theme(enum ThemeEnum theme, bool showSeconds);

// method to initlalize custom theme with custom colors
// @param theme        the custom theme
// @param showSeconds  specifies if the time shows seconds (to properly select font size)
void init_custom_theme(Theme theme, bool showSeconds);