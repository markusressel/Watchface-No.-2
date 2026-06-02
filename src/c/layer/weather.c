#include "weather.h"
#include <string.h>
#include "../theme.h"
#include "../clay_settings.h"
#include "../dotted_text_layer.h"
#include "../layer_factory.h"
#include "temperature_forecast.h"
#include "rain_forecast.h"

#define MAX_WEATHER_LAYERS 7

static char s_buffer[32];

static WeatherData weatherData;

// Timer to update weather after the given amount of time
static int s_weather_update_interval = 1800000;
//static int s_weather_update_interval = 30000;
static AppTimer *s_update_timer;

// Registry of all created weather layers
typedef struct {
    DottedTextLayer *dotted_text_layer;
} WeatherLayerInstance;

static WeatherLayerInstance s_weather_layers[MAX_WEATHER_LAYERS];
static int s_weather_layer_count = 0;
static ClaySettings *s_settings;

WeatherData *weather_get_data() {
    return &weatherData;
}

static void restore_saved_weather_data() {
    memset(&weatherData, 0, sizeof(weatherData));

    if (!persist_exists(WEATHER_DATA_KEY)) {
        return;
    }

    const int bytes = persist_read_data(WEATHER_DATA_KEY, &weatherData, sizeof(weatherData));
    if (bytes != sizeof(weatherData)) {
        memset(&weatherData, 0, sizeof(weatherData));
    }
}

static void save_current_weather_data() {
    // save WeatherData struct to persistent storage
    persist_write_data(WEATHER_DATA_KEY, &weatherData, sizeof(weatherData));
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

static void on_scheduled_update_triggered(void *data) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "scheduled weather update triggered!");

    if (s_update_timer) {
        // cancel weather update timer
        app_timer_cancel(s_update_timer);
    }

    // send AppMessage to trigger weather update via JS
    request_weather_update();

    //Register next execution
    s_update_timer = app_timer_register(s_weather_update_interval, on_scheduled_update_triggered, NULL);
}

// Update all weather layer instances
static void update_all_weather_layers() {
    // persist current data for fast access when opening the watchface
    save_current_weather_data();

    APP_LOG(
        APP_LOG_LEVEL_DEBUG,
        "weather rain forecast: next_1h=%d.%dmm pop=%d%%",
        weatherData.RainNextHourMmX10 / 10,
        weatherData.RainNextHourMmX10 % 10,
        weatherData.RainPopPercent
    );

    // Write the current temperature into a buffer
    snprintf(s_buffer, sizeof(s_buffer), "%d|%d", weatherData.MaxTemperature, weatherData.MinTemperature);

    for (int i = 0; i < s_weather_layer_count; i++) {
        // update text layer
        dotted_text_layer_set_text(s_weather_layers[i].dotted_text_layer, s_buffer);
    }

    update_temperature_forecast();
    update_rain_forecast();
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
