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
    theme->WeekdayTextColor = mainTextColor;

    theme->BatteryOutlineColor = foregroundColor;
    theme->BatteryFillColor = foregroundColor;
    theme->BatteryLowColor = GColorRed;

    theme->WeatherTextColor = mainTextColor;
    theme->WeatherMaxTempColor = GColorRed;
    theme->WeatherCurrentTempColor = mainTextColor;
    theme->WeatherMinTempColor = GColorPictonBlue;
    theme->WeatherAxisTickColor = GColorDarkGray;
    theme->WeatherIndicatorColor = mainTextColor;

#if defined(PBL_COLOR)
    // Forecast Graph Colors
    theme->ForecastTempColorM10 = GColorBlueMoon;
    theme->ForecastTempColor0 = GColorPictonBlue;
    theme->ForecastTempColor10 = GColorGreen;
    theme->ForecastTempColor20 = GColorChromeYellow;
    theme->ForecastTempColor30 = GColorOrange;
    theme->ForecastTempColor40 = GColorRed;

    theme->ForecastRainColor0 = GColorLightGray;
    theme->ForecastRainColor3 = GColorPictonBlue;
    theme->ForecastRainColor10 = GColorBlueMoon;
    theme->ForecastRainColor50 = GColorBlue;
    theme->ForecastRainColor100 = GColorDukeBlue;
#endif

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
        custom_theme.WeekdayTextColor = settings->WeekdayTextColor;
        custom_theme.BatteryOutlineColor = settings->BatteryFrameColor;
        custom_theme.BatteryFillColor = settings->BatteryFillColor;
        custom_theme.BatteryLowColor = settings->BatteryLowColor;
        custom_theme.WeatherTextColor = settings->WeatherTextColor;
        custom_theme.WeatherMaxTempColor = settings->WeatherMaxTempColor;
        custom_theme.WeatherCurrentTempColor = settings->WeatherCurrentTempColor;
        custom_theme.WeatherMinTempColor = settings->WeatherMinTempColor;
        custom_theme.WeatherAxisTickColor = settings->WeatherAxisTickColor;
        custom_theme.WeatherIndicatorColor = settings->WeatherIndicatorColor;

#if defined(PBL_COLOR)
        custom_theme.ForecastTempColorM10 = settings->ForecastTempColorM10;
        custom_theme.ForecastTempColor0 = settings->ForecastTempColor0;
        custom_theme.ForecastTempColor10 = settings->ForecastTempColor10;
        custom_theme.ForecastTempColor20 = settings->ForecastTempColor20;
        custom_theme.ForecastTempColor30 = settings->ForecastTempColor30;
        custom_theme.ForecastTempColor40 = settings->ForecastTempColor40;

        custom_theme.ForecastRainColor0 = settings->ForecastRainColor0;
        custom_theme.ForecastRainColor3 = settings->ForecastRainColor3;
        custom_theme.ForecastRainColor10 = settings->ForecastRainColor10;
        custom_theme.ForecastRainColor50 = settings->ForecastRainColor50;
        custom_theme.ForecastRainColor100 = settings->ForecastRainColor100;
#endif

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
