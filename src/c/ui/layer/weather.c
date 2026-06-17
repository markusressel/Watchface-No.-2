#include <pebble.h>
#include "weather.h"
#include "../../developer_options.h"
#include "../../ui/theme.h"
#include "../../settings/clay_settings.h"
#include "dotted_text_layer.h"
#include "weather_forecast.h"
#include "../ui_state.h"
#include "../../system/phone_connection.h"

static char s_buffer[32];

static WeatherData s_weather_data;
static WeatherData s_mock_weather_data;
static int s_runtime_temp_forecast[WEATHER_FORECAST_MAX_POINTS];
static int s_runtime_rain_forecast[WEATHER_FORECAST_MAX_POINTS];

static const uint16_t WEATHER_DATA_PERSIST_VERSION = 2;

typedef struct PersistedWeatherData {
    uint16_t Version;
    int CurrentTemperature;
    int MaxTemperature;
    int MinTemperature;
    int RainNextHourMmX10;
    int RainPopPercent;
    time_t ForecastStartTimestamp;
    int TemperatureForecastCount;
    int TemperatureForecast[WEATHER_FORECAST_MAX_POINTS];
    int RainForecastMmX10Count;
    int RainForecastMmX10[WEATHER_FORECAST_MAX_POINTS];
    char CurrentConditions[48];
} PersistedWeatherData;

static int s_mock_temp_forecast[WEATHER_FORECAST_MAX_POINTS] = {
    18, 20, 22, 24, 23, 21, 19, 17, 16, 15,
    17, 20, 23, 26, 28, 30, 29, 27, 25, 24,
    26, 28, 31, 33, 34, 32, 30, 28, 27, 25,
    22, 19, 16, 13, 10, 8, 7, 6, 5, 4,
    2, 0, -2, -4, -3, -1, 1, 3, 5, 7
};
static int s_mock_rain_forecast[WEATHER_FORECAST_MAX_POINTS] = {
    0, 0, 0, 0, 0, 5, 10, 8, 3, 0,
    0, 0, 0, 0, 0, 20, 50, 100, 30, 5,
    0, 0, 0, 0, 0, 0, 10, 30, 80, 20,
    0, 5, 15, 25, 20, 10, 5, 0, 0, 0,
    0, 0, 0, 5, 10, 15, 10, 5, 0, 0
};
static const WeatherData s_mock_weather_data_template = {
    .CurrentTemperature = 26,
    .MaxTemperature = 27,
    .MinTemperature = 19,
    .RainNextHourMmX10 = 0,
    .RainPopPercent = 0,
    .TemperatureForecastCount = sizeof(s_mock_temp_forecast) / sizeof(s_mock_temp_forecast[0]),
    .TemperatureForecast = s_mock_temp_forecast,
    .RainForecastMmX10Count = sizeof(s_mock_rain_forecast) / sizeof(s_mock_rain_forecast[0]),
    .RainForecastMmX10 = s_mock_rain_forecast,
    .ForecastStartTimestamp = 0,
    .CurrentConditions = "Mock",
};

static int weather_get_current_temp(WeatherData *data);
static void update_weather_ui();

static void ensure_runtime_forecast_storage() {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "ensuring runtime forecast storage");
    // Only set default pointers if they are not dynamically allocated
    if (!s_weather_data.is_temp_forecast_dynamic_alloc && s_weather_data.TemperatureForecast == NULL) {
        s_weather_data.TemperatureForecast = s_runtime_temp_forecast;
    }
    if (!s_weather_data.is_rain_forecast_dynamic_alloc && s_weather_data.RainForecastMmX10 == NULL) {
        s_weather_data.RainForecastMmX10 = s_runtime_rain_forecast;
    }
}

static int clamp_forecast_count(const int value) {
    if (value < 0) {
        return 0;
    }
    if (value > WEATHER_FORECAST_MAX_POINTS) {
        return WEATHER_FORECAST_MAX_POINTS;
    }
    return value;
}

