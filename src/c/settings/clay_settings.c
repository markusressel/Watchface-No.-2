#include <pebble.h>
#include "clay_settings.h"
#include "../ui/layer/widget.h"
#include "../app_messaging/app_messaging.h"
#include "../ui/watch_layout.h"

#ifdef WF_EMULATOR
const char *THEME_DEFAULT = THEME_DARK_STR;
#else
const char *THEME_DEFAULT = THEME_LIGHT_STR;
#endif

// An instance of the struct
static ClaySettings s_settings;

void clay_log_settings_debug(const char *context_label, ClaySettings *settings) {
    const char *label = context_label ? context_label : "settings";

    APP_LOG(
        APP_LOG_LEVEL_DEBUG,
        "%s theme=%s show_seconds=%d show_year=%d show_weekday=%d weekday_upper=%d",
        label,
        settings->ThemeValue,
        settings->ShowSeconds,
        settings->ShowYear,
        settings->ShowWeekdayAbbreviation,
        settings->WeekdayAbbreviationUppercase
    );

    APP_LOG(
        APP_LOG_LEVEL_DEBUG,
        "%s colors bg=%lu time=%lu date=%lu weather=%lu max=%lu cur=%lu min=%lu step=%lu hr=%lu",
        label,
        (unsigned long) settings->BackgroundColor.argb,
        (unsigned long) settings->TimeTextColor.argb,
        (unsigned long) settings->DateTextColor.argb,
        (unsigned long) settings->WeatherTextColor.argb,
        (unsigned long) settings->WeatherMaxTempColor.argb,
        (unsigned long) settings->WeatherCurrentTempColor.argb,
        (unsigned long) settings->WeatherMinTempColor.argb,
        (unsigned long) settings->StepcountTextColor.argb,
        (unsigned long) settings->HeartrateTextColor.argb
    );

    APP_LOG(
        APP_LOG_LEVEL_DEBUG,
        "%s weather slots: %d, %d, %d",
        label,
        settings->WeatherSlot1,
        settings->WeatherSlot2,
        settings->WeatherSlot3
    );

    APP_LOG(
        APP_LOG_LEVEL_DEBUG,
        "%s dotted digit_w=%d dot_w=%d dot_h=%d gap_h=%d gap_v=%d auto_scale=%d scale=%.2f",
        label,
        settings->DigitWidth,
        settings->DotWidth,
        settings->DotHeight,
        settings->DotHorizontalGap,
        settings->DotVerticalGap,
        settings->DotAutoScale,
        (double) settings->DotScaleFactor
    );

    APP_LOG(
        APP_LOG_LEVEL_DEBUG,
        "%s layout rows=%d r0=%d r1=%d r2=%d r3=%d r4=%d r5=%d r6=%d",
        label,
        settings->LayoutRowCount,
        settings->Row0Widget,
        settings->Row1Widget,
        settings->Row2Widget,
        settings->Row3Widget,
        settings->Row4Widget,
        settings->Row5Widget,
        settings->Row6Widget
    );

    APP_LOG(
        APP_LOG_LEVEL_DEBUG,
        "%s weather_simulation=%d forecast_preview_hours=%d update_interval_mins=%d",
        label,
        settings->WeatherUseSimulation,
        settings->SliderWeatherForecastPreviewHoursCount,
        settings->WeatherUpdateIntervalMinutes
    );
}

static bool is_row_widget_valid(const int widget) {
    return widget >= 0 && widget < WIDGET_COUNT;
}

static int layout_row_count_max_for_platform() {
#ifdef PBL_PLATFORM_EMERY
    return 7;
#else
    return 5;
#endif
}

static int clamp_layout_row_count(const int requested_row_count) {
    const int min_row_count = WATCH_LAYOUT_MIN_ROWS;
    const int max_row_count = layout_row_count_max_for_platform();

    if (requested_row_count < min_row_count) {
        return min_row_count;
    }
    if (requested_row_count > max_row_count) {
        return max_row_count;
    }
    return requested_row_count;
}

ClaySettings *clay_sanitize_settings(ClaySettings *settings) {
    if (settings->DigitWidth < 3 || settings->DigitWidth > 5) {
        settings->DigitWidth = 4;
    }

    if (settings->DotWidth < 1 || settings->DotWidth > 5) settings->DotWidth = 3;
    if (settings->DotHeight < 1 || settings->DotHeight > 5) settings->DotHeight = 3;
    if (settings->DotHorizontalGap < 1 || settings->DotHorizontalGap > 5) settings->DotHorizontalGap = 3;
    if (settings->DotVerticalGap < 1 || settings->DotVerticalGap > 5) settings->DotVerticalGap = 3;

    if (settings->DotScaleFactor <= 0.0f || settings->DotScaleFactor > 4.0f) {
        settings->DotScaleFactor = 1.0f;
    }
    settings->DotAutoScale = settings->DotAutoScale ? true : false;

    settings->LayoutRowCount = clamp_layout_row_count(settings->LayoutRowCount);

    if (!is_row_widget_valid(settings->Row0Widget)) settings->Row0Widget = WIDGET_WEATHER;
    if (!is_row_widget_valid(settings->Row1Widget)) settings->Row1Widget = WIDGET_DATE;
    if (!is_row_widget_valid(settings->Row2Widget)) settings->Row2Widget = WIDGET_TIME;
    if (!is_row_widget_valid(settings->Row3Widget)) settings->Row3Widget = WIDGET_STEPCOUNT;
    if (!is_row_widget_valid(settings->Row4Widget)) settings->Row4Widget = WIDGET_BATTERY_BAR;
    if (!is_row_widget_valid(settings->Row5Widget)) settings->Row5Widget = WIDGET_STEPCOUNT;
    if (!is_row_widget_valid(settings->Row6Widget)) settings->Row6Widget = WIDGET_STEPCOUNT;

    settings->WeatherUseSimulation = settings->WeatherUseSimulation;

    if (settings->SliderWeatherForecastPreviewHoursCount < 1 || settings->SliderWeatherForecastPreviewHoursCount > 24) {
        settings->SliderWeatherForecastPreviewHoursCount = 6;
    }

    if (settings->WeatherUpdateIntervalMinutes < 15 || settings->WeatherUpdateIntervalMinutes > 180) {
        settings->WeatherUpdateIntervalMinutes = 15;
    }

    if (settings->WeatherSlot1 < 0 || settings->WeatherSlot1 > 3) settings->WeatherSlot1 = 2;
    if (settings->WeatherSlot2 < 0 || settings->WeatherSlot2 > 3) settings->WeatherSlot2 = 1;
    if (settings->WeatherSlot3 < 0 || settings->WeatherSlot3 > 3) settings->WeatherSlot3 = 3;

    return settings;
}

