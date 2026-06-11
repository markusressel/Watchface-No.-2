#include <pebble.h>

#include "date.h"

#include "../../settings/clay_settings.h"
#include "../../ui/theme.h"
#include "dotted_text_layer.h"
#include "../../ui/layer_factory.h"
#include "../ui_state.h"

// get the uppercase version if the char
static char upper(char c) {
    if (c >= 'a' && c <= 'z')
        return (c = c + 'A' - 'a');
    else
        return c;
}

static void update_date_for_layer(DottedTextLayer *date_layer) {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the current day, month and year into a buffer
    static char s_buffer[16];
    char date_format[32];
    strcpy(date_format, "");
    if (clay_get_settings()->ShowWeekdayAbbreviation) {
        strcat(date_format, "%a ");
    }

    strcat(date_format, "%d.%m");

    if (clay_get_settings()->ShowYear) {
        strcat(date_format, ".%y");
    }

    strftime(s_buffer, sizeof(s_buffer), date_format, tick_time);

    // Remove the third character of weekday abbreviation
    if (clay_get_settings()->ShowWeekdayAbbreviation) {
        int idxToDel = 2;
        memmove(&s_buffer[idxToDel], &s_buffer[idxToDel + 1], strlen(s_buffer) - idxToDel);

        // Convert to uppercase (if enabled)
        if (clay_get_settings()->WeekdayAbbreviationUppercase) {
            for (int j = 0; j < 2; j++) {
                s_buffer[j] = upper(s_buffer[j]);
            }
        }
    }

    // Display this date on the DottedTextLayer
    dotted_text_layer_set_text(date_layer, s_buffer);
}

// Backward compatible wrapper (called by tick listener)
void update_date() {
    for (int i = 0; i < ui_state_get_row_count(); i++) {
        if (ui_state_get_widget_id(i) == WIDGET_DATE) {
            update_date_for_layer((DottedTextLayer *) ui_state_get_layer(i));
        }
    }
}

// creates the date layer
Layer *create_date_layer(LayerBuilder builder) {
    DottedTextLayer *date_layer = layer_factory_create_dotted_text_layer(
        builder,
        theme_get_theme()->DateTextColor,
        HORIZONTAL_ALIGN_RIGHT,
        VERTICAL_ALIGN_TOP,
        NULL
    );
    if (clay_get_settings()->DotAutoScale) {
        dotted_text_layer_set_auto_scale(date_layer, true);
    } else {
        dotted_text_layer_set_scale_factor(date_layer, clay_get_settings()->DotScaleFactor);
    }

    update_date_for_layer(date_layer);

    return (Layer *) date_layer;
}

// destroys the date layer
void destroy_date_layer(Layer *layer) {
    dotted_text_layer_destroy((DottedTextLayer *) layer);
}