static void sanitize_weather_data() {
    ensure_runtime_forecast_storage();

    s_weather_data.TemperatureForecastCount = clamp_forecast_count(s_weather_data.TemperatureForecastCount);
    s_weather_data.RainForecastMmX10Count = clamp_forecast_count(s_weather_data.RainForecastMmX10Count);

    if (memchr(s_weather_data.CurrentConditions, '\0', sizeof(s_weather_data.CurrentConditions)) == NULL) {
        s_weather_data.CurrentConditions[0] = '\0';
    }
    s_weather_data.CurrentConditions[sizeof(s_weather_data.CurrentConditions) - 1] = '\0';
}

static AppTimer *s_update_timer = NULL;
static int s_active_weather_layers = 0;

WeatherData *weather_get_data() {
    if (clay_get_settings()->WeatherUseSimulation) {
        memcpy(&s_mock_weather_data, &s_mock_weather_data_template, sizeof(WeatherData));
        // set timestamp to a fixed point in the past (e.g. start of current hour) for testing
        // this prevents the "treadmill" effect where the indicator never moves relative to now.
        const time_t now = time(NULL);
        const struct tm *t = localtime(&now);
        struct tm start_of_hour = *t;
        start_of_hour.tm_min = 0;
        start_of_hour.tm_sec = 0;
        s_mock_weather_data.ForecastStartTimestamp = mktime(&start_of_hour);
        return &s_mock_weather_data;
    }

    ensure_runtime_forecast_storage();
    return &s_weather_data;
}

static bool s_weather_data_initialized = false;

static void restore_saved_weather_data() {
    if (s_weather_data_initialized) {
        return;
    }

    ensure_runtime_forecast_storage();

    if (!persist_exists(WEATHER_DATA_KEY)) {
        s_weather_data_initialized = true;
        return;
    }

    PersistedWeatherData persisted = {0};
    const int bytes = persist_read_data(WEATHER_DATA_KEY, &persisted, sizeof(persisted));
    if (bytes != sizeof(persisted) || persisted.Version != WEATHER_DATA_PERSIST_VERSION) {
        s_weather_data_initialized = true;
        return;
    }

    s_weather_data.CurrentTemperature = persisted.CurrentTemperature;
    s_weather_data.MaxTemperature = persisted.MaxTemperature;
    s_weather_data.MinTemperature = persisted.MinTemperature;
    s_weather_data.RainNextHourMmX10 = persisted.RainNextHourMmX10;
    s_weather_data.RainPopPercent = persisted.RainPopPercent;
    s_weather_data.ForecastStartTimestamp = persisted.ForecastStartTimestamp;

    s_weather_data.TemperatureForecastCount = clamp_forecast_count(persisted.TemperatureForecastCount);
    s_weather_data.RainForecastMmX10Count = clamp_forecast_count(persisted.RainForecastMmX10Count);

    if (s_weather_data.TemperatureForecastCount > 0) {
        memcpy(
            s_weather_data.TemperatureForecast,
            persisted.TemperatureForecast,
            sizeof(int) * s_weather_data.TemperatureForecastCount
        );
    }
    if (s_weather_data.RainForecastMmX10Count > 0) {
        memcpy(
            s_weather_data.RainForecastMmX10,
            persisted.RainForecastMmX10,
            sizeof(int) * s_weather_data.RainForecastMmX10Count
        );
    }

    strncpy(s_weather_data.CurrentConditions, persisted.CurrentConditions, sizeof(s_weather_data.CurrentConditions) - 1);
    s_weather_data.CurrentConditions[sizeof(s_weather_data.CurrentConditions) - 1] = '\0';
    sanitize_weather_data();
    s_weather_data_initialized = true;
}

void weather_init_data() {
    restore_saved_weather_data();
}

void weather_delete_persisted_data() {
    if (!persist_exists(WEATHER_DATA_KEY)) {
        return;
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "deleting persisted weather data");
    persist_delete(WEATHER_DATA_KEY);
}

