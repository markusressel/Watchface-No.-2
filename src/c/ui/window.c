#include "window.h"

#include <pebble.h>
#include "watch_layout.h"
#include "ui_state.h"
#include "../system/tick_listener.h"
#include "../system/battery_listener.h"
#include "../system/health_listener.h"

// loads components into the main window
void window_load(Window *window) {
    if (!window) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "window_load: window is NULL!");
        return;
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "main_window_load");

    ui_state_create_layers(watch_layout_get_layout());

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
void window_unload(Window *window) {
    if (!window) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "window_unload: window is NULL!");
        return;
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "main_window_unload");
    // unregister listeners
    unregister_tick_listener();
    unregister_battery_listener();
    unregister_health_event_listener();

    ui_state_destroy_layers();
}
