#include <pebble.h>
#include <string.h>
#include "app_messaging.h"
#include "main.h"
#include "layer/weather.h"
#include "clay_settings.h"

static ClaySettings *s_settings;

static int tuple_to_int(const Tuple *tuple) {
    if (tuple == NULL) {
        return 0;
    }

    if (tuple->type == TUPLE_CSTRING) {
        return atoi(tuple->value->cstring);
    }

    return tuple->value->int32;
}

static int clamp_layout_row_count(const int row_count) {
    const int max_rows = 7;
    const int min_rows = 5;

    if (row_count < min_rows) {
        return min_rows;
    }
    if (row_count > max_rows) {
        return max_rows;
    }
    return row_count;
}

static bool apply_cstring_setting(
    DictionaryIterator *iterator,
    const uint32_t message_key,
    char *destination,
    const size_t destination_size
) {
    Tuple *tuple = dict_find(iterator, message_key);
    if (!tuple || destination_size == 0) {
        return false;
    }

    const char *value = tuple->value->cstring;
    if (!value) {
        destination[0] = '\0';
        return true;
    }

    strncpy(destination, value, destination_size - 1);
    destination[destination_size - 1] = '\0';
    return true;
}

static bool apply_color_setting(
    DictionaryIterator *iterator,
    const uint32_t message_key,
    GColor *destination
) {
    Tuple *tuple = dict_find(iterator, message_key);
    if (!tuple) {
        return false;
    }

    *destination = GColorFromHEX(tuple->value->int32);
    return true;
}

static bool apply_int_setting(
    DictionaryIterator *iterator,
    const uint32_t message_key,
    int *destination
) {
    Tuple *tuple = dict_find(iterator, message_key);
    if (!tuple) {
        return false;
    }

    *destination = tuple_to_int(tuple);
    return true;
}

static bool apply_int_setting_clamped(
    DictionaryIterator *iterator,
    const uint32_t message_key,
    int *destination,
    int (*clamp_fn)(int)
) {
    Tuple *tuple = dict_find(iterator, message_key);
    if (!tuple) {
        return false;
    }

    int value = tuple_to_int(tuple);
    *destination = clamp_fn ? clamp_fn(value) : value;
    return true;
}

static bool apply_bool_setting(
    DictionaryIterator *iterator,
    const uint32_t message_key,
    bool *destination
) {
    Tuple *tuple = dict_find(iterator, message_key);
    if (!tuple) {
        return false;
    }

    *destination = tuple_to_int(tuple) == 1;
    return true;
}

static void copy_tuple_cstring_to_buffer(
    DictionaryIterator *iterator,
    const uint32_t message_key,
    char *destination,
    const size_t destination_size
) {
    if (destination_size == 0) {
        return;
    }

    Tuple *tuple = dict_find(iterator, message_key);
    if (!tuple) {
        return;
    }

    strncpy(destination, tuple->value->cstring, destination_size - 1);
    destination[destination_size - 1] = '\0';
}

static bool read_configuration_properties(
    DictionaryIterator *iterator
) {
    s_settings = clay_get_settings();

    bool has_settings_update = false;

#define APPLY_STRING(KEY, FIELD) has_settings_update |= apply_cstring_setting(iterator, (KEY), s_settings->FIELD, sizeof(s_settings->FIELD))
#define APPLY_COLOR(KEY, FIELD) has_settings_update |= apply_color_setting(iterator, (KEY), &s_settings->FIELD)
#define APPLY_INT(KEY, FIELD) do { int _v = 0; if (apply_int_setting(iterator, (KEY), &_v)) { s_settings->FIELD = _v; has_settings_update = true; } } while (0)
#define APPLY_INT_CLAMPED(KEY, FIELD, CLAMP_FN) do { int _v = 0; if (apply_int_setting_clamped(iterator, (KEY), &_v, (CLAMP_FN))) { s_settings->FIELD = _v; has_settings_update = true; } } while (0)
#define APPLY_BOOL(KEY, FIELD) do { bool _v = false; if (apply_bool_setting(iterator, (KEY), &_v)) { s_settings->FIELD = _v; has_settings_update = true; } } while (0)

    APPLY_STRING(MESSAGE_KEY_Theme, ThemeValue);

    APPLY_COLOR(MESSAGE_KEY_BackgroundColor, BackgroundColor);
    APPLY_COLOR(MESSAGE_KEY_TimeTextColor, TimeTextColor);
    APPLY_COLOR(MESSAGE_KEY_DateTextColor, DateTextColor);
    APPLY_COLOR(MESSAGE_KEY_BatteryFrameColor, BatteryFrameColor);
    APPLY_COLOR(MESSAGE_KEY_BatteryFillColor, BatteryFillColor);
    APPLY_COLOR(MESSAGE_KEY_WeatherTextColor, WeatherTextColor);
    APPLY_COLOR(MESSAGE_KEY_StepcountTextColor, StepcountTextColor);
    APPLY_COLOR(MESSAGE_KEY_HeartrateTextColor, HeartrateTextColor);

    APPLY_INT(MESSAGE_KEY_SliderDigitWidth, DigitWidth);
    APPLY_INT(MESSAGE_KEY_SliderDotWidth, DotWidth);
    APPLY_INT(MESSAGE_KEY_SliderDotHeight, DotHeight);
    APPLY_INT(MESSAGE_KEY_SliderDotHorizontalGap, DotHorizontalGap);
    APPLY_INT(MESSAGE_KEY_SliderDotVerticalGap, DotVerticalGap);
    APPLY_BOOL(MESSAGE_KEY_ToggleDotAutoScale, DotAutoScale);

    {
        int dot_scale_percent = 0;
        if (apply_int_setting(iterator, MESSAGE_KEY_SliderDotScaleFactorPercent, &dot_scale_percent)) {
            s_settings->DotScaleFactor = (float) dot_scale_percent / 100.0f;
            has_settings_update = true;
        }
    }

    APPLY_BOOL(MESSAGE_KEY_ShowYear, ShowYear);
    APPLY_BOOL(MESSAGE_KEY_ShowSeconds, ShowSeconds);
    APPLY_BOOL(MESSAGE_KEY_ShowWeekdayAbbreviation, ShowWeekdayAbbreviation);
    APPLY_BOOL(MESSAGE_KEY_WeekdayAbbreviationUppercase, WeekdayAbbreviationUppercase);

    APPLY_INT(MESSAGE_KEY_Row0Widget, Row0Widget);
    APPLY_INT(MESSAGE_KEY_Row1Widget, Row1Widget);
    APPLY_INT(MESSAGE_KEY_Row2Widget, Row2Widget);
    APPLY_INT(MESSAGE_KEY_Row3Widget, Row3Widget);
    APPLY_INT(MESSAGE_KEY_Row4Widget, Row4Widget);
    APPLY_INT(MESSAGE_KEY_Row5Widget, Row5Widget);
    APPLY_INT(MESSAGE_KEY_Row6Widget, Row6Widget);
    APPLY_INT_CLAMPED(MESSAGE_KEY_LayoutRowCount, LayoutRowCount, clamp_layout_row_count);

#undef APPLY_STRING
#undef APPLY_COLOR
#undef APPLY_INT
#undef APPLY_INT_CLAMPED
#undef APPLY_BOOL

    return has_settings_update;
}