static void weather_clear_data() {
    memset(&s_weather_data, 0, sizeof(WeatherData));
    weather_delete_persisted_data();
    update_weather();
    update_weather_forecast();
}

void weather_tick_update() {
    if (s_weather_data.ForecastStartTimestamp <= 0) {
        return;
    }

    ClaySettings *settings = clay_get_settings();
    const int forecast_points = settings->SliderWeatherForecastPreviewHoursCount * 4; // 15 min points
    const time_t max_valid_time = s_weather_data.ForecastStartTimestamp + (forecast_points * 15 * 60);

    if (time(NULL) > max_valid_time) {
        APP_LOG(APP_LOG_LEVEL_INFO, "Weather data expired. Clearing.");
        weather_clear_data();
    } else {
        update_weather_ui();
    }
}


static void save_current_weather_data(WeatherData *weather_data) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "saving current weather data: %p", weather_data);

    if (clay_get_settings()->WeatherUseSimulation) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "skipping saving weather data in simulation mode");
        return;
    }

    if (weather_data == NULL) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "cannot save NULL weather data");
        weather_delete_persisted_data();
        return;
    }

    ensure_runtime_forecast_storage();

    // PersistedWeatherData is too big to be allocated on the stack (will likely result in stackoverflow), so we use malloc here to allocate it on the heap instead
    PersistedWeatherData *persisted = malloc(sizeof(PersistedWeatherData));
    if (persisted == NULL) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to allocate memory for persisted weather data");
        return;
    }
    memset(persisted, 0, sizeof(PersistedWeatherData));

    persisted->Version = WEATHER_DATA_PERSIST_VERSION;
    persisted->CurrentTemperature = weather_data->CurrentTemperature;
    persisted->MaxTemperature = weather_data->MaxTemperature;
    persisted->MinTemperature = weather_data->MinTemperature;
    persisted->RainNextHourMmX10 = weather_data->RainNextHourMmX10;
    persisted->RainPopPercent = weather_data->RainPopPercent;
    persisted->ForecastStartTimestamp = weather_data->ForecastStartTimestamp;
    persisted->TemperatureForecastCount = clamp_forecast_count(weather_data->TemperatureForecastCount);
    persisted->RainForecastMmX10Count = clamp_forecast_count(weather_data->RainForecastMmX10Count);

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Copying forecast arrays");

    if (weather_data->TemperatureForecast && persisted->TemperatureForecastCount > 0) {
        memcpy(
            persisted->TemperatureForecast,
            weather_data->TemperatureForecast,
            sizeof(int) * persisted->TemperatureForecastCount
        );
    }
    if (weather_data->RainForecastMmX10 && persisted->RainForecastMmX10Count > 0) {
        memcpy(
            persisted->RainForecastMmX10,
            weather_data->RainForecastMmX10,
            sizeof(int) * persisted->RainForecastMmX10Count
        );
    }

    strncpy(persisted->CurrentConditions, weather_data->CurrentConditions, sizeof(persisted->CurrentConditions) - 1);
    persisted->CurrentConditions[sizeof(persisted->CurrentConditions) - 1] = '\0';

    persist_write_data(WEATHER_DATA_KEY, persisted, sizeof(PersistedWeatherData));
    free(persisted);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "saved weather data");
}

void weather_request_update() {
    if (!phone_connection_is_connected()) {
        APP_LOG(APP_LOG_LEVEL_INFO, "Skipping weather update: Phone not connected.");
        return;
    }

    // Declare the dictionary's iterator
    DictionaryIterator *out_iter;

    // Prepare the outbox buffer for this message
    AppMessageResult result = app_message_outbox_begin(&out_iter);
    if (result != APP_MSG_OK) {
        // The outbox cannot be used right now
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int) result);
        return;
    }

    // Construct the message with a dummy value
    int value = 0;
    // Add an item to ask for weather data
    dict_write_int(out_iter, MESSAGE_KEY_RequestWeatherData, &value, sizeof(int), true);

    APP_LOG(APP_LOG_LEVEL_INFO, "Sending RequestUpdate message for weather...");

    // Send this message
    result = app_message_outbox_send();

    // Check the result
    if (result != APP_MSG_OK) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int) result);
    }
}

