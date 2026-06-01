#include "time_layer.h"
#include "../theme.h"
#include "../clay_settings.h"
#include "../layer_factory.h"

#define MAX_TIME_LAYERS 5

typedef struct {
    DottedTextLayer *dotted_text_layer;
    char time_format[16];
} TimeLayerInstance;

static TimeLayerInstance s_time_layers[MAX_TIME_LAYERS];
static int s_time_layer_count = 0;

static ClaySettings *s_settings;

static void update_all_time_layers(void) {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    for (int i = 0; i < s_time_layer_count; i++) {
        static char s_buffer[16];
        strftime(
            s_buffer,
            sizeof(s_buffer),
            s_time_layers[i].time_format,
            tick_time
        );

        dotted_text_layer_set_text(s_time_layers[i].dotted_text_layer, s_buffer);
    }
}

void update_time_layer() {
    update_all_time_layers();
}

void update_time_layer_layer(Layer *layer) {
    update_all_time_layers();
}

Layer *create_time_layer_layer(LayerBuilder builder) {
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

    instance->dotted_text_layer = layer_factory_create_dotted_text_layer(
        builder,
        theme_get_theme()->TimeTextColor,
        true,
        "00:00"
    );
    dotted_text_layer_set_solid_blocks(instance->dotted_text_layer, true);

    if (s_settings->DotAutoScale) {
        dotted_text_layer_set_auto_scale(instance->dotted_text_layer, true);
    } else {
        dotted_text_layer_set_scale_factor(instance->dotted_text_layer, s_settings->DotScaleFactor);
    }

    s_time_layer_count++;
    update_all_time_layers();

    return instance->dotted_text_layer;
}

void destroy_time_layer_layer(Layer *layer) {
    DottedTextLayer *dotted_text_layer_to_destroy = layer;

    for (int i = 0; i < s_time_layer_count; i++) {
        if (s_time_layers[i].dotted_text_layer == dotted_text_layer_to_destroy) {
            for (int j = i; j < s_time_layer_count - 1; j++) {
                s_time_layers[j] = s_time_layers[j + 1];
            }
            s_time_layer_count--;
            break;
        }
    }

    dotted_text_layer_destroy(dotted_text_layer_to_destroy);
}