// Initialize the default settings
// Note: Defaults are also set in the configPage.json, keep them in sync!
static ClaySettings *clay_reset_to_default_settings() {
    ClaySettings *settings = clay_get_settings();

    const bool is_dark = strcmp(THEME_DEFAULT, THEME_DARK_STR) == 0;
    settings->BackgroundColor = is_dark ? GColorBlack : GColorWhite;
    GColor foregroundColor = is_dark ? GColorWhite : GColorBlack;
    GColor textColor = is_dark ? GColorWhite : GColorBlack;

    // Time Layer
    settings->TimeTextColor = textColor;
    // Date Layer
    settings->DateTextColor = textColor;

    // Battery Bar Layer
    settings->BatteryFrameColor = foregroundColor;
    settings->BatteryFillColor = foregroundColor;

    // Weather Layer
    settings->WeatherTextColor = textColor;
    settings->WeatherMaxTempColor = GColorRed;
    settings->WeatherCurrentTempColor = textColor;
    settings->WeatherMinTempColor = GColorPictonBlue;

    settings->WeatherSlot1 = 2; // Max
    settings->WeatherSlot2 = 1; // Current
    settings->WeatherSlot3 = 3; // Min

    // Stepcount layer
    settings->StepcountTextColor = textColor;

    // Heartrate layer
    settings->HeartrateTextColor = textColor;

    settings->ShowYear = false;
    settings->ShowSeconds = false;
    settings->ShowWeekdayAbbreviation = false;
    settings->WeekdayAbbreviationUppercase = false;
    strcpy(settings->ThemeValue, THEME_DEFAULT);

    settings->DigitWidth = 4;

    settings->DotWidth = 3;
    settings->DotHeight = 3;
    settings->DotHorizontalGap = 3;
    settings->DotVerticalGap = 3;
    settings->DotScaleFactor = 1.0f;
    settings->DotAutoScale = true;

    settings->LayoutRowCount = layout_row_count_max_for_platform() == 7 ? 7 : 5;

    // Row layout defaults: Weather, Date, Time, Stepcount, Battery.
    settings->Row0Widget = WIDGET_WEATHER;
    settings->Row1Widget = WIDGET_DATE;
    settings->Row2Widget = WIDGET_TIME;
    settings->Row3Widget = WIDGET_STEPCOUNT;
    settings->Row4Widget = WIDGET_BATTERY_BAR;
    settings->Row5Widget = WIDGET_HEARTRATE;
    settings->Row6Widget = WIDGET_WEATHER_FORECAST;

#ifdef WF_EMULATOR
    settings->WeatherUseSimulation = true;
#else
    settings->WeatherUseSimulation = false;
#endif
    settings->SliderWeatherForecastPreviewHoursCount = 6;
    settings->WeatherUpdateIntervalMinutes = 15;

    return settings;
}

ClaySettings *clay_get_settings() {
    return &s_settings;
}

ClaySettings *internal_load_settings() {
    // Load the default settings
    ClaySettings *settings = clay_reset_to_default_settings();

    // Migrate/reset settings when the struct layout changes across versions.
    if (!persist_exists(SETTINGS_VERSION_KEY) ||
        persist_read_int(SETTINGS_VERSION_KEY) != SETTINGS_VERSION) {
        APP_LOG(
            APP_LOG_LEVEL_INFO,
            "Settings version mismatch: %d != %d. Deleting old settings and requesting new ones.",
            persist_read_int(SETTINGS_VERSION_KEY), SETTINGS_VERSION
        );
        // save default settings to persistent storage
        clay_save_settings(settings);
        app_messaging_request_settings();
        return settings;
    }

    if (persist_exists(SETTINGS_KEY)) {
        const int bytes = persist_read_data(SETTINGS_KEY, settings, sizeof(*settings));
        if (bytes != sizeof(*settings)) {
            APP_LOG(APP_LOG_LEVEL_WARNING, "Could not read settings, using defaults.");
            settings = clay_reset_to_default_settings();
            return settings;
        }
    }

    settings = clay_sanitize_settings(settings);
    clay_log_settings_debug("loaded persisted settings", settings);
    return settings;
}

// Read settings from persistent storage
ClaySettings *clay_load_settings() {
    ClaySettings *settings = internal_load_settings();
    s_settings = *settings;
    return settings;
}

ClaySettings *clay_save_settings(ClaySettings *settings) {
    persist_delete(SETTINGS_KEY);
    persist_delete(SETTINGS_VERSION_KEY);
    // save ClaySettings struct to persistent storage
    persist_write_int(SETTINGS_VERSION_KEY, SETTINGS_VERSION);
    persist_write_data(SETTINGS_KEY, settings, sizeof(ClaySettings));

    return settings;
}