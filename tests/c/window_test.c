#include "unity/unity.h"
#include <stdbool.h>
#include <string.h>
#include "pebble/pebble.h"

// Mock dependencies
#include "../../src/c/ui/watch_layout.h"
WatchLayout *watch_layout_get_layout() { return NULL; }

#include "../../src/c/ui/ui_state.h"
void ui_state_create_layers(WatchLayout *layout) {}
void ui_state_destroy_layers() {}

#include "../../src/c/system/tick_listener.h"
void register_tick_listener() {}
void unregister_tick_listener() {}

#include "../../src/c/system/battery_listener.h"
void register_battery_listener() {}
void unregister_battery_listener() {}
void force_battery_update() {}

#include "../../src/c/system/health_listener.h"
void register_health_event_listener() {}
void unregister_health_event_listener() {}

#include "../../src/c/system/phone_connection.h"
void register_phone_connection_listener() {}
void unregister_phone_connection_listener() {}
void force_phone_connection_update() {}

#include "../../src/c/ui/layer/status.h"
Layer *status_layer_create(Layer *parent) { return NULL; }
void status_layer_destroy() {}

#include "../../src/c/ui/window.c"

void setUp(void) {}
void tearDown(void) {}

void test_window_load_unload(void) {
    Window *window = window_create();
    window_load(window);
    window_unload(window);
    window_destroy(window);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_window_load_unload);
    return UNITY_END();
}
