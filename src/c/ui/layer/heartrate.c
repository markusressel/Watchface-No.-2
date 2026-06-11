#include <pebble.h>

#include "heartrate.h"

#include "../../settings/clay_settings.h"
#include "../../system/health_listener.h"
#include "../../ui/theme.h"
#include "dotted_text_layer.h"
#include "../../ui/layer_factory.h"
#include "../ui_state.h"

static void update_heartrate_for_layer(DottedTextLayer *heartrate_layer) {
    static char s_buffer[16];

    if (s_heartrate_bpm <= 0) {
        snprintf(s_buffer, sizeof(s_buffer), "--");
    } else {
        snprintf(s_buffer, sizeof(s_buffer), "%d", s_heartrate_bpm);
    }

    dotted_text_layer_set_text(heartrate_layer, s_buffer);
}

void update_heartrate() {
    for (int i = 0; i < ui_state_get_row_count(); i++) {
        if (ui_state_get_widget_id(i) == WIDGET_HEARTRATE) {
            update_heartrate_for_layer((DottedTextLayer *) ui_state_get_layer(i));
        }
    }
}

Layer *create_heartrate_layer(LayerBuilder builder) {
    DottedTextLayer *heartrate_layer = layer_factory_create_dotted_text_layer(
        builder,
        theme_get_theme()->HeartrateTextColor,
        HORIZONTAL_ALIGN_RIGHT,
        VERTICAL_ALIGN_TOP,
        NULL
    );

    if (clay_get_settings()->DotAutoScale) {
        dotted_text_layer_set_auto_scale(heartrate_layer, true);
    } else {
        dotted_text_layer_set_scale_factor(heartrate_layer, clay_get_settings()->DotScaleFactor);
    }

    s_heartrate_bpm = health_service_peek_current_value(HealthMetricHeartRateBPM);
    update_heartrate_for_layer(heartrate_layer);

    return (Layer *) heartrate_layer;
}

void destroy_heartrate_layer(Layer *layer) {
    dotted_text_layer_destroy((DottedTextLayer *) layer);
}