static void cancel_update_timer() {
    if (s_update_timer != NULL) {
        // cancel weather update timer
        app_timer_cancel(s_update_timer);
        s_update_timer = NULL;
    }
}

static void schedule_next_update(const int interval, AppTimerCallback callback) {
    cancel_update_timer();
    s_update_timer = app_timer_register(interval, callback, NULL);
}

/**
 * The API updates every +/15 minutes, so we schedule a weather update request for
 * 1 minute past the next 15-minute interval (to make sure we request new data 100% after the previous request).
 * @return
 */
static int compute_next_weather_update_request_ms() {
    const int interval_mins = clay_get_settings()->WeatherUpdateIntervalMinutes;
    const time_t now = time(NULL);
    const struct tm *time_now = localtime(&now);

    // Create a tm struct for the start of the current day
    struct tm day_start_tm = *time_now;
    day_start_tm.tm_hour = 0;
    day_start_tm.tm_min = 0;
    day_start_tm.tm_sec = 0;

    const time_t day_start = mktime(&day_start_tm);
    const int seconds_since_day_start = now - day_start;
    const int interval_secs = interval_mins * 60;

    // We want the next update at (k * interval_secs + 60) seconds since day start
    int next_boundary_secs = ((seconds_since_day_start / interval_secs) + 1) * interval_secs + 60;

    // Check if we are still before the 1-minute mark of the current interval
    const int current_interval_boundary_secs = (seconds_since_day_start / interval_secs) * interval_secs + 60;
    if (seconds_since_day_start < current_interval_boundary_secs) {
        next_boundary_secs = current_interval_boundary_secs;
    }

    const int seconds_to_wait = next_boundary_secs - seconds_since_day_start;

    return seconds_to_wait * 1000;
}

static void on_scheduled_update_triggered(void *data) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "scheduled weather update triggered!");

    // The timer has fired, so the handle is now dead.
    s_update_timer = NULL;

    // send AppMessage to trigger weather update via JS
    weather_request_update();

    //Register next execution
    const int next_request_ms = compute_next_weather_update_request_ms();
    schedule_next_update(next_request_ms, on_scheduled_update_triggered);
}

typedef enum WeatherValueType {
    WEATHER_VALUE_TYPE_NONE = 0,
    WEATHER_VALUE_TYPE_CURRENT = 1,
    WEATHER_VALUE_TYPE_MAX = 2,
    WEATHER_VALUE_TYPE_MIN = 3
} WeatherValueType;

typedef struct WeatherLayerData {
    DottedTextLayer *slots[3];
    DottedTextLayer *separators[2];
} WeatherLayerData;

static void weather_layer_update_proc(Layer *layer, GContext *ctx) {
    WeatherLayerData *data = layer_get_data(layer);
    GRect bounds = layer_get_bounds(layer);

    int slot_widths[3] = {0};
    int sep_widths[2] = {0};
    int total_width = 0;
    const int spacing = 2; // pixel gap between elements

    // 1. Calculate widths and visibility
    for (int i = 0; i < 3; i++) {
        if (data->slots[i]) {
            slot_widths[i] = dotted_text_layer_get_content_width(data->slots[i]);
            total_width += slot_widths[i];
        }
    }

    // Separators are visible only if both adjacent slots have content
    for (int i = 0; i < 2; i++) {
        if (data->separators[i]) {
            if (slot_widths[i] > 0 && slot_widths[i + 1] > 0) {
                dotted_text_layer_set_text(data->separators[i], "|");
                sep_widths[i] = dotted_text_layer_get_content_width(data->separators[i]);
                total_width += sep_widths[i];
            } else {
                dotted_text_layer_set_text(data->separators[i], NULL);
                sep_widths[i] = 0;
            }
        }
    }

    int visible_elements = 0;
    for (int i = 0; i < 3; i++) if (slot_widths[i] > 0) visible_elements++;
    for (int i = 0; i < 2; i++) if (sep_widths[i] > 0) visible_elements++;

    if (visible_elements > 0) {
        total_width += (visible_elements - 1) * spacing;
    }

    // 2. Position child layers (Right aligned)
    int current_x = bounds.size.w - total_width;
    for (int i = 0; i < 3; i++) {
        // Slot i
        if (data->slots[i] && slot_widths[i] > 0) {
            layer_set_frame(data->slots[i], GRect(current_x, 0, slot_widths[i], bounds.size.h));
            current_x += slot_widths[i] + spacing;
        }
        // Separator after slot i
        if (i < 2 && data->separators[i] && sep_widths[i] > 0) {
            layer_set_frame(data->separators[i], GRect(current_x, 0, sep_widths[i], bounds.size.h));
            current_x += sep_widths[i] + spacing;
        }
    }
}

