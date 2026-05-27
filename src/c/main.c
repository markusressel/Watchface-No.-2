#include <pebble.h>
#include "theme.h"
#include "clay_settings.h"
#include "watch_layout.h"
#include "time.h"
#include "tick_listener.h"
#include "date.h"
#include "battery_bar.h"
#include "battery_listener.h"
#include "weather.h"
#include "app_messaging.h"
#include "stepcount.h"
#include "health_listener.h"

// Main Window
static Window *s_main_window;

static ClaySettings *s_settings;

// Display order: change the rows array to reorder widgets.
// Row 0 is at the top; the layout system spaces them evenly across the screen.
static const WatchLayout s_layout = {
    .row_count = 5,
    .rows = {
        [0] = { .widget = WIDGET_WEATHER     },
        [1] = { .widget = WIDGET_DATE        },
        [2] = { .widget = WIDGET_TIME        },
        [3] = { .widget = WIDGET_STEPCOUNT   },
        [4] = { .widget = WIDGET_BATTERY_BAR },
    },
};

// loads components into the main window
static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);

    // Create each layer in row order.  To reorder, edit s_layout.rows above.
    for (int i = 0; i < s_layout.row_count; i++) {
        LayerBuilder builder = watch_layout_make_builder(&s_layout, window_layer, i);
        switch (s_layout.rows[i].widget) {
            case WIDGET_TIME:        create_time_layer(builder);        break;
            case WIDGET_DATE:        create_date_layer(builder);        break;
            case WIDGET_WEATHER:     create_weather_layer(builder);     break;
            case WIDGET_STEPCOUNT:   create_stepcount_layer(builder);   break;
            case WIDGET_BATTERY_BAR: create_battery_bar_layer(builder); break;
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

    initialize_app_messaging();
}

// destroys all components of the main window
static void main_window_unload(Window *window) {
    // unregister listeners

    unregister_tick_listener();
    unregister_battery_listener();
    unregister_health_event_listener();

    /*
    unregister_system_event_listener();

    // destroy layers
    destroy_heartrate_layer();
    destroy_phone_connection_indicator_layer();
    destroy_battery_text_layer();
    */

    destroy_stepcount_layer();
    destroy_battery_bar_layer();
    destroy_date_layer();
    destroy_time_layer();
    destroy_weather_layer();
}

// initializes the watchface
static void init() {
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "initializing settings");
    // load clay configuration
    clay_load_settings();
    s_settings = clay_get_settings();

    // map clay configuration value to ThemeEnum
    // Check if ThemeValue is empty (first character is null)
    if (s_settings->ThemeValue[0] == '\0') {
        strcpy(s_settings->ThemeValue, "LIGHT");
    }

    enum ThemeEnum theme;
    if (strcmp(s_settings->ThemeValue, "DARK") == 0) {
        theme = DARK;
    } else if (strcmp(s_settings->ThemeValue, "CUSTOM") == 0) {
        theme = CUSTOM;
    } else {
        theme = LIGHT;
    }

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "initializing theme");
    // initialize theme based on ThemeEnum
    if (theme == CUSTOM) {
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "init custom theme");
        Theme custom_theme;
        custom_theme.BackgroundColor = s_settings->BackgroundColor;

        // Time Layer
        custom_theme.TimeTextColor = s_settings->TimeTextColor;
        // Date Layer
        custom_theme.DateTextColor = s_settings->DateTextColor;

        // Battery Bar Layer
        custom_theme.BatteryOutlineColor = s_settings->BatteryFrameColor;
        custom_theme.BatteryFillColor = s_settings->BatteryFillColor;

        // Weather Layer
        custom_theme.WeatherTextColor = s_settings->WeatherTextColor;

        // Stepcount Layer
        custom_theme.StepcountTextColor = s_settings->StepcountTextColor;

        init_custom_theme(custom_theme, s_settings->ShowSeconds);
    } else {
        init_theme(theme, s_settings->ShowSeconds);
    }

    // Create main Window element and assign to pointer
    s_main_window = window_create();

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "applying window background");
    // Apply theme
    window_set_background_color(s_main_window, theme_get_theme()->BackgroundColor);

    // Set handler to manage the elements inside the Window
    window_set_window_handlers(s_main_window, (WindowHandlers){
                                   .load = main_window_load,
                                   .unload = main_window_unload
                               });

    // Show the Window on the watch, with animated=true
    window_stack_push(s_main_window, true);
}

// deinitializes the watchface
static void deinit() {
    // Destroy Window
    window_destroy(s_main_window);
}

// watchface lifecycle
int main(void) {
    init();
    app_event_loop();
    deinit();
}
