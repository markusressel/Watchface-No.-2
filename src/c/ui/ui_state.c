#include "ui_state.h"
#include "layer_factory.h"
#include "layer/widget.h"
#include "layer/time.h"
#include "layer/date.h"
#include "layer/weather.h"
#include "layer/stepcount.h"
#include "layer/heartrate.h"
#include "layer/weather_forecast.h"
#include "layer/battery_bar.h"
#include "layer/debug_layer.h"

#define MAX_ROW_LAYERS WATCH_LAYOUT_MAX_ROWS

typedef struct {
    Layer *layer;
    WidgetId widget_id;
} RowLayer;

static struct {
    Window *window;
    RowLayer row_layers[MAX_ROW_LAYERS];
    int row_count;
} s_ui_state;

void ui_state_init(Window *window) {
    s_ui_state.window = window;
    s_ui_state.row_count = 0;
    for (int i = 0; i < MAX_ROW_LAYERS; i++) {
        s_ui_state.row_layers[i].layer = NULL;
        s_ui_state.row_layers[i].widget_id = WIDGET_COUNT;
    }
}

void ui_state_deinit() {
    ui_state_destroy_layers();
}

void ui_state_create_layers(WatchLayout *layout) {
    if (!s_ui_state.window) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "ui_state_create_layers: window is NULL!");
        return;
    }

    if (s_ui_state.row_count > 0) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "Destroying existing layers before creating new ones");
        ui_state_destroy_layers();
    }

    s_ui_state.row_count = layout->row_count;
    Layer *window_layer = window_get_root_layer(s_ui_state.window);

    for (int i = 0; i < s_ui_state.row_count; i++) {
        LayerBuilder builder = watch_layout_make_builder(layout, window_layer, i);
        WidgetId widget_id = layout->rows[i].widget;
        s_ui_state.row_layers[i].widget_id = widget_id;

        switch (widget_id) {
            case WIDGET_TIME:
                s_ui_state.row_layers[i].layer = create_time_layer(builder);
                break;
            case WIDGET_DATE:
                s_ui_state.row_layers[i].layer = create_date_layer(builder);
                break;
            case WIDGET_WEATHER:
                s_ui_state.row_layers[i].layer = create_weather_layer(builder);
                break;
            case WIDGET_STEPCOUNT:
                s_ui_state.row_layers[i].layer = create_stepcount_layer(builder);
                break;
            case WIDGET_HEARTRATE:
                s_ui_state.row_layers[i].layer = create_heartrate_layer(builder);
                break;
            case WIDGET_WEATHER_FORECAST:
                s_ui_state.row_layers[i].layer = create_weather_forecast_layer(builder);
                break;
            case WIDGET_BATTERY_BAR:
                s_ui_state.row_layers[i].layer = create_battery_bar_layer(builder);
                break;
            default:
                break;
        }
    }
}

void ui_state_destroy_layers() {
    debug_layer_destroy_all_borders();

    for (int i = 0; i < s_ui_state.row_count; i++) {
        if (s_ui_state.row_layers[i].layer != NULL) {
            switch (s_ui_state.row_layers[i].widget_id) {
                case WIDGET_TIME:
                    destroy_time_layer(s_ui_state.row_layers[i].layer);
                    break;
                case WIDGET_DATE:
                    destroy_date_layer(s_ui_state.row_layers[i].layer);
                    break;
                case WIDGET_WEATHER:
                    destroy_weather_layer(s_ui_state.row_layers[i].layer);
                    break;
                case WIDGET_STEPCOUNT:
                    destroy_stepcount_layer(s_ui_state.row_layers[i].layer);
                    break;
                case WIDGET_HEARTRATE:
                    destroy_heartrate_layer(s_ui_state.row_layers[i].layer);
                    break;
                case WIDGET_WEATHER_FORECAST:
                    destroy_weather_forecast_layer(s_ui_state.row_layers[i].layer);
                    break;
                case WIDGET_BATTERY_BAR:
                    destroy_battery_bar_layer(s_ui_state.row_layers[i].layer);
                    break;
                default:
                    break;
            }
            s_ui_state.row_layers[i].layer = NULL;
            s_ui_state.row_layers[i].widget_id = WIDGET_COUNT;
        }
    }
    s_ui_state.row_count = 0;
}

int ui_state_get_row_count(void) {
    return s_ui_state.row_count;
}

Layer *ui_state_get_layer(int index) {
    if (index < 0 || index >= s_ui_state.row_count) {
        return NULL;
    }
    return s_ui_state.row_layers[index].layer;
}

WidgetId ui_state_get_widget_id(int index) {
    if (index < 0 || index >= s_ui_state.row_count) {
        return WIDGET_COUNT;
    }
    return s_ui_state.row_layers[index].widget_id;
}
