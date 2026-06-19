#include <pebble.h>

#include "date.h"

#include "../../settings/clay_settings.h"
#include "../../ui/theme.h"
#include "dotted_text_layer.h"
#include "../../ui/layer_factory.h"
#include "../ui_state.h"

typedef enum {
    DATE_PART_WEEKDAY = 0,
    DATE_PART_DAY = 1,
    DATE_PART_SEP1 = 2,
    DATE_PART_MONTH = 3,
    DATE_PART_SEP2 = 4,
    DATE_PART_YEAR = 5,
    DATE_PART_COUNT = 6
} DatePart;

typedef struct DateLayerData {
    DottedTextLayer *parts[DATE_PART_COUNT];
} DateLayerData;

#define UPPER(c) (((c) >= 'a' && (c) <= 'z') ? ((c) - 32) : (c))

static void date_layer_update_proc(Layer *layer, GContext *ctx) {
    DateLayerData *data = layer_get_data(layer);
    GRect bounds = layer_get_bounds(layer);
    ClaySettings *settings = clay_get_settings();

    // 1. Calculate metrics and dynamic spacing
    float scale_factor = 1.0f;
    if (settings->DotAutoScale) {
        int base_height = (5 * settings->DotHeight) + (4 * settings->DotVerticalGap);
        if (base_height > 0) {
            scale_factor = (float) bounds.size.h / (float) base_height;
        }
    } else {
        scale_factor = settings->DotScaleFactor;
    }
    if (scale_factor <= 0.0f) {
        scale_factor = 1.0f;
    }

    float dot_width = (float) settings->DotWidth * scale_factor;
    int char_spacing = (int) (2.0f * dot_width + 0.5f);
    if (char_spacing < 1) {
        char_spacing = 1;
    }
    int weekday_spacing = (int) (3.0f * dot_width + 0.5f);
    if (weekday_spacing < 1) {
        weekday_spacing = 1;
    }

    // 2. Measure widths
    int widths[DATE_PART_COUNT] = {0};
    int total_width = 0;

    for (int i = 0; i < DATE_PART_COUNT; i++) {
        if (data->parts[i]) {
            widths[i] = dotted_text_layer_get_content_width(data->parts[i]);
            if (widths[i] > 0) {
                total_width += widths[i];
            }
        }
    }

    // Add spacing between elements
    if (widths[DATE_PART_WEEKDAY] > 0 && (widths[DATE_PART_DAY] > 0 || widths[DATE_PART_MONTH] > 0)) {
        total_width += weekday_spacing;
    }
    if (widths[DATE_PART_DAY] > 0 && widths[DATE_PART_SEP1] > 0) {
        total_width += char_spacing;
    }
    if (widths[DATE_PART_SEP1] > 0 && widths[DATE_PART_MONTH] > 0) {
        total_width += char_spacing;
    }
    if (widths[DATE_PART_MONTH] > 0 && widths[DATE_PART_SEP2] > 0) {
        total_width += char_spacing;
    }
    if (widths[DATE_PART_SEP2] > 0 && widths[DATE_PART_YEAR] > 0) {
        total_width += char_spacing;
    }

    int current_x = bounds.size.w - total_width;

    // Position layers
    for (int i = 0; i < DATE_PART_COUNT; i++) {
        if (data->parts[i]) {
            if (widths[i] > 0) {
                layer_set_frame((Layer *) data->parts[i], GRect(current_x, 0, widths[i], bounds.size.h));
                int next_spacing = char_spacing;
                if (i == DATE_PART_WEEKDAY) {
                    next_spacing = weekday_spacing;
                }
                current_x += widths[i] + next_spacing;
            } else {
                layer_set_frame((Layer *) data->parts[i], GRect(0, 0, 0, 0));
            }
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

    // 1. Set text on active parts
    if (data->parts[DATE_PART_WEEKDAY]) {
        if (settings->ShowWeekdayAbbreviation) {
            char weekday_buffer[8];
            strftime(weekday_buffer, sizeof(weekday_buffer), "%a", tick_time);
            weekday_buffer[2] = '\0';
            if (settings->WeekdayAbbreviationUppercase) {
                weekday_buffer[0] = UPPER(weekday_buffer[0]);
                weekday_buffer[1] = UPPER(weekday_buffer[1]);
            }
            dotted_text_layer_set_text(data->parts[DATE_PART_WEEKDAY], weekday_buffer);
        } else {
            dotted_text_layer_set_text(data->parts[DATE_PART_WEEKDAY], NULL);
        }
    }

    if (data->parts[DATE_PART_DAY]) {
        char day_buffer[8];
        strftime(day_buffer, sizeof(day_buffer), "%d", tick_time);
        if (!settings->DateZeroPadding && day_buffer[0] == '0') {
            day_buffer[0] = day_buffer[1];
            day_buffer[1] = '\0';
        }
        dotted_text_layer_set_text(data->parts[DATE_PART_DAY], day_buffer);
    }

    if (data->parts[DATE_PART_MONTH]) {
        char month_buffer[8];
        strftime(month_buffer, sizeof(month_buffer), "%m", tick_time);
        if (!settings->DateZeroPadding && month_buffer[0] == '0') {
            month_buffer[0] = month_buffer[1];
            month_buffer[1] = '\0';
        }
        dotted_text_layer_set_text(data->parts[DATE_PART_MONTH], month_buffer);
    }

    if (data->parts[DATE_PART_SEP2]) {
        dotted_text_layer_set_text(data->parts[DATE_PART_SEP2], settings->ShowYear ? "." : NULL);
    }

    if (data->parts[DATE_PART_YEAR]) {
        if (settings->ShowYear) {
            char year_buffer[8];
            strftime(year_buffer, sizeof(year_buffer), "%y", tick_time);
            dotted_text_layer_set_text(data->parts[DATE_PART_YEAR], year_buffer);
        } else {
            dotted_text_layer_set_text(data->parts[DATE_PART_YEAR], NULL);
        }
    }

    // 2. Apply colors in a loop
    GColor colors[DATE_PART_COUNT] = {
        theme->WeekdayTextColor,
        theme->DateTextColor,
        theme->DateSeparatorColor,
        theme->DateTextColor,
        theme->DateSeparatorColor,
        theme->DateTextColor
    };
    for (int i = 0; i < DATE_PART_COUNT; i++) {
        if (data->parts[i]) {
            dotted_text_layer_set_color(data->parts[i], colors[i]);
        }
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
    LayerBuilder child_builder = layer_builder_from_rect(container, GRect(0, 0, builder.bounds.size.w, builder.bounds.size.h));

    for (int i = 0; i < DATE_PART_COUNT; i++) {
        data->parts[i] = layer_factory_create_dotted_text_layer(
            child_builder,
            GColorClear, // will be set by update_date_for_layer
            HORIZONTAL_ALIGN_LEFT,
            VERTICAL_ALIGN_TOP,
            (i == DATE_PART_SEP1) ? "." : NULL
        );
        if (settings->DotAutoScale) {
            dotted_text_layer_set_auto_scale(data->parts[i], true);
        } else {
            dotted_text_layer_set_scale_factor(data->parts[i], settings->DotScaleFactor);
        }
    }

    update_date_for_layer(container);

    return container;
}

// destroys the date layer
void destroy_date_layer(Layer *layer) {
    DateLayerData *data = layer_get_data(layer);
    for (int i = 0; i < DATE_PART_COUNT; i++) {
        if (data->parts[i]) {
            dotted_text_layer_destroy(data->parts[i]);
        }
    }
    layer_destroy(layer);
}
