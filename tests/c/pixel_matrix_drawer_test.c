#include "unity/unity.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../../src/c/pixel_matrix_drawer.h"
#include "../../src/c/pixel_matrix_drawer.c" // Include the C implementation file directly for testing
#include "pebble/pebble.h" // Mock Pebble SDK

// Helper to compare GRects
bool GRect_equal(GRect r1, GRect r2) {
    return r1.origin.x == r2.origin.x &&
           r1.origin.y == r2.origin.y &&
           r1.size.w == r2.size.w &&
           r1.size.h == r2.size.h;
}

void setUp(void) {
    reset_graphics_fill_rect_calls();
}

void tearDown(void) {
}

// Test pixel_matrix_drawer_char_width
void test_char_width(void) {
    TEST_ASSERT_EQUAL(1, pixel_matrix_drawer_char_width('.', 4));
    TEST_ASSERT_EQUAL(3, pixel_matrix_drawer_char_width('M', 4));
    TEST_ASSERT_EQUAL(5, pixel_matrix_drawer_char_width('C', 4));
    TEST_ASSERT_EQUAL(5, pixel_matrix_drawer_char_width('0', 5)); // Digit size 5
    TEST_ASSERT_EQUAL(4, pixel_matrix_drawer_char_width('0', 4)); // Digit size 4
    TEST_ASSERT_EQUAL(3, pixel_matrix_drawer_char_width('0', 3)); // Digit size 3
    TEST_ASSERT_EQUAL(5, pixel_matrix_drawer_char_width('Z', 4)); // Undefined char should return default width
}

// Test pixel_matrix_drawer_draw_char for a simple character 'X' (from s_default)
void test_draw_char_X(void) {
    GContext *ctx = (GContext *)1; // Dummy context
    GPoint point_zero = {.x = 10, .y = 20};
    char character = 'X'; // This character is not defined, so it should use s_default
    int dot_width = 2;
    int dot_height = 3;
    int gap_h = 1;
    int gap_v = 1;
    bool align_right = false;
    int digit_size = 4;

    int drawn_width = pixel_matrix_drawer_draw_char(
        ctx, point_zero, character, dot_width, dot_height, gap_h, gap_v, align_right, digit_size
    );

    TEST_ASSERT_EQUAL(5, drawn_width); // s_default width is 5
    TEST_ASSERT_EQUAL(25, get_graphics_fill_rect_call_count()); // 5x5 matrix

    GraphicsFillRectCall *calls = get_graphics_fill_rect_calls();

    // Verify a few specific pixels
    // Top-left pixel (0,0)
    TEST_ASSERT_TRUE(GRect_equal(calls[0].rect, GRect(10, 20, 2, 3)));
    // Second pixel in first row (0,1)
    TEST_ASSERT_TRUE(GRect_equal(calls[1].rect, GRect(10 + (2 + 1), 20, 2, 3)));
    // First pixel in second row (1,0)
    TEST_ASSERT_TRUE(GRect_equal(calls[5].rect, GRect(10, 20 + (3 + 1), 2, 3)));
    // Last pixel (4,4)
    TEST_ASSERT_TRUE(GRect_equal(calls[24].rect, GRect(10 + 4 * (2 + 1), 20 + 4 * (3 + 1), 2, 3)));
}

