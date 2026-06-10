#include <pebble.h>
#include "date.h"
#include "../settings/clay_settings.h"
#include "../ui/theme.h"
#include "dotted_text_layer.h"
#include "../ui/layer_factory.h"

#define MAX_DATE_LAYERS 5

// Registry of all created date layers
typedef struct {
    DottedTextLayer *dotted_text_layer;
    char date_format[32];
} DateLayerInstance;

static DateLayerInstance s_date_layers[MAX_DATE_LAYERS];
static int s_date_layer_count = 0;

// get the uppercase version if the char
static char upper(char c) {
    if (c >= 'a' && c <= 'z')
        return (c = c + 'A' - 'a');
    else
        return c;
}

// Helper to update all date layer instances
static void update_all_date_layers() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    for (int i = 0; i < s_date_layer_count; i++) {
        // Write the current day, month and year into a buffer
        static char s_buffer[16];
        strftime(
            s_buffer,
            sizeof(s_buffer),
            s_date_layers[i].date_format,
            tick_time
        );

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
        dotted_text_layer_set_text(s_date_layers[i].dotted_text_layer, s_buffer);
    }
}

// Backward compatible wrapper (called by tick listener)
void update_date() {
    update_all_date_layers();
}

// creates the date layer
Layer *create_date_layer(LayerBuilder builder) {
    if (s_date_layer_count >= MAX_DATE_LAYERS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Max date layers exceeded!");
        return NULL;
    }

    DateLayerInstance *instance = &s_date_layers[s_date_layer_count];

    strcpy(instance->date_format, "");
    if (clay_get_settings()->ShowWeekdayAbbreviation) {
        strcat(instance->date_format, "%a ");
    }

    strcat(instance->date_format, "%d.%m");

    if (clay_get_settings()->ShowYear) {
        strcat(instance->date_format, ".%y");
    }

    instance->dotted_text_layer = layer_factory_create_dotted_text_layer(
        builder,
        theme_get_theme()->DateTextColor,
        HORIZONTAL_ALIGN_RIGHT,
        VERTICAL_ALIGN_TOP,
        NULL
    );
    if (clay_get_settings()->DotAutoScale) {
        dotted_text_layer_set_auto_scale(instance->dotted_text_layer, true);
    } else {
        dotted_text_layer_set_scale_factor(instance->dotted_text_layer, clay_get_settings()->DotScaleFactor);
    }

    s_date_layer_count++;

    update_all_date_layers();

    return instance->dotted_text_layer;
}

// destroys the date layer
void destroy_date_layer(Layer *layer) {
    DottedTextLayer *dotted_text_layer_to_destroy = layer;

    // Find and remove from registry
    for (int i = 0; i < s_date_layer_count; i++) {
        if (s_date_layers[i].dotted_text_layer == dotted_text_layer_to_destroy) {
            // Remove from array by shifting remaining elements
            for (int j = i; j < s_date_layer_count - 1; j++) {
                s_date_layers[j] = s_date_layers[j + 1];
            }
            s_date_layer_count--;
            break;
        }
    }

    dotted_text_layer_destroy(dotted_text_layer_to_destroy);
}