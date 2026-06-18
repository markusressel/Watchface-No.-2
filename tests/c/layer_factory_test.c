#include "unity/unity.h"
#include <stdbool.h>
#include <stdlib.h>
#include "pebble/pebble.h"

// Mock dependencies
#include "../../src/c/ui/layer/dotted_text_layer.h"

void debug_layer_add_border(Layer *layer) {
}

DottedTextLayer *dotted_text_layer_create(GRect bounds) { return (DottedTextLayer *) malloc(1); }

void dotted_text_layer_set_auto_scale(DottedTextLayer *layer, bool enabled) {
}

void dotted_text_layer_set_color(DottedTextLayer *layer, GColor color) {
}

void dotted_text_layer_set_horizontal_alignment(DottedTextLayer *layer, HorizontalAlignment alignment) {
}

void dotted_text_layer_set_vertical_alignment(DottedTextLayer *layer, VerticalAlignment alignment) {
}

void dotted_text_layer_set_text(DottedTextLayer *layer, char *text) {
}

void dotted_text_layer_destroy(DottedTextLayer *layer) { free(layer); }

#include "../../src/c/ui/layer_factory.c"

void setUp(void) {
}

void tearDown(void) {
}

void test_layer_builder(void) {
    Layer *parent = (Layer *) 0x1234;
    LayerLayout layout = {.x = 10, .y = 20, .width_margin = 5, .height = 30};

    LayerBuilder builder = layer_builder(parent, layout);

    TEST_ASSERT_EQUAL_PTR(parent, builder.parent);
    TEST_ASSERT_EQUAL_INT(10, builder.bounds.origin.x);
    TEST_ASSERT_EQUAL_INT(20, builder.bounds.origin.y);
    TEST_ASSERT_EQUAL_INT(139, builder.bounds.size.w); // 144 - 5
    TEST_ASSERT_EQUAL_INT(30, builder.bounds.size.h);
}

void test_layer_builder_from_rect(void) {
    Layer *parent = (Layer *) 0x1234;
    GRect bounds = GRect(5, 5, 100, 100);

    LayerBuilder builder = layer_builder_from_rect(parent, bounds);

    TEST_ASSERT_EQUAL_PTR(parent, builder.parent);
    TEST_ASSERT_EQUAL_INT(5, builder.bounds.origin.x);
    TEST_ASSERT_EQUAL_INT(100, builder.bounds.size.w);
}

void test_layer_factory_create_text_layer(void) {
    Layer *parent = (Layer *) 0x1234;
    LayerBuilder builder = {.parent = parent, .bounds = GRect(0, 0, 100, 20)};
    TextLayerStyle style = {.background_color = GColorBlack, .text_color = GColorWhite};

    TextLayer *layer = layer_factory_create_text_layer(builder, style);

    TEST_ASSERT_NOT_NULL(layer);
    free(layer);
}

void test_layer_factory_create_dotted_text_layer(void) {
    Layer *parent = (Layer *) 0x1234;
    LayerBuilder builder = {.parent = parent, .bounds = GRect(0, 0, 100, 20)};

    // We need a mock for dotted_text_layer_create
    // Since we are testing the factory, we can just check it calls it.
    // But we need the implementation to return something that can be passed to other mocks.
    DottedTextLayer *layer = layer_factory_create_dotted_text_layer(builder, GColorWhite, HORIZONTAL_ALIGN_LEFT, VERTICAL_ALIGN_TOP, "Test");

    TEST_ASSERT_NOT_NULL(layer);
    free(layer);
}

void test_layer_factory_create_custom_layer(void) {
    Layer *parent = (Layer *) 0x1234;
    LayerBuilder builder = {.parent = parent, .bounds = GRect(0, 0, 100, 20)};

    Layer *layer = layer_factory_create_custom_layer(builder, NULL);

    TEST_ASSERT_NOT_NULL(layer);
    free(layer);
}

void test_layer_factory_create_custom_layer_with_data(void) {
    Layer *parent = (Layer *) 0x1234;
    LayerBuilder builder = {.parent = parent, .bounds = GRect(0, 0, 100, 20)};

    Layer *layer = layer_factory_create_custom_layer_with_data(builder, NULL, 10);

    TEST_ASSERT_NOT_NULL(layer);
    free(layer);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_layer_builder);
    RUN_TEST(test_layer_builder_from_rect);
    RUN_TEST(test_layer_factory_create_text_layer);
    RUN_TEST(test_layer_factory_create_dotted_text_layer);
    RUN_TEST(test_layer_factory_create_custom_layer);
    RUN_TEST(test_layer_factory_create_custom_layer_with_data);
    return UNITY_END();
}