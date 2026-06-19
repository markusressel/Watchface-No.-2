#include <pebble.h>
#include <string.h>
#include "app_messaging.h"
#include "../main.h"
#include "forecast_series.h"
#include "../ui/layer/weather.h"
#include "../settings/clay_settings.h"
#include "../ui/watch_layout.h"
#include "../ui/layer/weather_forecast.h"

void queue_message(const uint32_t key, const int value) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    if (iter == NULL) {
        return;
    }
    dict_write_uint8(iter, key, value);
    dict_write_end(iter);
}

void app_messaging_send_app_ready() {
    queue_message(MESSAGE_KEY_AppReady, 1);
    app_message_outbox_send();
}

void app_messaging_send_app_ready_and_request_settings() {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    if (iter == NULL) {
        return;
    }
    dict_write_uint8(iter, MESSAGE_KEY_AppReady, 1);
    dict_write_uint8(iter, MESSAGE_KEY_RequestSettings, 1);
    dict_write_end(iter);
    app_message_outbox_send();
}

void app_messaging_request_settings() {
    queue_message(MESSAGE_KEY_RequestSettings, 1);
    app_message_outbox_send();
}

static int hex_to_int(const char *hex_str) {
    if (hex_str == NULL) return 0;
    if (hex_str[0] == '0' && (hex_str[1] == 'x' || hex_str[1] == 'X')) {
        hex_str += 2;
    }
    int result = 0;
    while (*hex_str) {
        char c = *hex_str++;
        int val = 0;
        if (c >= '0' && c <= '9') val = c - '0';
        else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') val = c - 'A' + 10;
        else break;
        result = (result << 4) | val;
    }
    return result;
}

static int tuple_to_int(const Tuple *tuple) {
    if (tuple == NULL) {
        return 0;
    }

    if (tuple->type == TUPLE_CSTRING) {
        // Handle both decimal and hex strings
        const char *str = tuple->value->cstring;
        if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
            return hex_to_int(str);
        }
        return atoi(str);
    }

    return tuple->value->int32;
}

static int clamp_layout_row_count(const int row_count) {
    const int max_rows = WATCH_LAYOUT_MAX_ROWS;
    const int min_rows = WATCH_LAYOUT_MIN_ROWS;

    if (row_count < min_rows) {
        return min_rows;
    }
    if (row_count > max_rows) {
        return max_rows;
    }
    return row_count;
}

