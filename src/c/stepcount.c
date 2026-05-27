#include <pebble.h>
#include "stepcount.h"
#include "health_listener.h"
#include "theme.h"
#include "dotted_text_layer.h"
#include "layer_factory.h"

// Stepcount DottedTextLayer
static DottedTextLayer *s_dotted_text_layer;

void update_stepcount() {
    // Write the current day, month and year into a buffer
    static char s_buffer[16];
    snprintf(s_buffer, sizeof(s_buffer), "%d", (int) s_step_count);

    // Display this value on the DottedTextLayer
    dotted_text_layer_set_text(s_dotted_text_layer, s_buffer);
}

// creates the stepcount layer
void create_stepcount_layer(LayerBuilder builder) {
    s_dotted_text_layer = layer_factory_create_dotted_text_layer(
        builder,
        theme_get_theme()->StepcountTextColor,
        true,
        NULL
    );

    s_step_count = health_service_sum_today(HealthMetricStepCount);
    update_stepcount();
}

// destroys the stepcount layer
void destroy_stepcount_layer() {
    dotted_text_layer_destroy(s_dotted_text_layer);
}