static void update_weather_for_layer(Layer *weather_container_layer) {
    WeatherLayerData *layer_data = layer_get_data(weather_container_layer);
    WeatherData *weather_data = weather_get_data();
    ClaySettings *settings = clay_get_settings();
    Theme *theme = theme_get_theme();

    if (weather_data == NULL) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "Weather data is NULL!");
        return;
    }

    int slot_configs[3] = {
        settings->WeatherSlot1,
        settings->WeatherSlot2,
        settings->WeatherSlot3
    };

    for (int i = 0; i < 3; i++) {
        if (layer_data->slots[i] == NULL) continue;

        int value = 0;
        GColor color = theme->WeatherTextColor;
        bool has_value = true;

        switch (slot_configs[i]) {
            case WEATHER_VALUE_TYPE_CURRENT:
                value = weather_get_current_temp(weather_data);
                color = theme->WeatherCurrentTempColor;
                break;
            case WEATHER_VALUE_TYPE_MAX:
                value = weather_data->MaxTemperature;
                color = theme->WeatherMaxTempColor;
                break;
            case WEATHER_VALUE_TYPE_MIN:
                value = weather_data->MinTemperature;
                color = theme->WeatherMinTempColor;
                break;
            default:
                has_value = false;
                break;
        }

        if (has_value) {
            snprintf(s_buffer, sizeof(s_buffer), "%d", value);
            dotted_text_layer_set_text(layer_data->slots[i], s_buffer);
            dotted_text_layer_set_color(layer_data->slots[i], color);
        } else {
            dotted_text_layer_set_text(layer_data->slots[i], NULL);
        }
    }

    layer_mark_dirty(weather_container_layer);
}

static int weather_get_current_temp(WeatherData *data) {
    if (data == NULL) {
        return 0;
    }

    if (data->ForecastStartTimestamp <= 0 || data->TemperatureForecastCount <= 0 || data->TemperatureForecast == NULL) {
        return data->CurrentTemperature;
    }

    const time_t now = time(NULL);
    const int interval = 15 * 60; // 15 minutes

    // Calculate how many intervals have passed since the forecast started
    int seconds_since_start = (int) (now - data->ForecastStartTimestamp);

    // If we are before the forecast start, just return the received current temp
    if (seconds_since_start < 0) {
        return data->CurrentTemperature;
    }

    // Find the closest forecast index
    int index = (seconds_since_start + (interval / 2)) / interval;

    // Only switch to forecast data if we are closer to a later forecast entry (index > 0)
    // than to the start time/current temp.
    if (index <= 0) {
        return data->CurrentTemperature;
    }

    if (index >= data->TemperatureForecastCount) {
        return data->TemperatureForecast[data->TemperatureForecastCount - 1];
    }

    return data->TemperatureForecast[index];
}

