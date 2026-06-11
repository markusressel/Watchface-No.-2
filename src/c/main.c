#include "main.h"

#include <pebble.h>
#include "ui/theme.h"
#include "settings/clay_settings.h"
#include "ui/watch_layout.h"
#include "system/tick_listener.h"
#include "system/battery_listener.h"
#include "app_messaging/app_messaging.h"
#include "system/health_listener.h"
#include "ui/ui_state.h"
#include "ui/layer/weather.h"

// Main Window
static Window *s_main_window;

// Layout is built at runtime from settings (see build_layout_from_settings).
static WatchLayout s_layout;

static void build_layout_from_settings(ClaySettings *settings) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "build_layout_from_settings");

    int row_count = settings->LayoutRowCount;
    if (row_count < WATCH_LAYOUT_MIN_ROWS) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Layout row count too low, setting to minimum of %d", WATCH_LAYOUT_MIN_ROWS);
        row_count = WATCH_LAYOUT_MIN_ROWS;
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
    APP_LOG(APP_LOG_LEVEL_DEBUG, "main_window_load");

    ui_state_create_layers(&s_layout);

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
    APP_LOG(APP_LOG_LEVEL_DEBUG, "main_window_unload");
    // unregister listeners
    unregister_tick_listener();
    unregister_battery_listener();
    unregister_health_event_listener();

    ui_state_destroy_layers();
}

static void apply_theme_from_settings(ClaySettings *settings) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "apply_theme_from_settings");
    enum ThemeEnum theme;
    if (settings->ThemeValue[0] == '\0') {
        strcpy(settings->ThemeValue, THEME_LIGHT_STR);
    }
    if (strcmp(settings->ThemeValue, THEME_LIGHT_STR) == 0) {
        theme = LIGHT;
    } else if (strcmp(settings->ThemeValue, THEME_DARK_STR) == 0) {
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
        set_theme(theme, settings->ShowSeconds);
    }

    // Safely update the window background if the window exists
    if (s_main_window) {
        window_set_background_color(s_main_window, theme_get_theme()->BackgroundColor);
    }
}

// Rebuild layout from settings and reload all layers.
// Call this when row layout settings change at runtime.
void main_reload_layout(ClaySettings *settings) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "main_reload_layout");
    main_window_unload(s_main_window);
    apply_theme_from_settings(settings);
    build_layout_from_settings(settings);
    main_window_load(s_main_window);
}

// initializes the watchface
static void init() {
    ClaySettings *settings = clay_load_settings();

    // Create main Window element first
    s_main_window = window_create();

    // Initialize the UI state module
    ui_state_init(s_main_window);

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

    ui_state_deinit();

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
