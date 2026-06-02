#include <pebble.h>
#include "heartrate.h"
#include "../clay_settings.h"
#include "../health_listener.h"
#include "../theme.h"
#include "../dotted_text_layer.h"
#include "../layer_factory.h"

#define MAX_HEARTRATE_LAYERS 7

typedef struct {
    DottedTextLayer *dotted_text_layer;
} HeartrateLayerInstance;

static HeartrateLayerInstance s_heartrate_layers[MAX_HEARTRATE_LAYERS];
static int s_heartrate_layer_count = 0;
static ClaySettings *s_settings;

static void update_all_heartrate_layers() {
    for (int i = 0; i < s_heartrate_layer_count; i++) {
        static char s_buffer[16];

        if (s_heartrate_bpm <= 0) {
            snprintf(s_buffer, sizeof(s_buffer), "--");
        } else {
            snprintf(s_buffer, sizeof(s_buffer), "%d", s_heartrate_bpm);
        }

        dotted_text_layer_set_text(s_heartrate_layers[i].dotted_text_layer, s_buffer);
    }
}

void update_heartrate() {
    update_all_heartrate_layers();
}

Layer *create_heartrate_layer(LayerBuilder builder) {
    s_settings = clay_get_settings();

    if (s_heartrate_layer_count >= MAX_HEARTRATE_LAYERS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Max heartrate layers exceeded!");
        return NULL;
    }

    HeartrateLayerInstance *instance = &s_heartrate_layers[s_heartrate_layer_count];

    instance->dotted_text_layer = layer_factory_create_dotted_text_layer(
        builder,
        theme_get_theme()->HeartrateTextColor,
        HORIZONTAL_ALIGN_RIGHT,
        VERTICAL_ALIGN_TOP,
        NULL
    );

    if (s_settings->DotAutoScale) {
        dotted_text_layer_set_auto_scale(instance->dotted_text_layer, true);
    } else {
        dotted_text_layer_set_scale_factor(instance->dotted_text_layer, s_settings->DotScaleFactor);
    }

    s_heartrate_layer_count++;

    s_heartrate_bpm = health_service_peek_current_value(HealthMetricHeartRateBPM);
    update_all_heartrate_layers();

    return instance->dotted_text_layer;
}

void destroy_heartrate_layer(Layer *layer) {
    DottedTextLayer *dotted_text_layer_to_destroy = layer;

    for (int i = 0; i < s_heartrate_layer_count; i++) {
        if (s_heartrate_layers[i].dotted_text_layer == dotted_text_layer_to_destroy) {
            for (int j = i; j < s_heartrate_layer_count - 1; j++) {
                s_heartrate_layers[j] = s_heartrate_layers[j + 1];
            }
            s_heartrate_layer_count--;
            break;
        }
    }

    dotted_text_layer_destroy(dotted_text_layer_to_destroy);
}

