#include <pebble.h>
#include "stepcount.h"
#include "../clay_settings.h"
#include "../health_listener.h"
#include "../theme.h"
#include "../dotted_text_layer.h"
#include "../layer_factory.h"

#define MAX_STEPCOUNT_LAYERS 5

// Registry of all created stepcount layers
typedef struct {
    DottedTextLayer *dotted_text_layer;
} StepcountLayerInstance;

static StepcountLayerInstance s_stepcount_layers[MAX_STEPCOUNT_LAYERS];
static int s_stepcount_layer_count = 0;
static ClaySettings *s_settings;

// Helper to update all stepcount layer instances
static void update_all_stepcount_layers() {
    for (int i = 0; i < s_stepcount_layer_count; i++) {
        // Write the current day, month and year into a buffer
        static char s_buffer[16];
        snprintf(s_buffer, sizeof(s_buffer), "%d", (int) s_step_count);

        // Display this value on the DottedTextLayer
        dotted_text_layer_set_text(s_stepcount_layers[i].dotted_text_layer, s_buffer);
    }
}

// Backward compatible wrapper (called by health listener)
void update_stepcount() {
    update_all_stepcount_layers();
}

// creates the stepcount layer
Layer *create_stepcount_layer(LayerBuilder builder) {
    s_settings = clay_get_settings();

    if (s_stepcount_layer_count >= MAX_STEPCOUNT_LAYERS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Max stepcount layers exceeded!");
        return NULL;
    }

    StepcountLayerInstance *instance = &s_stepcount_layers[s_stepcount_layer_count];

    instance->dotted_text_layer = layer_factory_create_dotted_text_layer(
        builder,
        theme_get_theme()->StepcountTextColor,
        true,
        NULL
    );
    if (s_settings->DotAutoScale) {
        dotted_text_layer_set_auto_scale(instance->dotted_text_layer, true);
    } else {
        dotted_text_layer_set_scale_factor(instance->dotted_text_layer, s_settings->DotScaleFactor);
    }

    s_stepcount_layer_count++;

    s_step_count = health_service_sum_today(HealthMetricStepCount);
    update_all_stepcount_layers();

    return instance->dotted_text_layer;
}

// destroys the stepcount layer
void destroy_stepcount_layer(Layer *layer) {
    DottedTextLayer *dotted_text_layer_to_destroy = layer;

    // Find and remove from registry
    for (int i = 0; i < s_stepcount_layer_count; i++) {
        if (s_stepcount_layers[i].dotted_text_layer == dotted_text_layer_to_destroy) {
            // Remove from array by shifting remaining elements
            for (int j = i; j < s_stepcount_layer_count - 1; j++) {
                s_stepcount_layers[j] = s_stepcount_layers[j + 1];
            }
            s_stepcount_layer_count--;
            break;
        }
    }

    dotted_text_layer_destroy(dotted_text_layer_to_destroy);
}