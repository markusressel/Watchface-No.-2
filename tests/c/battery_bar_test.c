#include "unity/unity.h"
#include <stdbool.h>
#include <string.h>
#include "pebble/pebble.h"

// Mock dependencies
#include "../../src/c/settings/clay_settings.h"
#include "../../src/c/ui/theme.h"

ClaySettings s_settings;
ClaySettings *clay_get_settings() { return &s_settings; }

static Theme s_theme = {
    .BatteryOutlineColor = {.argb = 0b11111111},
    .BatteryFillColor = {.argb = 0b11001100},
    .BatteryLowColor = {.argb = 0b11110000}
};
Theme *theme_get_theme() { return &s_theme; }

// battery.h mocks
int s_battery_level = 100;
bool s_battery_charging = false;

// ui_state.h mocks
#include "../../src/c/ui/ui_state.h"
static Layer* s_mock_layers[5];
static WidgetId s_mock_widgets[5];
static int s_row_count = 0;

int ui_state_get_row_count() { return s_row_count; }
WidgetId ui_state_get_widget_id(int row) { return s_mock_widgets[row]; }
Layer* ui_state_get_layer(int row) { return s_mock_layers[row]; }

// layer_factory.h mocks
#include "../../src/c/ui/layer_factory.h"
Layer* layer_factory_create_custom_layer_with_data(LayerBuilder builder, LayerUpdateProc update_proc, size_t data_size) {
    Layer *layer = layer_create_with_data(builder.bounds, data_size);
    layer_set_update_proc(layer, update_proc);
    return layer;
}

#include "../../src/c/ui/layer/battery_bar.c"

void setUp(void) {
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.DotHeight = 3;
    s_settings.DotWidth = 3;
    s_settings.DotVerticalGap = 1;
    s_settings.DotHorizontalGap = 1;
    s_settings.DotScaleFactor = 1.0f;
    s_settings.LowBatteryThreshold = 20;

    s_battery_level = 100;
    s_battery_charging = false;
    s_row_count = 0;
    s_active_battery_layers = 0;
    charging_animation = NULL;

    reset_graphics_fill_rect_calls();
}

void tearDown(void) {
    if (charging_animation) {
        free(charging_animation);
        charging_animation = NULL;
    }
}

void test_battery_bar_layer_create_destroy(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_battery_bar_layer(builder);
    TEST_ASSERT_NOT_NULL(layer);
    TEST_ASSERT_EQUAL_INT(1, s_active_battery_layers);

    destroy_battery_bar_layer(layer);
    TEST_ASSERT_EQUAL_INT(0, s_active_battery_layers);
}

void test_battery_bar_setters(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_battery_bar_layer(builder);
    
    battery_bar_layer_set_horizontal_alignment(layer, HORIZONTAL_ALIGN_LEFT);
    BatteryLayerData *data = layer_get_data(layer);
    TEST_ASSERT_EQUAL(HORIZONTAL_ALIGN_LEFT, data->horizontal_alignment);

    battery_bar_layer_set_vertical_alignment(layer, VERTICAL_ALIGN_BOTTOM);
    TEST_ASSERT_EQUAL(VERTICAL_ALIGN_BOTTOM, data->vertical_alignment);

    battery_bar_layer_set_rendering_mode(layer, DOTTED_TEXT_RENDERING_MODE_SUBPIXEL);
    TEST_ASSERT_EQUAL(DOTTED_TEXT_RENDERING_MODE_SUBPIXEL, data->rendering_mode);

    destroy_battery_bar_layer(layer);
}

void test_battery_bar_update_proc_full(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_battery_bar_layer(builder);
    s_battery_level = 100;
    s_current_battery_level = 100;

    // Trigger update proc
    battery_update_proc(layer, NULL);

    // Should have drawn something
    TEST_ASSERT_TRUE(get_graphics_fill_rect_call_count() > 0);
    
    destroy_battery_bar_layer(layer);
}

void test_battery_bar_animation_state(void) {
    s_battery_charging = true;
    ensure_battery_animation_state();
    TEST_ASSERT_NOT_NULL(charging_animation);

    s_battery_charging = false;
    ensure_battery_animation_state();
    TEST_ASSERT_NULL(charging_animation);
}

void test_update_battery_bar_triggers_mark_dirty(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_battery_bar_layer(builder);
    
    s_row_count = 1;
    s_mock_widgets[0] = WIDGET_BATTERY_BAR;
    s_mock_layers[0] = layer;

    update_battery_bar();
    // In our mock, layer_mark_dirty doesn't do anything visible, 
    // but we've verified it doesn't crash.
    
    destroy_battery_bar_layer(layer);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_battery_bar_layer_create_destroy);
    RUN_TEST(test_battery_bar_setters);
    RUN_TEST(test_battery_bar_update_proc_full);
    RUN_TEST(test_battery_bar_animation_state);
    RUN_TEST(test_update_battery_bar_triggers_mark_dirty);
    return UNITY_END();
}
