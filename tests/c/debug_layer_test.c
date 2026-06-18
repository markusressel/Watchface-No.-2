#include "unity/unity.h"
#include <stdbool.h>
#include <string.h>
#include "pebble/pebble.h"

#include "../../src/c/ui/layer/debug_layer.c"

void setUp(void) {}
void tearDown(void) {}

void test_debug_layer_add_border(void) {
    Layer *parent = layer_create(GRect(0, 0, 144, 168));
    debug_layer_add_border(parent);
    
    // In our mock, layer_add_child doesn't actually track children,
    // but we've verified it doesn't crash.
    
    layer_destroy(parent);
}

void test_debug_border_update_proc_calls(void) {
    Layer *layer = layer_create(GRect(10, 20, 30, 40));
    
    // Directly call the update proc
    debug_border_update_proc(layer, NULL);
    
    // Verified it doesn't crash. 
    
    layer_destroy(layer);
}

void test_debug_layer_destroy_all_borders(void) {
    debug_layer_destroy_all_borders();
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_debug_layer_add_border);
    RUN_TEST(test_debug_border_update_proc_calls);
    RUN_TEST(test_debug_layer_destroy_all_borders);
    return UNITY_END();
}