static void read_weather_data(DictionaryIterator *iterator) {
    // Read tuples for data
    Tuple *temp_cur_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_TEMPERATURE_CURRENT);
    Tuple *temp_min_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_TEMPERATURE_MIN);
    Tuple *temp_max_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_TEMPERATURE_MAX);
    Tuple *condition_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_CONDITION);
    Tuple *rain_next_hour_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_RAIN_NEXT_HOUR_MM_X10);
    Tuple *rain_pop_percent_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_RAIN_POP_PERCENT);
    Tuple *temp_forecast_encoded_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_TEMP_FORECAST_ENCODED);
    Tuple *rain_forecast_encoded_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_RAIN_FORECAST_MM_X10_ENCODED);

    // If all data is available, use it
    if (temp_cur_tuple && temp_min_tuple && temp_max_tuple && condition_tuple) {
        WeatherData *weatherData = weather_get_data();

        APP_LOG(APP_LOG_LEVEL_DEBUG, "temp changed: old: %d new: %d", (int) weatherData->CurrentTemperature, (int) temp_cur_tuple->value->int32);

        weatherData->CurrentTemperature = temp_cur_tuple->value->int32;
        weatherData->MinTemperature = temp_min_tuple->value->int32;
        weatherData->MaxTemperature = temp_max_tuple->value->int32;
        strcpy(weatherData->CurrentConditions, condition_tuple->value->cstring);

        if (rain_next_hour_tuple) {
            weatherData->RainNextHourMmX10 = rain_next_hour_tuple->value->int32;
        }
        if (rain_pop_percent_tuple) {
            weatherData->RainPopPercent = rain_pop_percent_tuple->value->int32;
        }
        if (temp_forecast_encoded_tuple) {
            copy_tuple_cstring_to_buffer(
                iterator,
                MESSAGE_KEY_WEATHER_TEMP_FORECAST_ENCODED,
                weatherData->TemperatureForecastEncoded,
                sizeof(weatherData->TemperatureForecastEncoded)
            );
        }
        if (rain_forecast_encoded_tuple) {
            copy_tuple_cstring_to_buffer(
                iterator,
                MESSAGE_KEY_WEATHER_RAIN_FORECAST_MM_X10_ENCODED,
                weatherData->RainForecastMmX10Encoded,
                sizeof(weatherData->RainForecastMmX10Encoded)
            );
        }

        APP_LOG(
            APP_LOG_LEVEL_DEBUG,
            "weather rain update: next_1h=%d.%dmm pop=%d%%",
            weatherData->RainNextHourMmX10 / 10,
            weatherData->RainNextHourMmX10 % 10,
            weatherData->RainPopPercent
        );
        APP_LOG(
            APP_LOG_LEVEL_DEBUG,
            "weather forecast arrays: temp='%s' rain='%s'",
            weatherData->TemperatureForecastEncoded,
            weatherData->RainForecastMmX10Encoded
        );

        update_weather();
    }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    const bool has_settings_update = read_configuration_properties(iterator);
    read_weather_data(iterator);

    if (has_settings_update) {
        clay_log_settings_debug("received settings update");
        clay_save_settings();
        main_reload_layout();
    }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped! %d", (int) reason);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! %d", (int) reason);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void initialize_app_messaging() {
    // Register callbacks
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);

    // Open AppMessage with the largest receive buffer the platform supports,
    // to accommodate all settings including string fields like WeatherApiKey.
    app_message_open(app_message_inbox_size_maximum(), 256);
}
