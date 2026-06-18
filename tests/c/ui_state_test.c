#include "unity/unity.h"
#include <time.h>
#include <pebble.h>
#include "../../src/c/ui/ui_state.h"

// Mock dependencies
#include "../../src/c/ui/watch_layout.h"

// Provide dummy implementations for layer creation/destruction
Layer *create_time_layer(LayerBuilder builder) { return (Layer *) 1; }

void destroy_time_layer(Layer *layer) {
}

Layer *create_date_layer(LayerBuilder builder) { return (Layer *) 2; }

void destroy_date_layer(Layer *layer) {
}

Layer *create_weather_layer(LayerBuilder builder) { return (Layer *) 3; }

void destroy_weather_layer(Layer *layer) {
}

Layer *create_stepcount_layer(LayerBuilder builder) { return (Layer *) 4; }

void destroy_stepcount_layer(Layer *layer) {
}

Layer *create_heartrate_layer(LayerBuilder builder) { return (Layer *) 5; }

void destroy_heartrate_layer(Layer *layer) {
}

Layer *create_weather_forecast_layer(LayerBuilder builder) { return (Layer *) 6; }

void destroy_weather_forecast_layer(Layer *layer) {
}

Layer *create_battery_bar_layer(LayerBuilder builder) { return (Layer *) 7; }

void destroy_battery_bar_layer(Layer *layer) {
}

void debug_layer_destroy_all_borders() {
}

// Mock window_get_root_layer
Layer *window_get_root_layer(const Window *window) { return (Layer *) 100; }

// Mock watch_layout_make_builder
LayerBuilder watch_layout_make_builder(const WatchLayout *layout, Layer *parent, int row_index) {
    return (LayerBuilder)
    {
        .
        parent = parent
    };
}

#include "../../src/c/ui/ui_state.c"

void setUp(void) {
}

void tearDown(void) {
}

void test_ui_state_init(void) {
    Window *window = (Window *) 0x1234;
    ui_state_init(window);

    TEST_ASSERT_EQUAL_INT(0, ui_state_get_row_count());
    TEST_ASSERT_NULL(ui_state_get_layer(0));
    TEST_ASSERT_EQUAL(WIDGET_COUNT, ui_state_get_widget_id(0));
}

void test_ui_state_create_destroy_layers(void) {
    Window *window = (Window *) 0x1234;
    ui_state_init(window);

    WatchLayout layout;
    layout.row_count = 7;
    layout.rows[0].widget = WIDGET_TIME;
    layout.rows[1].widget = WIDGET_DATE;
    layout.rows[2].widget = WIDGET_WEATHER;
    layout.rows[3].widget = WIDGET_STEPCOUNT;
    layout.rows[4].widget = WIDGET_HEARTRATE;
    layout.rows[5].widget = WIDGET_WEATHER_FORECAST;
    layout.rows[6].widget = WIDGET_BATTERY_BAR;

    ui_state_create_layers(&layout);

    TEST_ASSERT_EQUAL_INT(7, ui_state_get_row_count());
    TEST_ASSERT_EQUAL_PTR((Layer *) 1, ui_state_get_layer(0));
    TEST_ASSERT_EQUAL_PTR((Layer *) 2, ui_state_get_layer(1));
    TEST_ASSERT_EQUAL_PTR((Layer *) 3, ui_state_get_layer(2));
    TEST_ASSERT_EQUAL_PTR((Layer *) 4, ui_state_get_layer(3));
    TEST_ASSERT_EQUAL_PTR((Layer *) 5, ui_state_get_layer(4));
    TEST_ASSERT_EQUAL_PTR((Layer *) 6, ui_state_get_layer(5));
    TEST_ASSERT_EQUAL_PTR((Layer *) 7, ui_state_get_layer(6));

    TEST_ASSERT_EQUAL(WIDGET_TIME, ui_state_get_widget_id(0));
    TEST_ASSERT_EQUAL(WIDGET_DATE, ui_state_get_widget_id(1));
    TEST_ASSERT_EQUAL(WIDGET_WEATHER, ui_state_get_widget_id(2));
    TEST_ASSERT_EQUAL(WIDGET_STEPCOUNT, ui_state_get_widget_id(3));
    TEST_ASSERT_EQUAL(WIDGET_HEARTRATE, ui_state_get_widget_id(4));
    TEST_ASSERT_EQUAL(WIDGET_WEATHER_FORECAST, ui_state_get_widget_id(5));
    TEST_ASSERT_EQUAL(WIDGET_BATTERY_BAR, ui_state_get_widget_id(6));

    ui_state_destroy_layers();

    TEST_ASSERT_EQUAL_INT(0, ui_state_get_row_count());
    TEST_ASSERT_NULL(ui_state_get_layer(0));
}

void test_ui_state_deinit(void) {
    Window *window = (Window *) 0x1234;
    ui_state_init(window);

    WatchLayout layout;
    layout.row_count = 1;
    layout.rows[0].widget = WIDGET_TIME;
    ui_state_create_layers(&layout);

    TEST_ASSERT_EQUAL_INT(1, ui_state_get_row_count());

    ui_state_deinit();

    TEST_ASSERT_EQUAL_INT(0, ui_state_get_row_count());
}

void test_ui_state_get_bounds_check(void) {
    ui_state_init(NULL);
    TEST_ASSERT_NULL(ui_state_get_layer(-1));
    TEST_ASSERT_NULL(ui_state_get_layer(10));
    TEST_ASSERT_EQUAL(WIDGET_COUNT, ui_state_get_widget_id(-1));
    TEST_ASSERT_EQUAL(WIDGET_COUNT, ui_state_get_widget_id(10));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_ui_state_init);
    RUN_TEST(test_ui_state_create_destroy_layers);
    RUN_TEST(test_ui_state_deinit);
    RUN_TEST(test_ui_state_get_bounds_check);
    return UNITY_END();
}