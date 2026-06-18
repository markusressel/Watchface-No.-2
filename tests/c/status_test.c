#include "unity/unity.h"
#include <stdbool.h>
#include <string.h>
#include "pebble/pebble.h"

// Mock dependencies
#include "../../src/c/settings/clay_settings.h"
ClaySettings s_settings;
ClaySettings *clay_get_settings() { return &s_settings; }

// phone_connection.h mocks
static bool s_connected = true;
bool phone_connection_is_connected() { return s_connected; }

// layer_factory.h mocks
#include "../../src/c/ui/layer_factory.h"
Layer* layer_factory_create_custom_layer(LayerBuilder builder, LayerUpdateProc update_proc) {
    Layer *layer = layer_create(builder.bounds);
    layer_set_update_proc(layer, update_proc);
    return layer;
}

#include "../../src/c/ui/layer/status.c"

void setUp(void) {
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.DotWidth = 3;
    s_settings.DotScaleFactor = 1.0f;
    s_connected = true;
    s_status_layer = NULL;
    reset_graphics_fill_rect_calls();
}

void tearDown(void) {
    if (s_status_layer) {
        layer_destroy(s_status_layer);
        s_status_layer = NULL;
    }
}

void test_status_layer_create_destroy(void) {
    Layer *parent = layer_create(GRect(0, 0, 144, 168));
    Layer *status = status_layer_create(parent);
    TEST_ASSERT_NOT_NULL(status);
    TEST_ASSERT_EQUAL_PTR(status, s_status_layer);
    
    status_layer_destroy();
    TEST_ASSERT_NULL(s_status_layer);
    
    layer_destroy(parent);
}

void test_status_layer_update_proc_connected(void) {
    Layer *parent = layer_create(GRect(0, 0, 144, 168));
    Layer *status = status_layer_create(parent);
    s_connected = true;
    
    // Trigger update proc
    status_layer_update_proc(status, NULL);
    
    // Should NOT have drawn anything (all segments clear)
    TEST_ASSERT_EQUAL_INT(0, get_graphics_fill_rect_call_count());
    
    layer_destroy(parent);
}

void test_status_layer_update_proc_disconnected(void) {
    Layer *parent = layer_create(GRect(0, 0, 144, 168));
    Layer *status = status_layer_create(parent);
    s_connected = false;
    
    // Trigger update proc
    status_layer_update_proc(status, NULL);
    
    // Should have drawn 1 segment (phone connection)
    TEST_ASSERT_EQUAL_INT(1, get_graphics_fill_rect_call_count());
    TEST_ASSERT_TRUE(gcolor_equal(GColorRed, get_graphics_fill_rect_calls()[0].fill_color));
    
    layer_destroy(parent);
}

void test_status_layer_update_triggers_mark_dirty(void) {
    Layer *parent = layer_create(GRect(0, 0, 144, 168));
    status_layer_create(parent);
    
    status_layer_update();
    // Verified it doesn't crash.
    
    layer_destroy(parent);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_status_layer_create_destroy);
    RUN_TEST(test_status_layer_update_proc_connected);
    RUN_TEST(test_status_layer_update_proc_disconnected);
    RUN_TEST(test_status_layer_update_triggers_mark_dirty);
    return UNITY_END();
}
