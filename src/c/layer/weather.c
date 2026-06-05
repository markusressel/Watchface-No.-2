#include "weather.h"
#include <string.h>
#include "../developer_options.h"
#include "../theme.h"
#include "../clay_settings.h"
#include "../dotted_text_layer.h"
#include "../layer_factory.h"
#include "weather_forecast.h"

#define MAX_WEATHER_LAYERS 7

static char s_buffer[32];

static WeatherData weatherData;
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
    if (weatherData.is_dynamic_alloc) {
        return;
    }

    if (weatherData.TemperatureForecast == NULL) {
        weatherData.TemperatureForecast = s_runtime_temp_forecast;
    }
    if (weatherData.RainForecastMmX10 == NULL) {
        weatherData.RainForecastMmX10 = s_runtime_rain_forecast;
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

    weatherData.TemperatureForecastCount = clamp_forecast_count(weatherData.TemperatureForecastCount);
    weatherData.RainForecastMmX10Count = clamp_forecast_count(weatherData.RainForecastMmX10Count);

    if (memchr(weatherData.CurrentConditions, '\0', sizeof(weatherData.CurrentConditions)) == NULL) {
        weatherData.CurrentConditions[0] = '\0';
    }
    weatherData.CurrentConditions[sizeof(weatherData.CurrentConditions) - 1] = '\0';
}

// Timer to update weather after the given amount of time
#ifdef PBL_EMULATOR
static int s_weather_update_interval = 1000 * 60;
#else
static int s_weather_update_interval = 1800000;
#endif

static AppTimer *s_update_timer;

// Registry of all created weather layers
typedef struct {
    DottedTextLayer *dotted_text_layer;
} WeatherLayerInstance;

static WeatherLayerInstance s_weather_layers[MAX_WEATHER_LAYERS];
static int s_weather_layer_count = 0;
static ClaySettings *s_settings;

WeatherData *weather_get_data() {
    if (DEV_OPTIONS.UseMockWeatherData) {
        memcpy(&s_mock_weather_data, &s_mock_weather_data_template, sizeof(WeatherData));
        return &s_mock_weather_data;
    }

    ensure_runtime_forecast_storage();
    return &weatherData;
}

static void restore_saved_weather_data() {
    memset(&weatherData, 0, sizeof(weatherData));
    ensure_runtime_forecast_storage();

    if (!persist_exists(WEATHER_DATA_KEY)) {
        return;
    }

    PersistedWeatherData persisted = {0};
    const int bytes = persist_read_data(WEATHER_DATA_KEY, &persisted, sizeof(persisted));
    if (bytes != sizeof(persisted) || persisted.Version != WEATHER_DATA_PERSIST_VERSION) {
        memset(&weatherData, 0, sizeof(weatherData));
        ensure_runtime_forecast_storage();
        return;
    }

    weatherData.CurrentTemperature = persisted.CurrentTemperature;
    weatherData.MaxTemperature = persisted.MaxTemperature;
    weatherData.MinTemperature = persisted.MinTemperature;
    weatherData.RainNextHourMmX10 = persisted.RainNextHourMmX10;
    weatherData.RainPopPercent = persisted.RainPopPercent;

    weatherData.TemperatureForecastCount = clamp_forecast_count(persisted.TemperatureForecastCount);
    weatherData.RainForecastMmX10Count = clamp_forecast_count(persisted.RainForecastMmX10Count);

    memcpy(
        weatherData.TemperatureForecast,
        persisted.TemperatureForecast,
        sizeof(int) * weatherData.TemperatureForecastCount
    );
    memcpy(
        weatherData.RainForecastMmX10,
        persisted.RainForecastMmX10,
        sizeof(int) * weatherData.RainForecastMmX10Count
    );

    strncpy(weatherData.CurrentConditions, persisted.CurrentConditions, sizeof(weatherData.CurrentConditions) - 1);
    weatherData.CurrentConditions[sizeof(weatherData.CurrentConditions) - 1] = '\0';
    sanitize_weather_data();
}

