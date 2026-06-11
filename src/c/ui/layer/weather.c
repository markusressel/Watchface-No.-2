#include "weather.h"
#include <string.h>
#include <stdlib.h>
#include "../../developer_options.h"
#include "../../ui/theme.h"
#include "../../settings/clay_settings.h"
#include "dotted_text_layer.h"
#include "../../ui/layer_factory.h"
#include "weather_forecast.h"
#include "../ui_state.h"

static char s_buffer[32];

static WeatherData s_weather_data;
static WeatherData s_mock_weather_data;
static int s_runtime_temp_forecast[WEATHER_FORECAST_MAX_POINTS];
static int s_runtime_rain_forecast[WEATHER_FORECAST_MAX_POINTS];

static const uint16_t WEATHER_DATA_PERSIST_VERSION = 1;

typedef struct PersistedWeatherData {
    uint16_t Version;
    int CurrentTemperature;
    int MaxTemperature;
    int MinTemperature;
    int RainNextHourMmX10;
    int RainPopPercent;
    int TemperatureForecastCount;
    int TemperatureForecast[WEATHER_FORECAST_MAX_POINTS];
    int RainForecastMmX10Count;
    int RainForecastMmX10[WEATHER_FORECAST_MAX_POINTS];
    char CurrentConditions[48];
} PersistedWeatherData;

static int s_mock_temp_forecast[50] = {
    18, 20, 22, 24, 23, 21, 19, 17, 16, 15,
    17, 20, 23, 26, 28, 30, 29, 27, 25, 24,
    26, 28, 31, 33, 34, 32, 30, 28, 27, 25,
    22, 19, 16, 13, 10, 8, 7, 6, 5, 4,
    2, 0, -2, -4, -3, -1, 1, 3, 5, 7
};
static int s_mock_rain_forecast[] = {
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
    .CurrentConditions = "Mock",
};

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

// Timer to update weather after the given amount of time
#ifdef WF_EMULATOR
static int s_weather_update_interval = 1000 * 60 * 1; // 1 minute
#else
static int s_weather_update_interval = 1000 * 60 * 30; // 30 minutes
#endif

static AppTimer *s_update_timer = NULL;

static int s_active_weather_layers = 0;

WeatherData *weather_get_data() {
    if (clay_get_settings()->WeatherUseSimulation) {
        memcpy(&s_mock_weather_data, &s_mock_weather_data_template, sizeof(WeatherData));
        return &s_mock_weather_data;
    }

    ensure_runtime_forecast_storage();
    return &s_weather_data;
}

static void restore_saved_weather_data() {
    memset(&s_weather_data, 0, sizeof(s_weather_data));
    ensure_runtime_forecast_storage();

    if (!persist_exists(WEATHER_DATA_KEY)) {
        return;
    }

    PersistedWeatherData persisted = {0};
    const int bytes = persist_read_data(WEATHER_DATA_KEY, &persisted, sizeof(persisted));
    if (bytes != sizeof(persisted) || persisted.Version != WEATHER_DATA_PERSIST_VERSION) {
        memset(&s_weather_data, 0, sizeof(s_weather_data));
        ensure_runtime_forecast_storage();
        return;
    }

    s_weather_data.CurrentTemperature = persisted.CurrentTemperature;
    s_weather_data.MaxTemperature = persisted.MaxTemperature;
    s_weather_data.MinTemperature = persisted.MinTemperature;
    s_weather_data.RainNextHourMmX10 = persisted.RainNextHourMmX10;
    s_weather_data.RainPopPercent = persisted.RainPopPercent;

    s_weather_data.TemperatureForecastCount = clamp_forecast_count(persisted.TemperatureForecastCount);
    s_weather_data.RainForecastMmX10Count = clamp_forecast_count(persisted.RainForecastMmX10Count);

    memcpy(
        s_weather_data.TemperatureForecast,
        persisted.TemperatureForecast,
        sizeof(int) * s_weather_data.TemperatureForecastCount
    );
    memcpy(
        s_weather_data.RainForecastMmX10,
        persisted.RainForecastMmX10,
        sizeof(int) * s_weather_data.RainForecastMmX10Count
    );

    strncpy(s_weather_data.CurrentConditions, persisted.CurrentConditions, sizeof(s_weather_data.CurrentConditions) - 1);
    s_weather_data.CurrentConditions[sizeof(s_weather_data.CurrentConditions) - 1] = '\0';
    sanitize_weather_data();
}

void weather_delete_persisted_data() {
    if (!persist_exists(WEATHER_DATA_KEY)) {
        return;
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "deleting persisted weather data");
    persist_delete(WEATHER_DATA_KEY);
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

static void request_weather_update() {
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

static void on_scheduled_update_triggered(void *data) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "scheduled weather update triggered!");

    // The timer has fired, so the handle is now dead.
    s_update_timer = NULL;

    // send AppMessage to trigger weather update via JS
    request_weather_update();

    //Register next execution
    schedule_next_update(s_weather_update_interval, on_scheduled_update_triggered);
}

static void update_weather_for_layer(DottedTextLayer *weather_layer) {
    WeatherData *data = weather_get_data();
    if (data == NULL) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "Weather data is NULL!");
        return;
    }

    // Write the current temperature into a buffer
    snprintf(s_buffer, sizeof(s_buffer), "%d|%d", data->MaxTemperature, data->MinTemperature);

    // update text layer
    dotted_text_layer_set_text(weather_layer, s_buffer);
}

// Backward compatible wrapper (called by app messaging or other code)
void update_weather() {
    WeatherData *data = weather_get_data();
    if (data != NULL) {
        // persist current data for fast access when opening the watchface
        save_current_weather_data(data);

        APP_LOG(
            APP_LOG_LEVEL_DEBUG,
            "weather rain forecast: next_1h=%d.%dmm pop=%d%%",
            data->RainNextHourMmX10 / 10,
            data->RainNextHourMmX10 % 10,
            data->RainPopPercent
        );
    }

    for (int i = 0; i < ui_state_get_row_count(); i++) {
        if (ui_state_get_widget_id(i) == WIDGET_WEATHER) {
            update_weather_for_layer((DottedTextLayer *) ui_state_get_layer(i));
        }
    }
}

Layer *create_weather_layer(LayerBuilder builder) {
    restore_saved_weather_data();

    DottedTextLayer *weather_layer = layer_factory_create_dotted_text_layer(
        builder,
        theme_get_theme()->WeatherTextColor,
        HORIZONTAL_ALIGN_RIGHT,
        VERTICAL_ALIGN_TOP,
        "---"
    );
    if (clay_get_settings()->DotAutoScale) {
        dotted_text_layer_set_auto_scale(weather_layer, true);
    } else {
        dotted_text_layer_set_scale_factor(weather_layer, clay_get_settings()->DotScaleFactor);
    }

    s_active_weather_layers++;

    update_weather_for_layer(weather_layer);

    if (s_update_timer == NULL) {
        s_update_timer = app_timer_register(s_weather_update_interval, on_scheduled_update_triggered, NULL);
    }

    return (Layer *) weather_layer;
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

    dotted_text_layer_destroy((DottedTextLayer *) layer);
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