static void update_weather_ui() {
    for (int i = 0; i < ui_state_get_row_count(); i++) {
        if (ui_state_get_widget_id(i) == WIDGET_WEATHER) {
            update_weather_for_layer(ui_state_get_layer(i));
        }
    }
}

// update all weather layer instances (backward compatible wrapper)
void update_weather() {
    WeatherData *data = weather_get_data();
    if (data != NULL) {
        // persist current data for fast access when opening the watchface
        save_current_weather_data(data);

        APP_LOG(
            APP_LOG_LEVEL_DEBUG,
            "weather rain update: next_1h=%d.%dmm pop=%d%%",
            data->RainNextHourMmX10 / 10,
            data->RainNextHourMmX10 % 10,
            data->RainPopPercent
        );
    }

    update_weather_ui();
}

Layer *create_weather_layer(LayerBuilder builder) {
    Layer *container = layer_factory_create_custom_layer_with_data(
        builder,
        weather_layer_update_proc,
        sizeof(WeatherLayerData)
    );
    WeatherLayerData *data = layer_get_data(container);

    ClaySettings *settings = clay_get_settings();
    Theme *theme = theme_get_theme();
    LayerBuilder child_builder = layer_builder_from_rect(container, GRect(0, 0, builder.bounds.size.w, builder.bounds.size.h));

    for (int i = 0; i < 3; i++) {
        data->slots[i] = layer_factory_create_dotted_text_layer(
            child_builder,
            theme->WeatherTextColor,
            HORIZONTAL_ALIGN_LEFT,
            VERTICAL_ALIGN_TOP,
            NULL
        );
        if (settings->DotAutoScale) {
            dotted_text_layer_set_auto_scale(data->slots[i], true);
        } else {
            dotted_text_layer_set_scale_factor(data->slots[i], settings->DotScaleFactor);
        }

        if (i < 2) {
            data->separators[i] = layer_factory_create_dotted_text_layer(
                child_builder,
                GColorLightGray,
                HORIZONTAL_ALIGN_LEFT,
                VERTICAL_ALIGN_TOP,
                NULL
            );
            if (settings->DotAutoScale) {
                dotted_text_layer_set_auto_scale(data->separators[i], true);
            } else {
                dotted_text_layer_set_scale_factor(data->separators[i], settings->DotScaleFactor);
            }
        }
    }

    s_active_weather_layers++;

    update_weather_for_layer(container);

    if (s_update_timer == NULL) {
        const int next_request_ms = compute_next_weather_update_request_ms();
        s_update_timer = app_timer_register(next_request_ms, on_scheduled_update_triggered, NULL);
    }

    return container;
}

void destroy_weather_layer(Layer *layer) {
    s_active_weather_layers--;

    if (s_update_timer) {
        // cancel weather update timer only if this is the last instance
        if (s_active_weather_layers <= 0) {
            app_timer_cancel(s_update_timer);
            s_update_timer = NULL;
        }
    }

    WeatherLayerData *data = layer_get_data(layer);
    for (int i = 0; i < 3; i++) {
        if (data->slots[i]) {
            dotted_text_layer_destroy(data->slots[i]);
        }
        if (i < 2 && data->separators[i]) {
            dotted_text_layer_destroy(data->separators[i]);
        }
    }
    layer_destroy(layer);
}

void deinit_weather_data() {
    if (s_weather_data.is_temp_forecast_dynamic_alloc && s_weather_data.TemperatureForecast) {
        free(s_weather_data.TemperatureForecast);
        s_weather_data.is_temp_forecast_dynamic_alloc = false;
    }
    if (s_weather_data.is_rain_forecast_dynamic_alloc && s_weather_data.RainForecastMmX10) {
        free(s_weather_data.RainForecastMmX10);
        s_weather_data.is_rain_forecast_dynamic_alloc = false;
    }
    s_weather_data.TemperatureForecast = NULL;
    s_weather_data.RainForecastMmX10 = NULL;
    s_weather_data.TemperatureForecastCount = 0;
    s_weather_data.RainForecastMmX10Count = 0;
}
