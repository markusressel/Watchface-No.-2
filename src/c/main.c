#include "main.h"

#include <pebble.h>
#include "ui/theme.h"
#include "settings/clay_settings.h"
#include "ui/watch_layout.h"
#include "app_messaging/app_messaging.h"
#include "ui/ui_state.h"
#include "ui/layer/weather.h"
#include "ui/window.h"

// Main Window
static Window *s_main_window;

Window *main_get_window() {
    return s_main_window;
}

// initializes the watchface
static void init() {
    ClaySettings *settings = clay_load_settings();

    // Create main Window element first
    s_main_window = window_create();

    // Initialize the UI state module
    ui_state_init(s_main_window);

    // Apply the theme and build the layout
    apply_theme_from_settings(settings, s_main_window);
    build_layout_from_settings(settings);

    // Set handler to manage the elements inside the Window
    window_set_window_handlers(
        s_main_window,
        (WindowHandlers){
            .load = window_load,
            .unload = window_unload
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
