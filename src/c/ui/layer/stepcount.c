#include <pebble.h>

#include "stepcount.h"

#include "../../settings/clay_settings.h"
#include "../../system/health_listener.h"
#include "../../ui/theme.h"
#include "dotted_text_layer.h"
#include "../../ui/layer_factory.h"
#include "../ui_state.h"

static void update_stepcount_for_layer(DottedTextLayer *stepcount_layer) {
    // Write the current step count into a buffer
    static char s_buffer[16];
    snprintf(s_buffer, sizeof(s_buffer), "%d", (int) s_step_count);

    // Display this value on the DottedTextLayer
    dotted_text_layer_set_text(stepcount_layer, s_buffer);
}

// Backward compatible wrapper (called by health listener)
void update_stepcount() {
    for (int i = 0; i < ui_state_get_row_count(); i++) {
        if (ui_state_get_widget_id(i) == WIDGET_STEPCOUNT) {
            update_stepcount_for_layer(ui_state_get_layer(i));
        }
    }
}

// creates the stepcount layer
Layer *create_stepcount_layer(LayerBuilder builder) {
    DottedTextLayer *stepcount_layer = layer_factory_create_dotted_text_layer(
        builder,
        theme_get_theme()->StepcountTextColor,
        HORIZONTAL_ALIGN_RIGHT,
        VERTICAL_ALIGN_TOP,
        NULL
    );
    if (clay_get_settings()->DotAutoScale) {
        dotted_text_layer_set_auto_scale(stepcount_layer, true);
    } else {
        dotted_text_layer_set_scale_factor(stepcount_layer, clay_get_settings()->DotScaleFactor);
    }

    s_step_count = health_service_sum_today(HealthMetricStepCount);
    update_stepcount_for_layer(stepcount_layer);

    return stepcount_layer;
}

// destroys the stepcount layer
void destroy_stepcount_layer(Layer *layer) {
    dotted_text_layer_destroy(layer);
}