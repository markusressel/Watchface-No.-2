#include "unity/unity.h"
#include <stdbool.h>
#include <string.h>
#include "pebble/pebble.h"

// Mock dependencies
#include "../../src/c/settings/clay_settings.h"
static ClaySettings s_settings;
ClaySettings *clay_load_settings() { return &s_settings; }
ClaySettings *clay_get_settings() { return &s_settings; }

#include "../../src/c/ui/theme.h"
void apply_theme_from_settings(ClaySettings *settings, Window *window) {}

#include "../../src/c/ui/layer/weather.h"
void weather_init_data() {}
void deinit_weather_data() {}

#include "../../src/c/ui/ui_state.h"
void ui_state_init(Window *window) {}
void ui_state_deinit() {}

#include "../../src/c/ui/watch_layout.h"
void build_layout_from_settings(ClaySettings *settings) {}

#include "../../src/c/app_messaging/app_messaging.h"
void app_messaging_initialize() {}
void app_messaging_send_app_ready() {}
void app_messaging_send_app_ready_and_request_settings() {}

#include "../../src/c/ui/window.h"
void window_load(Window *window) {}
void window_unload(Window *window) {}

// Avoid collision with real main
#define main main_under_test
#include "../../src/c/main.c"
#undef main

void setUp(void) {
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.InitialSyncDone = true;
}
void tearDown(void) {}

void test_main_init_deinit(void) {
    init();
    deinit();
}

void test_main_lifecycle(void) {
    main_under_test();
}

void test_main_lifecycle_request_settings(void) {
    s_settings.InitialSyncDone = false;
    main_under_test();
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_main_init_deinit);
    RUN_TEST(test_main_lifecycle);
    RUN_TEST(test_main_lifecycle_request_settings);
    return UNITY_END();
}