static void save_current_weather_data() {
    if (DEV_OPTIONS.UseMockWeatherData) {
        return;
    }

    ensure_runtime_forecast_storage();

    PersistedWeatherData persisted = {
        .Version = WEATHER_DATA_PERSIST_VERSION,
        .CurrentTemperature = weatherData.CurrentTemperature,
        .MaxTemperature = weatherData.MaxTemperature,
        .MinTemperature = weatherData.MinTemperature,
        .RainNextHourMmX10 = weatherData.RainNextHourMmX10,
        .RainPopPercent = weatherData.RainPopPercent,
        .TemperatureForecastCount = clamp_forecast_count(weatherData.TemperatureForecastCount),
        .RainForecastMmX10Count = clamp_forecast_count(weatherData.RainForecastMmX10Count),
    };

    if (weatherData.TemperatureForecast && persisted.TemperatureForecastCount > 0) {
        memcpy(
            persisted.TemperatureForecast,
            weatherData.TemperatureForecast,
            sizeof(int) * persisted.TemperatureForecastCount
        );
    }
    if (weatherData.RainForecastMmX10 && persisted.RainForecastMmX10Count > 0) {
        memcpy(
            persisted.RainForecastMmX10,
            weatherData.RainForecastMmX10,
            sizeof(int) * persisted.RainForecastMmX10Count
        );
    }

    strncpy(persisted.CurrentConditions, weatherData.CurrentConditions, sizeof(persisted.CurrentConditions) - 1);
    persisted.CurrentConditions[sizeof(persisted.CurrentConditions) - 1] = '\0';

    persist_write_data(WEATHER_DATA_KEY, &persisted, sizeof(persisted));
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
    dict_write_int(out_iter, MESSAGE_KEY_RequestData, &value, sizeof(int), true);

    APP_LOG(APP_LOG_LEVEL_ERROR, "Sending RequestUpdate message for weather...");

    // Send this message
    result = app_message_outbox_send();

    // Check the result
    if (result != APP_MSG_OK) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int) result);
    }
}

static void cancel_update_timer() {
    if (s_update_timer) {
        // cancel weather update timer
        app_timer_cancel(s_update_timer);
        s_update_timer = NULL;
    }
}

static void schedule_next_update(const int interval, AppTimerCallback callback) {
    s_update_timer = app_timer_register(interval, callback, NULL);
}

static void on_scheduled_update_triggered(void *data) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "scheduled weather update triggered!");
    cancel_update_timer();

    // send AppMessage to trigger weather update via JS
    request_weather_update();

    //Register next execution
    schedule_next_update(s_weather_update_interval, on_scheduled_update_triggered);
}

// Update all weather layer instances
static void update_all_weather_layers() {
    WeatherData *data = weather_get_data();

    // persist current data for fast access when opening the watchface
    save_current_weather_data();

    APP_LOG(
        APP_LOG_LEVEL_DEBUG,
        "weather rain forecast: next_1h=%d.%dmm pop=%d%%",
        data->RainNextHourMmX10 / 10,
        data->RainNextHourMmX10 % 10,
        data->RainPopPercent
    );

    // Write the current temperature into a buffer
    snprintf(s_buffer, sizeof(s_buffer), "%d|%d", data->MaxTemperature, data->MinTemperature);

    for (int i = 0; i < s_weather_layer_count; i++) {
        // update text layer
        dotted_text_layer_set_text(s_weather_layers[i].dotted_text_layer, s_buffer);
    }

    update_temperature_forecast();
}

// Backward compatible wrapper (called by app messaging or other code)
void update_weather() {
    update_all_weather_layers();
}

Layer *create_weather_layer(LayerBuilder builder) {
    s_settings = clay_get_settings();
    restore_saved_weather_data();

    if (s_weather_layer_count >= MAX_WEATHER_LAYERS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Max weather layers exceeded!");
        return NULL;
    }

    WeatherLayerInstance *instance = &s_weather_layers[s_weather_layer_count];

    instance->dotted_text_layer = layer_factory_create_dotted_text_layer(
        builder,
        theme_get_theme()->WeatherTextColor,
        HORIZONTAL_ALIGN_RIGHT,
        VERTICAL_ALIGN_TOP,
        "---"
    );
    if (s_settings->DotAutoScale) {
        dotted_text_layer_set_auto_scale(instance->dotted_text_layer, true);
    } else {
        dotted_text_layer_set_scale_factor(instance->dotted_text_layer, s_settings->DotScaleFactor);
    }

    s_weather_layer_count++;

    update_all_weather_layers();

    s_update_timer = app_timer_register(s_weather_update_interval, on_scheduled_update_triggered, NULL);

    return instance->dotted_text_layer;
}

void destroy_weather_layer(Layer *layer) {
    DottedTextLayer *dotted_text_layer_to_destroy = layer;

    // Find and remove from registry
    for (int i = 0; i < s_weather_layer_count; i++) {
        if (s_weather_layers[i].dotted_text_layer == dotted_text_layer_to_destroy) {
            // Remove from array by shifting remaining elements
            for (int j = i; j < s_weather_layer_count - 1; j++) {
                s_weather_layers[j] = s_weather_layers[j + 1];
            }
            s_weather_layer_count--;
            break;
        }
    }

    if (s_update_timer) {
        // cancel weather update timer only if this is the last instance
        if (s_weather_layer_count == 0) {
            app_timer_cancel(s_update_timer);
            s_update_timer = NULL;
        }
    }

    dotted_text_layer_destroy(dotted_text_layer_to_destroy);
}

void deinit_weather_data() {
    if (weatherData.is_dynamic_alloc) {
        free(weatherData.TemperatureForecast);
        free(weatherData.RainForecastMmX10);
        weatherData.is_dynamic_alloc = false;
    }
    weatherData.TemperatureForecast = NULL;
    weatherData.RainForecastMmX10 = NULL;
    weatherData.TemperatureForecastCount = 0;
    weatherData.RainForecastMmX10Count = 0;
}
