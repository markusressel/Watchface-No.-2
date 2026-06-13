#include <pebble.h>
#include "theme.h"
#include <string.h>

static Theme s_theme;

Theme *theme_set_fonts(Theme *theme, bool showSeconds) {
    GFont timeFont;
    if (showSeconds) {
        timeFont = fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS);
    } else {
        timeFont = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
    }
    theme->TimeFont = timeFont;
    return theme;
}

static void set_colors(Theme *theme, enum ThemeEnum themeEnum) {
    GColor mainTextColor;
    GColor foregroundColor;

    switch (themeEnum) {
        case LIGHT:
            mainTextColor = GColorBlack;
            foregroundColor = GColorBlack;

            theme->BackgroundColor = GColorWhite;
            break;
        case DARK:
        default:
            mainTextColor = GColorWhite;
            foregroundColor = GColorWhite;

            theme->BackgroundColor = GColorBlack;
            break;
    }

    theme->TimeTextColor = mainTextColor;
    theme->DateTextColor = mainTextColor;

    theme->BatteryOutlineColor = foregroundColor;
    theme->BatteryFillColor = foregroundColor;

    theme->WeatherTextColor = mainTextColor;
    theme->WeatherMaxTempColor = GColorRed;
    theme->WeatherCurrentTempColor = mainTextColor;
    theme->WeatherMinTempColor = GColorPictonBlue;

    theme->StepcountTextColor = mainTextColor;
    theme->HeartrateTextColor = mainTextColor;
}

Theme *theme_get_theme() {
    return &s_theme;
}

void set_theme(enum ThemeEnum themeEnum, bool showSeconds) {
    s_theme.CurrentThemeEnum = themeEnum;

    set_colors(&s_theme, themeEnum);
    theme_set_fonts(&s_theme, showSeconds);
}

void set_custom_theme(Theme *theme) {
    s_theme = *theme;
}

void apply_theme_from_settings(ClaySettings *settings, Window *window) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "apply_theme_from_settings");
    enum ThemeEnum theme;
    if (settings->ThemeValue[0] == '\0') {
        strcpy(settings->ThemeValue, THEME_LIGHT_STR);
    }
    if (strcmp(settings->ThemeValue, THEME_LIGHT_STR) == 0) {
        theme = LIGHT;
    } else if (strcmp(settings->ThemeValue, THEME_DARK_STR) == 0) {
        theme = DARK;
    } else {
        theme = CUSTOM;
    }

    if (theme == CUSTOM) {
        Theme custom_theme;
        custom_theme.CurrentThemeEnum = CUSTOM;
        custom_theme.BackgroundColor = settings->BackgroundColor;
        custom_theme.TimeTextColor = settings->TimeTextColor;
        custom_theme.DateTextColor = settings->DateTextColor;
        custom_theme.BatteryOutlineColor = settings->BatteryFrameColor;
        custom_theme.BatteryFillColor = settings->BatteryFillColor;
        custom_theme.WeatherTextColor = settings->WeatherTextColor;
        custom_theme.WeatherMaxTempColor = settings->WeatherMaxTempColor;
        custom_theme.WeatherCurrentTempColor = settings->WeatherCurrentTempColor;
        custom_theme.WeatherMinTempColor = settings->WeatherMinTempColor;
        custom_theme.StepcountTextColor = settings->StepcountTextColor;
        custom_theme.HeartrateTextColor = settings->HeartrateTextColor;
        theme_set_fonts(&custom_theme, settings->ShowSeconds);

        set_custom_theme(&custom_theme);
    } else {
        set_theme(theme, settings->ShowSeconds);
    }

    // Safely update the window background if the window exists
    if (window) {
        window_set_background_color(window, theme_get_theme()->BackgroundColor);
    }
}
