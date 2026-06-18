#include "unity/unity.h"
#include <stdbool.h>
#include <string.h>
#include "pebble/pebble.h"

// Mock dependencies
#include "../../src/c/ui/layer_factory.h"
#include "../../src/c/settings/clay_settings.h"

ClaySettings s_settings;
ClaySettings *clay_get_settings() { return &s_settings; }

void window_unload(Window *window) {
}

void window_load(Window *window) {
}

void apply_theme_from_settings(ClaySettings *settings, Window *window) {
}

LayerBuilder layer_builder_from_rect(Layer *parent, GRect bounds) {
    return (LayerBuilder)
    {
        .
        parent = parent,
        .
        bounds = bounds
    };
}

#include "../../src/c/ui/watch_layout.c"

void setUp(void) {
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.DotHeight = 3;
    s_settings.DotVerticalGap = 3;
    s_settings.LayoutRowCount = 5;
    s_settings.Row0Widget = WIDGET_WEATHER;
    s_settings.Row1Widget = WIDGET_TIME;
    s_settings.Row2Widget = WIDGET_DATE;
    s_settings.Row3Widget = WIDGET_STEPCOUNT;
    s_settings.Row4Widget = WIDGET_BATTERY_BAR;
    s_settings.DotAutoScale = false;
    s_settings.DotScaleFactor = 1.0f;
}

void tearDown(void) {
}

void test_build_layout_from_settings(void) {
    build_layout_from_settings(&s_settings);
    WatchLayout *layout = watch_layout_get_layout();

    TEST_ASSERT_EQUAL_INT(5, layout->row_count);
    TEST_ASSERT_EQUAL(WIDGET_WEATHER, layout->rows[0].widget);
    TEST_ASSERT_EQUAL(WIDGET_TIME, layout->rows[1].widget);
    TEST_ASSERT_EQUAL(WIDGET_DATE, layout->rows[2].widget);
}

void test_watch_layout_make_builder_manual_scale(void) {
    build_layout_from_settings(&s_settings);
    WatchLayout *layout = watch_layout_get_layout();
    Layer *window_layer = (Layer *) 0x1234;

    // Row 0: Weather
    LayerBuilder builder0 = watch_layout_make_builder(layout, window_layer, 0);
    TEST_ASSERT_EQUAL_PTR(window_layer, builder0.parent);
    TEST_ASSERT_EQUAL_INT(0, builder0.bounds.origin.x);
    TEST_ASSERT_EQUAL_INT(5, builder0.bounds.origin.y); // EDGE_MARGIN
    TEST_ASSERT_EQUAL_INT(141, builder0.bounds.size.w); // 144 - 3
    TEST_ASSERT_EQUAL_INT(27, builder0.bounds.size.h); // (5*3) + (4*3) = 27

    // Row 1: Time
    LayerBuilder builder1 = watch_layout_make_builder(layout, window_layer, 1);
    TEST_ASSERT_EQUAL_INT(54, builder1.bounds.size.h); // (5*10) + (4*1) = 54 (gap is 1 because of scaled_dimension)
}

void test_watch_layout_make_builder_auto_scale(void) {
    s_settings.DotAutoScale = true;
    s_settings.TimeRowRatio = 1.5f;
    build_layout_from_settings(&s_settings);
    WatchLayout *layout = watch_layout_get_layout();
    Layer *window_layer = (Layer *) 0x1234;

    LayerBuilder builder1 = watch_layout_make_builder(layout, window_layer, 1);
    TEST_ASSERT_TRUE(builder1.bounds.size.h > 0);
}

void test_main_reload_layout(void) {
    Window *window = (Window *) 0x1234;
    main_reload_layout(&s_settings, window);
    WatchLayout *layout = watch_layout_get_layout();
    TEST_ASSERT_EQUAL_INT(5, layout->row_count);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_build_layout_from_settings);
    RUN_TEST(test_watch_layout_make_builder_manual_scale);
    RUN_TEST(test_watch_layout_make_builder_auto_scale);
    RUN_TEST(test_main_reload_layout);
    return UNITY_END();
}