// Test pixel_matrix_drawer_draw_char for character '0' (digit_size 4)
void test_draw_char_0_size_4(void) {
    GContext *ctx = (GContext *)1;
    GPoint point_zero = {.x = 0, .y = 0};
    char character = '0';
    int dot_width = 1;
    int dot_height = 1;
    int gap_h = 0;
    int gap_v = 0;
    bool align_right = false;
    int digit_size = 4;

    int drawn_width = pixel_matrix_drawer_draw_char(
        ctx, point_zero, character, dot_width, dot_height, gap_h, gap_v, align_right, digit_size
    );

    TEST_ASSERT_EQUAL(4, drawn_width); // Width for '0' with digit_size 4
    // Expected pixels for '0' (size 4):
    // X X X X
    // X _ _ X
    // X _ _ X
    // X _ _ X
    // X X X X
    TEST_ASSERT_EQUAL(14, get_graphics_fill_rect_call_count()); // Corrected from 16 to 14

    GraphicsFillRectCall *calls = get_graphics_fill_rect_calls();

    // Verify top row
    TEST_ASSERT_TRUE(GRect_equal(calls[0].rect, GRect(0, 0, 1, 1)));
    TEST_ASSERT_TRUE(GRect_equal(calls[1].rect, GRect(1, 0, 1, 1)));
    TEST_ASSERT_TRUE(GRect_equal(calls[2].rect, GRect(2, 0, 1, 1)));
    TEST_ASSERT_TRUE(GRect_equal(calls[3].rect, GRect(3, 0, 1, 1)));

    // Verify second row (X _ _ X)
    TEST_ASSERT_TRUE(GRect_equal(calls[4].rect, GRect(0, 1, 1, 1)));
    TEST_ASSERT_TRUE(GRect_equal(calls[5].rect, GRect(3, 1, 1, 1))); // Note: calls[5] is the second 'X'

    // Verify last row
    TEST_ASSERT_TRUE(GRect_equal(calls[10].rect, GRect(0, 4, 1, 1))); // Corrected index
    TEST_ASSERT_TRUE(GRect_equal(calls[11].rect, GRect(1, 4, 1, 1))); // Corrected index
    TEST_ASSERT_TRUE(GRect_equal(calls[12].rect, GRect(2, 4, 1, 1))); // Corrected index
    TEST_ASSERT_TRUE(GRect_equal(calls[13].rect, GRect(3, 4, 1, 1))); // Corrected index
}

// Test pixel_matrix_drawer_draw_char with align_right
void test_draw_char_align_right(void) {
    GContext *ctx = (GContext *)1;
    GPoint point_zero = {.x = 50, .y = 30}; // Right edge of drawing area
    char character = '1'; // Width 2 for digit_size 4
    int dot_width = 2;
    int dot_height = 2;
    int gap_h = 1;
    int gap_v = 1;
    bool align_right = true;
    int digit_size = 4;

    int drawn_width = pixel_matrix_drawer_draw_char(
        ctx, point_zero, character, dot_width, dot_height, gap_h, gap_v, align_right, digit_size
    );

    TEST_ASSERT_EQUAL(2, drawn_width); // Width for '1' with digit_size 4
    // Expected pixels for '1' (size 4):
    // X X
    // _ X
    // _ X
    // _ X
    // _ X
    TEST_ASSERT_EQUAL(6, get_graphics_fill_rect_call_count());

    GraphicsFillRectCall *calls = get_graphics_fill_rect_calls();

    // Calculate expected x_offset for align_right
    // Glyph width = 2
    // Total pixel width = 2 * dot_width + (2 - 1) * gap_h = 2*2 + 1*1 = 5
    // x_offset for align_right = point_zero.x - total_pixel_width = 50 - 5 = 45

    // Top-left pixel (0,0) of '1'
    TEST_ASSERT_TRUE(GRect_equal(calls[0].rect, GRect(50 - (2 * 2 + 1 * 1) + 0 * (2 + 1), 30 + 0 * (2 + 1), 2, 2))); // X at (0,0)
    TEST_ASSERT_TRUE(GRect_equal(calls[1].rect, GRect(50 - (2 * 2 + 1 * 1) + 1 * (2 + 1), 30 + 0 * (2 + 1), 2, 2))); // X at (0,1)

    // Second row, second pixel (1,1)
    TEST_ASSERT_TRUE(GRect_equal(calls[2].rect, GRect(50 - (2 * 2 + 1 * 1) + 1 * (2 + 1), 30 + 1 * (2 + 1), 2, 2))); // X at (1,1)
}


int main() {
    UNITY_BEGIN();
    RUN_TEST(test_char_width);
    RUN_TEST(test_draw_char_X);
    RUN_TEST(test_draw_char_0_size_4);
    RUN_TEST(test_draw_char_align_right);
    return UNITY_END();
}
