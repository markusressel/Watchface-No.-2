#include <pebble.h>
#include "theme.h"

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