static bool apply_cstring_setting(
    const DictionaryIterator *iterator,
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

    *destination = GColorFromHEX(tuple_to_int(tuple));
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

static int parse_forecast_tuple_to_array(const Tuple *tuple, int *destination, const int max_values) {
    if (!tuple || tuple->type != TUPLE_CSTRING || !destination || max_values <= 0) {
        return 0;
    }

    const char *value = tuple->value->cstring;
    if (!value) {
        return 0;
    }

    const size_t capacity = tuple->length > 0 ? tuple->length : (strlen(value) + 1);
    return forecast_parse_int_series(value, capacity, destination, max_values);
}

static bool read_configuration_properties(
    DictionaryIterator *iterator
) {
    ClaySettings *settings = clay_get_settings();
    bool has_settings_update = false;

#define APPLY_STRING(KEY, FIELD) has_settings_update |= apply_cstring_setting(iterator, (KEY), settings->FIELD, sizeof(settings->FIELD))
#define APPLY_COLOR(KEY, FIELD) has_settings_update |= apply_color_setting(iterator, (KEY), &settings->FIELD)
#define APPLY_INT(KEY, FIELD) do { int _v = 0; if (apply_int_setting(iterator, (KEY), &_v)) { settings->FIELD = _v; has_settings_update = true; } } while (0)
#define APPLY_INT_CLAMPED(KEY, FIELD, CLAMP_FN) do { int _v = 0; if (apply_int_setting_clamped(iterator, (KEY), &_v, (CLAMP_FN))) { settings->FIELD = _v; has_settings_update = true; } } while (0)
#define APPLY_BOOL(KEY, FIELD) do { bool _v = false; if (apply_bool_setting(iterator, (KEY), &_v)) { settings->FIELD = _v; has_settings_update = true; } } while (0)

    APPLY_STRING(MESSAGE_KEY_Theme, ThemeValue);

    APPLY_COLOR(MESSAGE_KEY_BackgroundColor, BackgroundColor);
    APPLY_COLOR(MESSAGE_KEY_TimeTextColor, TimeTextColor);
    APPLY_COLOR(MESSAGE_KEY_DateTextColor, DateTextColor);
    APPLY_COLOR(MESSAGE_KEY_WeekdayTextColor, WeekdayTextColor);
    APPLY_COLOR(MESSAGE_KEY_BatteryFrameColor, BatteryFrameColor);
    APPLY_COLOR(MESSAGE_KEY_BatteryFillColor, BatteryFillColor);
    APPLY_COLOR(MESSAGE_KEY_BatteryLowColor, BatteryLowColor);
    APPLY_COLOR(MESSAGE_KEY_WeatherTextColor, WeatherTextColor);
    APPLY_COLOR(MESSAGE_KEY_WeatherMaxTempColor, WeatherMaxTempColor);
    APPLY_COLOR(MESSAGE_KEY_WeatherCurrentTempColor, WeatherCurrentTempColor);
    APPLY_COLOR(MESSAGE_KEY_WeatherMinTempColor, WeatherMinTempColor);
    APPLY_COLOR(MESSAGE_KEY_WeatherAxisTickColor, WeatherAxisTickColor);
    APPLY_COLOR(MESSAGE_KEY_WeatherIndicatorColor, WeatherIndicatorColor);

#if defined(PBL_COLOR)
    APPLY_COLOR(MESSAGE_KEY_ForecastTempColorM10, ForecastTempColorM10);
    APPLY_COLOR(MESSAGE_KEY_ForecastTempColor0, ForecastTempColor0);
    APPLY_COLOR(MESSAGE_KEY_ForecastTempColor10, ForecastTempColor10);
    APPLY_COLOR(MESSAGE_KEY_ForecastTempColor20, ForecastTempColor20);
    APPLY_COLOR(MESSAGE_KEY_ForecastTempColor30, ForecastTempColor30);
    APPLY_COLOR(MESSAGE_KEY_ForecastTempColor40, ForecastTempColor40);

    APPLY_COLOR(MESSAGE_KEY_ForecastRainColor0, ForecastRainColor0);
    APPLY_COLOR(MESSAGE_KEY_ForecastRainColor3, ForecastRainColor3);
    APPLY_COLOR(MESSAGE_KEY_ForecastRainColor10, ForecastRainColor10);
    APPLY_COLOR(MESSAGE_KEY_ForecastRainColor50, ForecastRainColor50);
    APPLY_COLOR(MESSAGE_KEY_ForecastRainColor100, ForecastRainColor100);
#endif

    APPLY_COLOR(MESSAGE_KEY_StepcountTextColor, StepcountTextColor);
    APPLY_COLOR(MESSAGE_KEY_HeartrateTextColor, HeartrateTextColor);

    APPLY_INT(MESSAGE_KEY_WeatherSlot1, WeatherSlot1);
    APPLY_INT(MESSAGE_KEY_WeatherSlot2, WeatherSlot2);
    APPLY_INT(MESSAGE_KEY_WeatherSlot3, WeatherSlot3);
    APPLY_INT(MESSAGE_KEY_LowBatteryThreshold, LowBatteryThreshold);

    APPLY_INT(MESSAGE_KEY_SliderDigitWidth, DigitWidth);
    APPLY_INT(MESSAGE_KEY_SliderDotWidth, DotWidth);
    APPLY_INT(MESSAGE_KEY_SliderDotHeight, DotHeight);
    APPLY_INT(MESSAGE_KEY_SliderDotHorizontalGap, DotHorizontalGap);
    APPLY_INT(MESSAGE_KEY_SliderDotVerticalGap, DotVerticalGap);
    APPLY_BOOL(MESSAGE_KEY_ToggleDotAutoScale, DotAutoScale);

    {
        int dot_scale_percent = 0;
        if (apply_int_setting(iterator, MESSAGE_KEY_SliderDotScaleFactorPercent, &dot_scale_percent)) {
            settings->DotScaleFactor = (float) dot_scale_percent / 100.0f;
            has_settings_update = true;
        }
    }

    APPLY_BOOL(MESSAGE_KEY_ShowYear, ShowYear);
    APPLY_BOOL(MESSAGE_KEY_ShowSeconds, ShowSeconds);
    APPLY_BOOL(MESSAGE_KEY_ShowWeekdayAbbreviation, ShowWeekdayAbbreviation);
    APPLY_BOOL(MESSAGE_KEY_WeekdayAbbreviationUppercase, WeekdayAbbreviationUppercase);
    APPLY_BOOL(MESSAGE_KEY_DateZeroPadding, DateZeroPadding);
    APPLY_BOOL(MESSAGE_KEY_WeatherUseSimulation, WeatherUseSimulation);
    APPLY_INT(MESSAGE_KEY_SliderWeatherForecastPreviewHoursCount, SliderWeatherForecastPreviewHoursCount);
    APPLY_INT(MESSAGE_KEY_WeatherUpdateIntervalMinutes, WeatherUpdateIntervalMinutes);

    APPLY_INT(MESSAGE_KEY_Row0Widget, Row0Widget);
    APPLY_INT(MESSAGE_KEY_Row1Widget, Row1Widget);
    APPLY_INT(MESSAGE_KEY_Row2Widget, Row2Widget);
    APPLY_INT(MESSAGE_KEY_Row3Widget, Row3Widget);
    APPLY_INT(MESSAGE_KEY_Row4Widget, Row4Widget);
    APPLY_INT(MESSAGE_KEY_Row5Widget, Row5Widget);
    APPLY_INT(MESSAGE_KEY_Row6Widget, Row6Widget);
    APPLY_INT_CLAMPED(MESSAGE_KEY_LayoutRowCount, LayoutRowCount, clamp_layout_row_count);

    {
        int time_ratio_percent = 0;
        if (apply_int_setting(iterator, MESSAGE_KEY_SliderTimeRowRatioPercent, &time_ratio_percent)) {
            settings->TimeRowRatio = (float) time_ratio_percent / 100.0f;
            has_settings_update = true;
        }
    }

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
    Tuple *forecast_start_ts_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_FORECAST_START_TS);

    // If all data is available, use it
    // Move scratch buffer to static, to prevent Stack Overflow (800+ bytes is too much for Pebble stack)
    static int s_forecast_scratch[WEATHER_FORECAST_MAX_POINTS];

    if (temp_cur_tuple && temp_min_tuple && temp_max_tuple && condition_tuple) {
        weather_init_data(); // Ensure initialized flag is set if it wasn't already
        WeatherData *weatherData = weather_get_data();

        weatherData->CurrentTemperature = tuple_to_int(temp_cur_tuple);
        weatherData->MinTemperature = tuple_to_int(temp_min_tuple);
        weatherData->MaxTemperature = tuple_to_int(temp_max_tuple);
        if (forecast_start_ts_tuple) {
            weatherData->ForecastStartTimestamp = (time_t) tuple_to_int(forecast_start_ts_tuple);
        }

        if (condition_tuple->type == TUPLE_CSTRING) {
            strncpy(
                weatherData->CurrentConditions,
                condition_tuple->value->cstring,
                sizeof(weatherData->CurrentConditions) - 1
            );
            weatherData->CurrentConditions[sizeof(weatherData->CurrentConditions) - 1] = '\0';
        } else {
            weatherData->CurrentConditions[0] = '\0';
        }

        if (rain_next_hour_tuple) {
            weatherData->RainNextHourMmX10 = tuple_to_int(rain_next_hour_tuple);
        }
        if (rain_pop_percent_tuple) {
            weatherData->RainPopPercent = tuple_to_int(rain_pop_percent_tuple);
        }

        // 1. ONLY free the arrays if they were genuinely allocated via malloc
        if (weatherData->is_temp_forecast_dynamic_alloc) {
            if (weatherData->TemperatureForecast) {
                free(weatherData->TemperatureForecast);
            }
            weatherData->is_temp_forecast_dynamic_alloc = false;
        }

        if (weatherData->is_rain_forecast_dynamic_alloc) {
            if (weatherData->RainForecastMmX10) {
                free(weatherData->RainForecastMmX10);
            }
            weatherData->is_rain_forecast_dynamic_alloc = false;
        }

        // 2. Always safely break the pointer connections so they don't point to abandoned memory
        weatherData->TemperatureForecast = NULL;
        weatherData->TemperatureForecastCount = 0;
        weatherData->RainForecastMmX10 = NULL;
        weatherData->RainForecastMmX10Count = 0;

        // 3. Handle Temperature Forecast Processing
        if (temp_forecast_encoded_tuple) {
            int count = parse_forecast_tuple_to_array(temp_forecast_encoded_tuple, s_forecast_scratch, WEATHER_FORECAST_MAX_POINTS);

            if (count > 0) {
                weatherData->TemperatureForecast = malloc(count * sizeof(int));
                if (weatherData->TemperatureForecast) {
                    memcpy(weatherData->TemperatureForecast, s_forecast_scratch, count * sizeof(int));
                    weatherData->TemperatureForecastCount = count;
                    weatherData->is_temp_forecast_dynamic_alloc = true; // Set flag because malloc succeeded
                }
            }
        }

        // 4. Handle Rain Forecast Processing
        if (rain_forecast_encoded_tuple) {
            int count = parse_forecast_tuple_to_array(rain_forecast_encoded_tuple, s_forecast_scratch, WEATHER_FORECAST_MAX_POINTS);

            if (count > 0) {
                weatherData->RainForecastMmX10 = malloc(count * sizeof(int));
                if (weatherData->RainForecastMmX10) {
                    memcpy(weatherData->RainForecastMmX10, s_forecast_scratch, count * sizeof(int));
                    weatherData->RainForecastMmX10Count = count;
                    weatherData->is_rain_forecast_dynamic_alloc = true; // Set flag because malloc succeeded
                }
            }
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
            "weather forecast points: temp=%d rain=%d",
            weatherData->TemperatureForecastCount,
            weatherData->RainForecastMmX10Count
        );

        weatherData->is_dirty = true;
        update_weather();
        update_weather_forecast();
    }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    ClaySettings *settings = clay_get_settings();
    const bool was_sync_pending = !settings->InitialSyncDone;

    const bool has_settings_update = read_configuration_properties(iterator);
    read_weather_data(iterator);

    if (has_settings_update || was_sync_pending) {
        settings->InitialSyncDone = true;
        clay_log_settings_debug("received settings update", settings);
        clay_save_settings(settings);
        main_reload_layout(settings, main_get_window());
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

void app_messaging_initialize() {
    // Register callbacks
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);

    // Avoid app_message_inbox_size_maximum() to keep heap usage stable when
    // forecast layers are enabled.
    static const uint32_t k_outbox_size = 256;
    static const uint32_t k_inbox_sizes[] = {1024, 768, 512};

    AppMessageResult result = APP_MSG_INTERNAL_ERROR;
    for (unsigned int i = 0; i < sizeof(k_inbox_sizes) / sizeof(k_inbox_sizes[0]); i++) {
        result = app_message_open(k_inbox_sizes[i], k_outbox_size);
        if (result == APP_MSG_OK) {
            APP_LOG(APP_LOG_LEVEL_INFO, "app_message_open() inbox=%lu outbox=%lu", k_inbox_sizes[i], k_outbox_size);
            return;
        }
    }

    APP_LOG(APP_LOG_LEVEL_ERROR, "app_message_open() failed: %d", (int) result);
}