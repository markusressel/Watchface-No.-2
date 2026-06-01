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
#include "health_listener.h"

// Main Window
static Window *s_main_window;

static ClaySettings *s_settings;

// Emery (time 2 hardware) supports up to 7 rows, others up to 5.
#ifdef PBL_PLATFORM_EMERY
#define MAX_LAYOUT_ROW_COUNT 7
#else
#define MAX_LAYOUT_ROW_COUNT 5
#endif

// Layout is built at runtime from settings (see build_layout_from_settings).
// Row 2 is always WIDGET_TIME; all other rows come from user config.
static WatchLayout s_layout;

// Store created layer instances indexed by row for proper cleanup
static Layer *s_row_layers[WATCH_LAYOUT_MAX_ROWS];

static void init_row_layers() {
    for (int i = 0; i < WATCH_LAYOUT_MAX_ROWS; i++) {
        s_row_layers[i] = NULL;
    }
}

static int clamp_layout_row_count(const int row_count) {
    const int min_rows = 5;
    if (row_count < min_rows) {
        return min_rows;
    }
    if (row_count > MAX_LAYOUT_ROW_COUNT) {
        return MAX_LAYOUT_ROW_COUNT;
    }
    return row_count;
}

static void build_layout_from_settings() {
    const int row_count = clamp_layout_row_count(s_settings->LayoutRowCount);

    s_layout = (WatchLayout){
        .row_count = row_count,
        .rows = {
            [0] = {.widget = (WidgetId) s_settings->Row0Widget},
            [1] = {.widget = (WidgetId) s_settings->Row1Widget},
            [2] = {.widget = WIDGET_TIME},
            [3] = {.widget = (WidgetId) s_settings->Row3Widget},
            [4] = {.widget = (WidgetId) s_settings->Row4Widget},
            [5] = {.widget = (WidgetId) s_settings->Row5Widget},
            [6] = {.widget = (WidgetId) s_settings->Row6Widget},
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
                case WIDGET_BATTERY_BAR: destroy_battery_bar_layer(s_row_layers[i]);
                    break;
                default: break;
            }
            s_row_layers[i] = NULL;
        }
    }
}

static void apply_theme_from_settings() {
    if (s_settings->ThemeValue[0] == '\0') {
        strcpy(s_settings->ThemeValue, "LIGHT");
    }

    enum ThemeEnum theme;
    if (strcmp(s_settings->ThemeValue, "DARK") == 0) {
        theme = DARK;
    } else if (strcmp(s_settings->ThemeValue, "CUSTOM") == 0) {
        theme = CUSTOM;
    } else {
        theme = DARK;
    }

    if (theme == CUSTOM) {
        Theme custom_theme;
        custom_theme.BackgroundColor = s_settings->BackgroundColor;
        custom_theme.TimeTextColor = s_settings->TimeTextColor;
        custom_theme.DateTextColor = s_settings->DateTextColor;
        custom_theme.BatteryOutlineColor = s_settings->BatteryFrameColor;
        custom_theme.BatteryFillColor = s_settings->BatteryFillColor;
        custom_theme.WeatherTextColor = s_settings->WeatherTextColor;
        custom_theme.StepcountTextColor = s_settings->StepcountTextColor;

        init_custom_theme(custom_theme, s_settings->ShowSeconds);
    } else {
        init_theme(theme, s_settings->ShowSeconds);
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
    apply_theme_from_settings();
    build_layout_from_settings();
    main_window_load(s_main_window);
}

// initializes the watchface
static void init() {
    clay_load_settings();
    s_settings = clay_get_settings();

    // Initialize row layers array
    init_row_layers();

    // Create main Window element first
    s_main_window = window_create();

    // Apply the theme and build the layout
    apply_theme_from_settings();
    build_layout_from_settings();

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
    // Destroy Window
    window_destroy(s_main_window);
}

// watchface lifecycle
int main(void) {
    init();
    initialize_app_messaging();
    app_event_loop();
    deinit();
}
