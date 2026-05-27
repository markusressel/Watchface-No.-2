#include <pebble.h>
#include "date.h"
#include "clay_settings.h"
#include "theme.h"
#include "dotted_text_layer.h"
#include "layer_factory.h"

static ClaySettings *s_settings;

// Date DottedTextLayer
static DottedTextLayer *s_dotted_text_layer;

static char s_date_format[32];

// get the uppercase version if the char
static char upper(char c) {
    if (c >= 'a' && c <= 'z')
        return (c = c + 'A' - 'a');
    else
        return c;
}

void update_date() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the current day, month and year into a buffer
    static char s_buffer[16];
    strftime(s_buffer,
             sizeof(s_buffer),
             s_date_format,
             tick_time);

    // Remove the third character of weekday abbreviation
    if (s_settings->ShowWeekdayAbbreviation) {
        int idxToDel = 2;
        memmove(&s_buffer[idxToDel], &s_buffer[idxToDel + 1], strlen(s_buffer) - idxToDel);

        // Convert to uppercase (if enabled)
        if (s_settings->WeekdayAbbreviationUppercase) {
            for (int i = 0; i < 2; i++) {
                s_buffer[i] = upper(s_buffer[i]);
            }
        }
    }

    // Display this date on the DottedTextLayer
    dotted_text_layer_set_text(s_dotted_text_layer, s_buffer);
}

// creates the date layer
void create_date_layer(Layer *window_layer) {
    s_settings = clay_get_settings();

    strcpy(s_date_format, "");
    if (s_settings->ShowWeekdayAbbreviation) {
        strcat(s_date_format, "%a ");
    }

    strcat(s_date_format, "%d.%m");

    if (s_settings->ShowYear) {
        strcat(s_date_format, ".%y");
    }

    LayerBuilder builder = layer_builder(window_layer, (LayerLayout){
                                             .x = 0,
                                             .y = 27 + 5 + 5,
                                             .width_margin = 3,
                                             .height = 27,
                                         });
    s_dotted_text_layer = layer_factory_create_dotted_text_layer(
        builder,
        theme_get_theme()->DateTextColor,
        true,
        NULL
    );

    update_date();
}

// destroys the date layer
void destroy_date_layer() {
    dotted_text_layer_destroy(s_dotted_text_layer);
}
