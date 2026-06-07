#include "main.h"

#include <pebble.h>
#include "theme.h"
#include "clay_settings.h"
#include "watch_layout.h"
#include "layer/time.h"
#include "tick_listener.h"
#include "layer/date.h"
#include "layer/battery_bar.h"
#include "battery_listener.h"
#include "layer/weather.h"
#include "app_messaging.h"
#include "layer/stepcount.h"
#include "layer/heartrate.h"
#include "layer/weather_forecast.h"
#include "health_listener.h"
#include "layer/widget.h"

// Main Window
static Window *s_main_window;

// Layout is built at runtime from settings (see build_layout_from_settings).
static WatchLayout s_layout;

// Store created layer instances indexed by row for proper cleanup
static Layer *s_row_layers[WATCH_LAYOUT_MAX_ROWS];

static void init_row_layers() {
    for (int i = 0; i < WATCH_LAYOUT_MAX_ROWS; i++) {
        s_row_layers[i] = NULL;
    }
}

static void build_layout_from_settings(ClaySettings *settings) {
    int row_count = settings->LayoutRowCount;
    if (row_count < 5) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Layout row count too low, setting to minimum of 5");
        row_count = 5;
    } else if (row_count > WATCH_LAYOUT_MAX_ROWS) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Layout row count too high, setting to maximum of %d", WATCH_LAYOUT_MAX_ROWS);
        row_count = WATCH_LAYOUT_MAX_ROWS;
    }

    s_layout = (WatchLayout){
        .row_count = row_count,
        .rows = {
            [0] = {.widget = (WidgetId) settings->Row0Widget},
            [1] = {.widget = (WidgetId) settings->Row1Widget},
            [2] = {.widget = (WidgetId) settings->Row2Widget},
            [3] = {.widget = (WidgetId) settings->Row3Widget},
            [4] = {.widget = (WidgetId) settings->Row4Widget},
            [5] = {.widget = (WidgetId) settings->Row5Widget},
            [6] = {.widget = (WidgetId) settings->Row6Widget},
        },
    };
}

// loads components into the main window
static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);

    // Create each layer in row order.  To reorder, edit s_layout.rows above.
    for (int i = 0; i < s_layout.row_count; i++) {
        LayerBuilder builder = watch_layout_make_builder(&s_layout, window_layer, i);
        switch (s_layout.rows[i].widget) {
            case WIDGET_TIME: s_row_layers[i] = create_time_layer(builder);
                break;
            case WIDGET_DATE: s_row_layers[i] = create_date_layer(builder);
                break;
            case WIDGET_WEATHER: s_row_layers[i] = create_weather_layer(builder);
                break;
            case WIDGET_STEPCOUNT: s_row_layers[i] = create_stepcount_layer(builder);
                break;
            case WIDGET_HEARTRATE: s_row_layers[i] = create_heartrate_layer(builder);
                break;
            case WIDGET_WEATHER_FORECAST: s_row_layers[i] = create_temperature_forecast_layer(builder);
                break;
            case WIDGET_BATTERY_BAR: s_row_layers[i] = create_battery_bar_layer(builder);
                break;
            default: break;
        }
    }

    // Register for tick events (time)
    register_tick_listener();
    // Register for battery level updates
    register_battery_listener();
    // Ensure battery level is displayed from the start
    force_battery_update();

    // Register health event listener
    register_health_event_listener();
}

// destroys all components of the main window
static void main_window_unload(Window *window) {
    // unregister listeners
    unregister_tick_listener();
    unregister_battery_listener();
    unregister_health_event_listener();

    // Destroy only the layers that were created for the current layout
    for (int i = 0; i < s_layout.row_count; i++) {
        if (s_row_layers[i] != NULL) {
            switch (s_layout.rows[i].widget) {
                case WIDGET_TIME: destroy_time_layer(s_row_layers[i]);
                    break;
                case WIDGET_DATE: destroy_date_layer(s_row_layers[i]);
                    break;
                case WIDGET_WEATHER: destroy_weather_layer(s_row_layers[i]);
                    break;
                case WIDGET_STEPCOUNT: destroy_stepcount_layer(s_row_layers[i]);
                    break;
                case WIDGET_HEARTRATE: destroy_heartrate_layer(s_row_layers[i]);
                    break;
                case WIDGET_WEATHER_FORECAST: destroy_temperature_forecast_layer(s_row_layers[i]);
                    break;
                case WIDGET_BATTERY_BAR: destroy_battery_bar_layer(s_row_layers[i]);
                    break;
                default: break;
            }
            s_row_layers[i] = NULL;
        }
    }
}

static void apply_theme_from_settings(ClaySettings *settings) {
    enum ThemeEnum theme;
    if (settings->ThemeValue[0] == '\0') {
        strcpy(settings->ThemeValue, THEME_LIGHT);
    }
    if (strcmp(settings->ThemeValue, THEME_LIGHT) == 0) {
        theme = LIGHT;
    } else if (strcmp(settings->ThemeValue, THEME_DARK) == 0) {
        theme = DARK;
    } else {
        theme = CUSTOM;
    }

    if (theme == CUSTOM) {
        Theme custom_theme;
        custom_theme.CurrentThemeEnum = CUSTOM;
        custom_theme.BackgroundColor = settings->BackgroundColor;
        custom_theme.TimeTextColor = settings->TimeTextColor;
        custom_theme.DateTextColor = settings->DateTextColor;
        custom_theme.BatteryOutlineColor = settings->BatteryFrameColor;
        custom_theme.BatteryFillColor = settings->BatteryFillColor;
        custom_theme.WeatherTextColor = settings->WeatherTextColor;
        custom_theme.StepcountTextColor = settings->StepcountTextColor;
        custom_theme.HeartrateTextColor = settings->HeartrateTextColor;
        theme_set_fonts(&custom_theme, settings->ShowSeconds);

        set_custom_theme(&custom_theme);
    } else {
        init_theme(theme, settings->ShowSeconds);
    }

    // Safely update the window background if the window exists
    if (s_main_window) {
        window_set_background_color(s_main_window, theme_get_theme()->BackgroundColor);
    }
}

// Rebuild layout from settings and reload all layers.
// Call this when row layout settings change at runtime.
void main_reload_layout() {
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "main_reload_layout");
    main_window_unload(s_main_window);
    init_row_layers();
    ClaySettings *settings = clay_get_settings();
    apply_theme_from_settings(settings);
    build_layout_from_settings(settings);
    main_window_load(s_main_window);
}

// initializes the watchface
static void init() {
    ClaySettings *settings = clay_load_settings();

    // Initialize row layers array
    init_row_layers();

    // Create main Window element first
    s_main_window = window_create();

    // Apply the theme and build the layout
    apply_theme_from_settings(settings);
    build_layout_from_settings(settings);

    // Set handler to manage the elements inside the Window
    window_set_window_handlers(
        s_main_window,
        (WindowHandlers){
            .load = main_window_load,
            .unload = main_window_unload
        }
    );

    // Show the Window on the watch, with animated=true
    window_stack_push(s_main_window, true);
}

// deinitializes the watchface
static void deinit() {
    deinit_weather_data();

    // Destroy Window
    window_destroy(s_main_window);
}

// watchface lifecycle
int main(void) {
    app_messaging_initialize();
    init();
    app_messaging_send_app_ready();
    app_event_loop();
    deinit();
}
