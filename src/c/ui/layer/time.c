#include "time.h"

#include "dotted_text_layer.h"
#include "../../ui/theme.h"
#include "../../settings/clay_settings.h"
#include "../../ui/layer_factory.h"
#include "../ui_state.h"

static void update_time_for_layer(DottedTextLayer *time_layer) {
    const time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char s_buffer[16];
    char time_format[16];
    strcpy(time_format, clock_is_24h_style() ? "%H:%M" : "%I:%M");
    if (clay_get_settings()->ShowSeconds) {
        strcat(time_format, ":%S");
    }

    strftime(s_buffer, sizeof(s_buffer), time_format, tick_time);
    dotted_text_layer_set_text(time_layer, s_buffer);
}

void update_time_layer() {
    for (int i = 0; i < ui_state_get_row_count(); i++) {
        if (ui_state_get_widget_id(i) == WIDGET_TIME) {
            update_time_for_layer(ui_state_get_layer(i));
        }
    }
}

Layer *create_time_layer(LayerBuilder builder) {
    DottedTextLayer *time_layer = layer_factory_create_dotted_text_layer(
        builder,
        theme_get_theme()->TimeTextColor,
        HORIZONTAL_ALIGN_RIGHT,
        VERTICAL_ALIGN_CENTER,
        "00:00"
    );
    // For time, force square metrics so each block appears square.
    const int base_dot = 10;
    const int base_gap = 0;
    dotted_text_layer_set_custom_metrics(
        time_layer,
        base_dot,
        base_dot,
        base_gap,
        base_gap
    );
    dotted_text_layer_set_character_offset(
        time_layer,
        8, DOTTED_TEXT_OFFSET_PIXELS
    );

    if (clay_get_settings()->DotAutoScale) {
        dotted_text_layer_set_auto_scale(time_layer, true);
    } else {
        dotted_text_layer_set_scale_factor(time_layer, clay_get_settings()->DotScaleFactor);
    }

    update_time_for_layer(time_layer);

    return time_layer;
}

void destroy_time_layer(Layer *layer) {
    dotted_text_layer_destroy(layer);
}
