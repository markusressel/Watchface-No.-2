#include <pebble.h>
#include "theme.h"

static Theme s_theme;

Theme *theme_set_fonts(Theme *theme, bool showSeconds) {
    if (showSeconds) {
        theme->TimeFont = fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS);
    } else {
        theme->TimeFont = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
    }
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

    // Time Layer
    theme->TimeTextColor = mainTextColor;
    // Date Layer
    theme->DateTextColor = mainTextColor;

    // Battery Bar Layer
    theme->BatteryOutlineColor = foregroundColor;
    theme->BatteryFillColor = foregroundColor;

    // Weather Layer
    theme->WeatherTextColor = mainTextColor;

    // Stepcount Layer
    theme->StepcountTextColor = mainTextColor;

    // Heartrate Layer
    theme->HeartrateTextColor = mainTextColor;
}

Theme *theme_get_theme() {
    return &s_theme;
}

void init_theme(enum ThemeEnum themeEnum, bool showSeconds) {
    s_theme.CurrentThemeEnum = themeEnum;

    set_colors(&s_theme, themeEnum);
    theme_set_fonts(&s_theme, showSeconds);
}

void set_custom_theme(Theme *theme) {
    s_theme = *theme;
}

// Theme theme_create_custom(
//   ) {
//
// }
