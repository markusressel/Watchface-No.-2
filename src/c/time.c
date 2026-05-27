#include "time.h"
#include "theme.h"
#include "clay_settings.h"
#include "layer_factory.h"

#define MAX_TIME_LAYERS 5

// Registry of all created time layers (indexed by row)
typedef struct {
    TextLayer *text_layer;
    char time_format[16];
} TimeLayerInstance;

static TimeLayerInstance s_time_layers[MAX_TIME_LAYERS];
static int s_time_layer_count = 0;

static ClaySettings *s_settings;

// Helper to find or update time layer
static void update_all_time_layers() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    for (int i = 0; i < s_time_layer_count; i++) {
        // Write the current hours and minutes into a buffer
        static char s_buffer[16];
        strftime(
            s_buffer,
            sizeof(s_buffer),
            s_time_layers[i].time_format,
            tick_time
        );

        // Display this time on the TextLayer
        text_layer_set_text(s_time_layers[i].text_layer, s_buffer);
    }
}

// Backward compatible wrapper (called by tick listener)
void update_time() {
    update_all_time_layers();
}

// Method to update a specific time layer instance
void update_time_layer(Layer *layer) {
    update_all_time_layers();
}

// creates the time layer
Layer *create_time_layer(LayerBuilder builder) {
    s_settings = clay_get_settings();

    if (s_time_layer_count >= MAX_TIME_LAYERS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Max time layers exceeded!");
        return NULL;
    }

    TimeLayerInstance *instance = &s_time_layers[s_time_layer_count];

    strcpy(instance->time_format, clock_is_24h_style() ? "%H:%M" : "%I:%M");
    if (s_settings->ShowSeconds) {
        strcat(instance->time_format, ":%S");
    }

    instance->text_layer = layer_factory_create_text_layer(builder, (TextLayerStyle){
        .background_color = GColorClear,
        .text_color = theme_get_theme()->TimeTextColor,
        .font = theme_get_theme()->TimeFont,
        .alignment = GTextAlignmentRight,
    });

    s_time_layer_count++;

    // update time value before rendering so it is shown right from the beginning
    update_all_time_layers();

    return (Layer *)instance->text_layer;
}

// destroys the time layer
void destroy_time_layer(Layer *layer) {
    TextLayer *text_layer_to_destroy = (TextLayer *)layer;

    // Find and remove from registry
    for (int i = 0; i < s_time_layer_count; i++) {
        if (s_time_layers[i].text_layer == text_layer_to_destroy) {
            // Remove from array by shifting remaining elements
            for (int j = i; j < s_time_layer_count - 1; j++) {
                s_time_layers[j] = s_time_layers[j + 1];
            }
            s_time_layer_count--;
            break;
        }
    }

    text_layer_destroy(text_layer_to_destroy);
}
