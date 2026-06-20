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

static void apply_cstring_setting_from_tuple(const Tuple *tuple, char *destination, size_t destination_size) {
    if (!tuple || destination_size == 0) return;
    const char *value = tuple->value->cstring;
    if (!value) {
        destination[0] = '\0';
        return;
    }
    strncpy(destination, value, destination_size - 1);
    destination[destination_size - 1] = '\0';
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

static void process_weather_data(
    Tuple *temp_cur_tuple,
    Tuple *temp_min_tuple,
    Tuple *temp_max_tuple,
    Tuple *condition_tuple,
    Tuple *rain_next_hour_tuple,
    Tuple *rain_pop_percent_tuple,
    Tuple *temp_forecast_encoded_tuple,
    Tuple *rain_forecast_encoded_tuple,
    Tuple *forecast_start_ts_tuple
) {
    static int s_forecast_scratch[WEATHER_FORECAST_MAX_POINTS];

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

    weatherData->TemperatureForecast = NULL;
    weatherData->TemperatureForecastCount = 0;
    weatherData->RainForecastMmX10 = NULL;
    weatherData->RainForecastMmX10Count = 0;

    if (temp_forecast_encoded_tuple) {
        int count = parse_forecast_tuple_to_array(temp_forecast_encoded_tuple, s_forecast_scratch, WEATHER_FORECAST_MAX_POINTS);

        if (count > 0) {
            weatherData->TemperatureForecast = malloc(count * sizeof(int));
            if (weatherData->TemperatureForecast) {
                memcpy(weatherData->TemperatureForecast, s_forecast_scratch, count * sizeof(int));
                weatherData->TemperatureForecastCount = count;
                weatherData->is_temp_forecast_dynamic_alloc = true;
            }
        }
    }

    if (rain_forecast_encoded_tuple) {
        int count = parse_forecast_tuple_to_array(rain_forecast_encoded_tuple, s_forecast_scratch, WEATHER_FORECAST_MAX_POINTS);

        if (count > 0) {
            weatherData->RainForecastMmX10 = malloc(count * sizeof(int));
            if (weatherData->RainForecastMmX10) {
                memcpy(weatherData->RainForecastMmX10, s_forecast_scratch, count * sizeof(int));
                weatherData->RainForecastMmX10Count = count;
                weatherData->is_rain_forecast_dynamic_alloc = true;
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

static bool is_weather_key(uint32_t key) {
    return (key == MESSAGE_KEY_WEATHER_TEMPERATURE_CURRENT ||
            key == MESSAGE_KEY_WEATHER_TEMPERATURE_MIN ||
            key == MESSAGE_KEY_WEATHER_TEMPERATURE_MAX ||
            key == MESSAGE_KEY_WEATHER_CONDITION ||
            key == MESSAGE_KEY_WEATHER_RAIN_NEXT_HOUR_MM_X10 ||
            key == MESSAGE_KEY_WEATHER_RAIN_POP_PERCENT ||
            key == MESSAGE_KEY_WEATHER_TEMP_FORECAST_ENCODED ||
            key == MESSAGE_KEY_WEATHER_RAIN_FORECAST_MM_X10_ENCODED ||
            key == MESSAGE_KEY_WEATHER_FORECAST_START_TS);
}

static void handle_weather_message(DictionaryIterator *iterator) {
    Tuple *temp_cur_tuple = NULL;
    Tuple *temp_min_tuple = NULL;
    Tuple *temp_max_tuple = NULL;
    Tuple *condition_tuple = NULL;
    Tuple *rain_next_hour_tuple = NULL;
    Tuple *rain_pop_percent_tuple = NULL;
    Tuple *temp_forecast_encoded_tuple = NULL;
    Tuple *rain_forecast_encoded_tuple = NULL;
    Tuple *forecast_start_ts_tuple = NULL;

    for (Tuple *t = dict_read_first(iterator); t != NULL; t = dict_read_next(iterator)) {
        uint32_t key = t->key;

        if (key == MESSAGE_KEY_WEATHER_TEMPERATURE_CURRENT) {
            temp_cur_tuple = t;
        } else if (key == MESSAGE_KEY_WEATHER_TEMPERATURE_MIN) {
            temp_min_tuple = t;
        } else if (key == MESSAGE_KEY_WEATHER_TEMPERATURE_MAX) {
            temp_max_tuple = t;
        } else if (key == MESSAGE_KEY_WEATHER_CONDITION) {
            condition_tuple = t;
        } else if (key == MESSAGE_KEY_WEATHER_RAIN_NEXT_HOUR_MM_X10) {
            rain_next_hour_tuple = t;
        } else if (key == MESSAGE_KEY_WEATHER_RAIN_POP_PERCENT) {
            rain_pop_percent_tuple = t;
        } else if (key == MESSAGE_KEY_WEATHER_TEMP_FORECAST_ENCODED) {
            temp_forecast_encoded_tuple = t;
        } else if (key == MESSAGE_KEY_WEATHER_RAIN_FORECAST_MM_X10_ENCODED) {
            rain_forecast_encoded_tuple = t;
        } else if (key == MESSAGE_KEY_WEATHER_FORECAST_START_TS) {
            forecast_start_ts_tuple = t;
        }
    }

    if (temp_cur_tuple && temp_min_tuple && temp_max_tuple && condition_tuple) {
        process_weather_data(
            temp_cur_tuple,
            temp_min_tuple,
            temp_max_tuple,
            condition_tuple,
            rain_next_hour_tuple,
            rain_pop_percent_tuple,
            temp_forecast_encoded_tuple,
            rain_forecast_encoded_tuple,
            forecast_start_ts_tuple
        );
    }
}

static void handle_settings_message(DictionaryIterator *iterator) {
    ClaySettings *settings = clay_get_settings();
    bool has_settings_update = false;

    for (Tuple *t = dict_read_first(iterator); t != NULL; t = dict_read_next(iterator)) {
        uint32_t key = t->key;

        if (key == MESSAGE_KEY_Theme) {
            apply_cstring_setting_from_tuple(t, settings->ThemeValue, sizeof(settings->ThemeValue));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_BackgroundColor) {
            settings->BackgroundColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_TimeTextColor) {
            settings->TimeTextColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_DateTextColor) {
            settings->DateTextColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_WeekdayTextColor) {
            settings->WeekdayTextColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_DateSeparatorColor) {
            settings->DateSeparatorColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_BatteryFrameColor) {
            settings->BatteryFrameColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_BatteryFillColor) {
            settings->BatteryFillColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_BatteryLowColor) {
            settings->BatteryLowColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_WeatherTextColor) {
            settings->WeatherTextColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_WeatherMaxTempColor) {
            settings->WeatherMaxTempColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_WeatherCurrentTempColor) {
            settings->WeatherCurrentTempColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_WeatherMinTempColor) {
            settings->WeatherMinTempColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_WeatherAxisTickColor) {
            settings->WeatherAxisTickColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_WeatherIndicatorColor) {
            settings->WeatherIndicatorColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_WeatherSeparatorColor) {
            settings->WeatherSeparatorColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        }
#if defined(PBL_COLOR)
        else if (key == MESSAGE_KEY_ForecastTempColorM10) {
            settings->ForecastTempColorM10 = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_ForecastTempColor0) {
            settings->ForecastTempColor0 = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_ForecastTempColor10) {
            settings->ForecastTempColor10 = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_ForecastTempColor20) {
            settings->ForecastTempColor20 = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_ForecastTempColor30) {
            settings->ForecastTempColor30 = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_ForecastTempColor40) {
            settings->ForecastTempColor40 = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_ForecastRainColor0) {
            settings->ForecastRainColor0 = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_ForecastRainColor3) {
            settings->ForecastRainColor3 = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_ForecastRainColor10) {
            settings->ForecastRainColor10 = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_ForecastRainColor50) {
            settings->ForecastRainColor50 = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_ForecastRainColor100) {
            settings->ForecastRainColor100 = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        }
#endif
        else if (key == MESSAGE_KEY_StepcountTextColor) {
            settings->StepcountTextColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_HeartrateTextColor) {
            settings->HeartrateTextColor = GColorFromHEX(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_WeatherSlot1) {
            settings->WeatherSlot1 = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_WeatherSlot2) {
            settings->WeatherSlot2 = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_WeatherSlot3) {
            settings->WeatherSlot3 = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_LowBatteryThreshold) {
            settings->LowBatteryThreshold = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_BatteryWidth) {
            settings->BatteryWidth = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_SliderDigitWidth) {
            settings->DigitWidth = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_SliderDotWidth) {
            settings->DotWidth = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_SliderDotHeight) {
            settings->DotHeight = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_SliderDotHorizontalGap) {
            settings->DotHorizontalGap = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_SliderDotVerticalGap) {
            settings->DotVerticalGap = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_ToggleDotAutoScale) {
            settings->DotAutoScale = (tuple_to_int(t) == 1);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_SliderDotScaleFactorPercent) {
            settings->DotScaleFactor = (float) tuple_to_int(t) / 100.0f;
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_ShowYear) {
            settings->ShowYear = (tuple_to_int(t) == 1);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_ShowSeconds) {
            settings->ShowSeconds = (tuple_to_int(t) == 1);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_ShowWeekdayAbbreviation) {
            settings->ShowWeekdayAbbreviation = (tuple_to_int(t) == 1);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_WeekdayAbbreviationUppercase) {
            settings->WeekdayAbbreviationUppercase = (tuple_to_int(t) == 1);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_DateZeroPadding) {
            settings->DateZeroPadding = (tuple_to_int(t) == 1);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_WeatherUseSimulation) {
            settings->WeatherUseSimulation = (tuple_to_int(t) == 1);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_SliderWeatherForecastPreviewHoursCount) {
            settings->SliderWeatherForecastPreviewHoursCount = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_WeatherUpdateIntervalMinutes) {
            settings->WeatherUpdateIntervalMinutes = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_Row0Widget) {
            settings->Row0Widget = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_Row1Widget) {
            settings->Row1Widget = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_Row2Widget) {
            settings->Row2Widget = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_Row3Widget) {
            settings->Row3Widget = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_Row4Widget) {
            settings->Row4Widget = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_Row5Widget) {
            settings->Row5Widget = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_Row6Widget) {
            settings->Row6Widget = tuple_to_int(t);
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_LayoutRowCount) {
            settings->LayoutRowCount = clamp_layout_row_count(tuple_to_int(t));
            has_settings_update = true;
        } else if (key == MESSAGE_KEY_SliderTimeRowRatioPercent) {
            settings->TimeRowRatio = (float) tuple_to_int(t) / 100.0f;
            has_settings_update = true;
        }
    }

    if (has_settings_update) {
        settings->InitialSyncDone = true;
        clay_log_settings_debug("received settings update", settings);
        clay_save_settings(settings);
        main_reload_layout(settings, main_get_window());
    }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    Tuple *first = dict_read_first(iterator);
    if (first == NULL) {
        return;
    }

    if (is_weather_key(first->key)) {
        handle_weather_message(iterator);
    } else {
        handle_settings_message(iterator);
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