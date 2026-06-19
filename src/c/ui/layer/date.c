#include <pebble.h>

#include "date.h"

#include "../../settings/clay_settings.h"
#include "../../ui/theme.h"
#include "dotted_text_layer.h"
#include "../../ui/layer_factory.h"
#include "../ui_state.h"

typedef struct DateLayerData {
    DottedTextLayer *weekday_layer;
    DottedTextLayer *date_digits_layer;
} DateLayerData;

// get the uppercase version of the char
static char upper(char c) {
    if (c >= 'a' && c <= 'z')
        return (c = c + 'A' - 'a');
    else
        return c;
}

static void date_layer_update_proc(Layer *layer, GContext *ctx) {
    DateLayerData *data = layer_get_data(layer);
    GRect bounds = layer_get_bounds(layer);

    int weekday_width = 0;
    int digits_width = 0;
    int total_width = 0;
    const int spacing = 4; // spacing in pixels between weekday and date digits

    if (data->weekday_layer) {
        weekday_width = dotted_text_layer_get_content_width(data->weekday_layer);
        total_width += weekday_width;
    }
    if (data->date_digits_layer) {
        digits_width = dotted_text_layer_get_content_width(data->date_digits_layer);
        total_width += digits_width;
    }

    if (weekday_width > 0 && digits_width > 0) {
        total_width += spacing;
    }

    int current_x = bounds.size.w - total_width;

    if (data->weekday_layer) {
        if (weekday_width > 0) {
            layer_set_frame((Layer *) data->weekday_layer, GRect(current_x, 0, weekday_width, bounds.size.h));
            current_x += weekday_width + spacing;
        } else {
            layer_set_frame((Layer *) data->weekday_layer, GRect(0, 0, 0, 0));
        }
    }

    if (data->date_digits_layer) {
        if (digits_width > 0) {
            layer_set_frame((Layer *) data->date_digits_layer, GRect(current_x, 0, digits_width, bounds.size.h));
        } else {
            layer_set_frame((Layer *) data->date_digits_layer, GRect(0, 0, 0, 0));
        }
    }
}

static void update_date_for_layer(Layer *container_layer) {
    DateLayerData *data = layer_get_data(container_layer);
    if (data == NULL) {
        return;
    }

    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    ClaySettings *settings = clay_get_settings();
    Theme *theme = theme_get_theme();

    // 1. Weekday Abbreviation
    if (data->weekday_layer) {
        if (settings->ShowWeekdayAbbreviation) {
            char weekday_buffer[16];
            strftime(weekday_buffer, sizeof(weekday_buffer), "%a", tick_time);

            // Remove the third character of weekday abbreviation
            int idxToDel = 2;
            if (strlen(weekday_buffer) > (size_t) idxToDel) {
                memmove(&weekday_buffer[idxToDel], &weekday_buffer[idxToDel + 1], strlen(weekday_buffer) - idxToDel);
            }

            // Convert to uppercase (if enabled)
            if (settings->WeekdayAbbreviationUppercase) {
                for (int j = 0; j < 2; j++) {
                    weekday_buffer[j] = upper(weekday_buffer[j]);
                }
            }
            dotted_text_layer_set_text(data->weekday_layer, weekday_buffer);
            dotted_text_layer_set_color(data->weekday_layer, theme->WeekdayTextColor);
        } else {
            dotted_text_layer_set_text(data->weekday_layer, NULL);
        }
    }

    // 2. Date Digits
    if (data->date_digits_layer) {
        char digits_buffer[16];
        char date_format[16] = "%d.%m";
        if (settings->ShowYear) {
            strcat(date_format, ".%y");
        }
        strftime(digits_buffer, sizeof(digits_buffer), date_format, tick_time);

        if (!settings->DateZeroPadding) {
            char *p = digits_buffer;

            // Now p points to the start of the day digits.
            if (*p == '0') {
                memmove(p, p + 1, strlen(p + 1) + 1);
            }

            // Find the first '.' separator.
            while (*p && *p != '.') {
                p++;
            }

            if (*p == '.') {
                p++; // Point to the start of the month digits
                if (*p == '0') {
                    memmove(p, p + 1, strlen(p + 1) + 1);
                }
            }
        }
        dotted_text_layer_set_text(data->date_digits_layer, digits_buffer);
        dotted_text_layer_set_color(data->date_digits_layer, theme->DateTextColor);
    }

    layer_mark_dirty(container_layer);
}

// Backward compatible wrapper (called by tick listener)
void update_date() {
    for (int i = 0; i < ui_state_get_row_count(); i++) {
        if (ui_state_get_widget_id(i) == WIDGET_DATE) {
            update_date_for_layer(ui_state_get_layer(i));
        }
    }
}

// creates the date layer
Layer *create_date_layer(LayerBuilder builder) {
    Layer *container = layer_factory_create_custom_layer_with_data(
        builder,
        date_layer_update_proc,
        sizeof(DateLayerData)
    );
    DateLayerData *data = layer_get_data(container);

    ClaySettings *settings = clay_get_settings();
    Theme *theme = theme_get_theme();
    LayerBuilder child_builder = layer_builder_from_rect(container, GRect(0, 0, builder.bounds.size.w, builder.bounds.size.h));

    data->weekday_layer = layer_factory_create_dotted_text_layer(
        child_builder,
        theme->WeekdayTextColor,
        HORIZONTAL_ALIGN_LEFT,
        VERTICAL_ALIGN_TOP,
        NULL
    );
    data->date_digits_layer = layer_factory_create_dotted_text_layer(
        child_builder,
        theme->DateTextColor,
        HORIZONTAL_ALIGN_LEFT,
        VERTICAL_ALIGN_TOP,
        NULL
    );

    if (settings->DotAutoScale) {
        dotted_text_layer_set_auto_scale(data->weekday_layer, true);
        dotted_text_layer_set_auto_scale(data->date_digits_layer, true);
    } else {
        dotted_text_layer_set_scale_factor(data->weekday_layer, settings->DotScaleFactor);
        dotted_text_layer_set_scale_factor(data->date_digits_layer, settings->DotScaleFactor);
    }

    update_date_for_layer(container);

    return container;
}

// destroys the date layer
void destroy_date_layer(Layer *layer) {
    DateLayerData *data = layer_get_data(layer);
    if (data->weekday_layer) {
        dotted_text_layer_destroy(data->weekday_layer);
    }
    if (data->date_digits_layer) {
        dotted_text_layer_destroy(data->date_digits_layer);
    }
    layer_destroy(layer);
}
