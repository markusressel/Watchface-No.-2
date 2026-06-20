#include "unity/unity.h"
#include <stdbool.h>
#include <string.h>
#include "pebble/pebble.h"

// Mock dependencies
#include "../../src/c/settings/clay_settings.h"
ClaySettings s_settings;
ClaySettings *clay_get_settings() { return &s_settings; }

// Use real pixel_matrix_drawer
#include "../../src/c/ui/graphics/pixel_matrix_drawer.c"

#include "../../src/c/ui/layer/dotted_text_layer.c"

void setUp(void) {
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.DotHeight = 3;
    s_settings.DotWidth = 3;
    s_settings.DotVerticalGap = 1;
    s_settings.DotHorizontalGap = 1;
    s_settings.DigitWidth = 4;
}

void tearDown(void) {}

void test_dotted_text_layer_create_destroy(void) {
    DottedTextLayer *layer = dotted_text_layer_create(GRect(0, 0, 144, 30));
    TEST_ASSERT_NOT_NULL(layer);
    dotted_text_layer_destroy(layer);
}

void test_dotted_text_layer_set_text(void) {
    DottedTextLayer *layer = dotted_text_layer_create(GRect(0, 0, 144, 30));
    dotted_text_layer_set_text(layer, "123");
    DottedTextLayerData *data = layer_get_data(layer);
    TEST_ASSERT_EQUAL_STRING("123", data->text);
    
    dotted_text_layer_set_text(layer, "4567");
    TEST_ASSERT_EQUAL_STRING("4567", data->text);
    
    dotted_text_layer_set_text(layer, NULL);
    TEST_ASSERT_NULL(data->text);
    
    dotted_text_layer_destroy(layer);
}

void test_dotted_text_layer_setters(void) {
    DottedTextLayer *layer = dotted_text_layer_create(GRect(0, 0, 144, 30));
    
    dotted_text_layer_set_horizontal_alignment(layer, HORIZONTAL_ALIGN_CENTER);
    dotted_text_layer_set_vertical_alignment(layer, VERTICAL_ALIGN_BOTTOM);
    dotted_text_layer_set_rendering_mode(layer, DOTTED_TEXT_RENDERING_MODE_SUBPIXEL);
    dotted_text_layer_set_color(layer, GColorRed);
    dotted_text_layer_set_scale_factor(layer, 2.0f);
    dotted_text_layer_set_character_offset(layer, 5, DOTTED_TEXT_OFFSET_UNIT_BLOCKS);
    dotted_text_layer_set_auto_scale(layer, true);
    dotted_text_layer_set_custom_metrics(layer, 4, 4, 2, 2);
    dotted_text_layer_set_digit_width(layer, 5);
    
    DottedTextLayerData *data = layer_get_data(layer);
    TEST_ASSERT_EQUAL(HORIZONTAL_ALIGN_CENTER, data->horizontal_alignment);
    TEST_ASSERT_EQUAL(VERTICAL_ALIGN_BOTTOM, data->vertical_alignment);
    TEST_ASSERT_EQUAL(DOTTED_TEXT_RENDERING_MODE_SUBPIXEL, data->rendering_mode);
    TEST_ASSERT_TRUE(gcolor_equal(GColorRed, data->text_color));
    TEST_ASSERT_EQUAL_FLOAT(2.0f, data->scale_factor);
    TEST_ASSERT_EQUAL(5, data->character_offset_value);
    TEST_ASSERT_EQUAL(DOTTED_TEXT_OFFSET_UNIT_BLOCKS, data->character_offset_unit);
    TEST_ASSERT_TRUE(data->auto_scale);
    TEST_ASSERT_TRUE(data->use_custom_metrics);
    TEST_ASSERT_EQUAL(4, data->custom_dot_width);
    TEST_ASSERT_EQUAL(5, data->custom_digit_width);
    
    dotted_text_layer_destroy(layer);
}

void test_dotted_text_layer_get_content_width(void) {
    DottedTextLayer *layer = dotted_text_layer_create(GRect(0, 0, 144, 30));
    dotted_text_layer_set_text(layer, "1");
    
    int width = dotted_text_layer_get_content_width(layer);
    TEST_ASSERT_TRUE(width > 0);
    
    dotted_text_layer_set_text(layer, "11");
    int width2 = dotted_text_layer_get_content_width(layer);
    TEST_ASSERT_TRUE(width2 > width);
    
    dotted_text_layer_destroy(layer);
}

void test_dotted_text_layer_update_proc(void) {
    DottedTextLayer *layer = dotted_text_layer_create(GRect(0, 0, 144, 30));
    dotted_text_layer_set_text(layer, "12:34");
    
    // Trigger update proc
    update_proc(layer, NULL);
    
    // Verified it doesn't crash.
    
    dotted_text_layer_destroy(layer);
}

void test_dotted_text_layer_leak_checking(void) {
    mock_memory_reset();
    TEST_ASSERT_EQUAL_INT(0, mock_memory_allocated_bytes());

    DottedTextLayer *layer = dotted_text_layer_create(GRect(0, 0, 144, 30));
    TEST_ASSERT_NOT_NULL(layer);
    
    // Set some text (should allocate memory for the text)
    dotted_text_layer_set_text(layer, "Leak Test Text");
    TEST_ASSERT_TRUE(mock_memory_allocated_bytes() > 0);

    // Set other text (should free old text and allocate new text)
    dotted_text_layer_set_text(layer, "Different text");

    // Set text to NULL (should free text memory)
    dotted_text_layer_set_text(layer, NULL);

    dotted_text_layer_destroy(layer);

    // Verify all allocations are freed
    TEST_ASSERT_EQUAL_INT(0, mock_memory_allocated_bytes());
    TEST_ASSERT_EQUAL_INT(0, mock_memory_alloc_count());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_dotted_text_layer_create_destroy);
    RUN_TEST(test_dotted_text_layer_set_text);
    RUN_TEST(test_dotted_text_layer_setters);
    RUN_TEST(test_dotted_text_layer_get_content_width);
    RUN_TEST(test_dotted_text_layer_update_proc);
    RUN_TEST(test_dotted_text_layer_leak_checking);
    return UNITY_END();
}
