#include "unity/unity.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../../src/c/layer/graph_utils.h"
#include "../../src/c/layer/graph_utils.c" // Include the C implementation file directly for testing
#include "pebble/pebble.h" // Mock Pebble SDK

// Helper to compare GRects
bool GRect_equal(GRect r1, GRect r2) {
    return r1.origin.x == r2.origin.x &&
           r1.origin.y == r2.origin.y &&
           r1.size.w == r2.size.w &&
           r1.size.h == r2.size.h;
}

// Helper to compare GPoints
bool GPoint_equal(GPoint p1, GPoint p2) {
    return p1.x == p2.x && p1.y == p2.y;
}

// Helper macro for GColor comparison
#define TEST_ASSERT_EQUAL_GCOLOR(expected, actual) \
    TEST_ASSERT_EQUAL_HEX((expected).argb, (actual).argb)

void setUp(void) {
    reset_graphics_fill_rect_calls();
    reset_graphics_draw_line_calls();
}

void tearDown(void) {
}

// --- Test static helper functions ---

void test_clamp_int(void) {
    TEST_ASSERT_EQUAL(5, clamp_int(5, 0, 10));
    TEST_ASSERT_EQUAL(0, clamp_int(-5, 0, 10));
    TEST_ASSERT_EQUAL(10, clamp_int(15, 0, 10));
    TEST_ASSERT_EQUAL(0, clamp_int(0, 0, 0));
    TEST_ASSERT_EQUAL(0, clamp_int(10, 0, 0));
}

void test_channel2_to_8(void) {
    TEST_ASSERT_EQUAL(0, channel2_to_8(0));
    TEST_ASSERT_EQUAL(85, channel2_to_8(1));
    TEST_ASSERT_EQUAL(170, channel2_to_8(2));
    TEST_ASSERT_EQUAL(255, channel2_to_8(3));
    TEST_ASSERT_EQUAL(0, channel2_to_8(-1)); // Clamped
    TEST_ASSERT_EQUAL(255, channel2_to_8(4)); // Clamped
}

void test_channel8_to_2(void) {
    TEST_ASSERT_EQUAL(0, channel8_to_2(0));
    TEST_ASSERT_EQUAL(0, channel8_to_2(42)); // 0-42 -> 0
    TEST_ASSERT_EQUAL(1, channel8_to_2(43)); // 43-127 -> 1
    TEST_ASSERT_EQUAL(1, channel8_to_2(127));
    TEST_ASSERT_EQUAL(2, channel8_to_2(128)); // 128-212 -> 2
    TEST_ASSERT_EQUAL(2, channel8_to_2(212));
    TEST_ASSERT_EQUAL(3, channel8_to_2(213)); // 213-255 -> 3
    TEST_ASSERT_EQUAL(3, channel8_to_2(255));
    TEST_ASSERT_EQUAL(0, channel8_to_2(-10)); // Clamped
    TEST_ASSERT_EQUAL(3, channel8_to_2(300)); // Clamped
}

void test_interpolate_color(void) {
    GColor from = GColorBlack; // ARGB 0b11000000
    GColor to = GColorWhite; // ARGB 0b11111111

    // t=0 -> from
    TEST_ASSERT_EQUAL_GCOLOR(GColorBlack, interpolate_color(from, to, 0, false, 0, 0));
    // t=1000 -> to
    TEST_ASSERT_EQUAL_GCOLOR(GColorWhite, interpolate_color(from, to, 1000, false, 0, 0));

    // Midpoint (t=500) - should be a greyish color
    // Black (0,0,0) to White (255,255,255)
    // Midpoint should be (127,127,127) approx
    // channel8_to_2(127) = 1
    // So ARGB 0b11010101
    GColor expected_mid_bw = (GColor){.argb = 0b11010101};
    TEST_ASSERT_EQUAL_GCOLOR(expected_mid_bw, interpolate_color(from, to, 500, false, 0, 0));

    // Test with specific colors
    from = GColorRed; // 0b11110000 (A=3, R=3, G=0, B=0)
    to = GColorBlue; // 0b11000011 (A=3, R=0, G=0, B=3)
    // t=500: A=3, R=2, G=0, B=1 -> 0b11100001 (calculated: out_r=128 -> channel8_to_2(128)=2)
    GColor expected_mid_rb = (GColor){.argb = 0b11100001};
    TEST_ASSERT_EQUAL_GCOLOR(expected_mid_rb, interpolate_color(from, to, 500, false, 0, 0));
}

void test_compute_min_max(void) {
    int values1[] = {1, 5, 2, 8, 3};
    int min1, max1;
    compute_min_max(values1, 5, &min1, &max1);
    TEST_ASSERT_EQUAL(1, min1);
    TEST_ASSERT_EQUAL(8, max1);

    int values2[] = {10};
    int min2, max2;
    compute_min_max(values2, 1, &min2, &max2);
    TEST_ASSERT_EQUAL(10, min2);
    TEST_ASSERT_EQUAL(10, max2);

    int values3[] = {-5, -1, -10};
    int min3, max3;
    compute_min_max(values3, 3, &min3, &max3);
    TEST_ASSERT_EQUAL(-10, min3);
    TEST_ASSERT_EQUAL(-1, max3);
}

void test_graph_value_to_y(void) {
    // Basic mapping
    TEST_ASSERT_EQUAL(100, graph_value_to_y(0, 0, 100, 100)); // min value maps to max y
    TEST_ASSERT_EQUAL(0, graph_value_to_y(100, 0, 100, 100)); // max value maps to min y
    TEST_ASSERT_EQUAL(50, graph_value_to_y(50, 0, 100, 100)); // mid value maps to mid y

    // Different range
    TEST_ASSERT_EQUAL(100, graph_value_to_y(10, 10, 60, 100));
    TEST_ASSERT_EQUAL(0, graph_value_to_y(60, 10, 60, 100));
    TEST_ASSERT_EQUAL(50, graph_value_to_y(35, 10, 60, 100)); // (35-10)*100/(60-10) = 25*100/50 = 50. 100-50 = 50

    // Zero drawable height
    TEST_ASSERT_EQUAL(0, graph_value_to_y(50, 0, 100, 0));

    // Max <= Min
    TEST_ASSERT_EQUAL(50, graph_value_to_y(50, 0, 0, 100)); // Should return drawable_height / 2
}

void test_x_for_index(void) {
    // Total 1, width 100, dot_size 5
    TEST_ASSERT_EQUAL(0, x_for_index(0, 1, 100, 5));

    // Total 2, width 100, dot_size 5
    // (width - dot_size) = 95
    // (total - 1) = 1
    TEST_ASSERT_EQUAL(0, x_for_index(0, 2, 100, 5)); // 0 * 95 / 1 = 0
    TEST_ASSERT_EQUAL(95, x_for_index(1, 2, 100, 5)); // 1 * 95 / 1 = 95

    // Total 5, width 100, dot_size 5
    // (width - dot_size) = 95
    // (total - 1) = 4
    TEST_ASSERT_EQUAL(0, x_for_index(0, 5, 100, 5)); // 0 * 95 / 4 = 0
    TEST_ASSERT_EQUAL(23, x_for_index(1, 5, 100, 5)); // 1 * 95 / 4 = 23 (integer division)
    TEST_ASSERT_EQUAL(47, x_for_index(2, 5, 100, 5)); // 2 * 95 / 4 = 47
    TEST_ASSERT_EQUAL(71, x_for_index(3, 5, 100, 5)); // 3 * 95 / 4 = 71
    TEST_ASSERT_EQUAL(95, x_for_index(4, 5, 100, 5)); // 4 * 95 / 4 = 95
}

void test_graph_y_to_value(void) {
    // Basic mapping
    TEST_ASSERT_EQUAL(100, graph_y_to_value(0, 0, 100, 100)); // min y maps to max value
    TEST_ASSERT_EQUAL(0, graph_y_to_value(100, 0, 100, 100)); // max y maps to min value
    TEST_ASSERT_EQUAL(50, graph_y_to_value(50, 0, 100, 100)); // mid y maps to mid value

    // Different range
    TEST_ASSERT_EQUAL(60, graph_y_to_value(0, 10, 60, 100));
    TEST_ASSERT_EQUAL(10, graph_y_to_value(100, 10, 60, 100));
    TEST_ASSERT_EQUAL(35, graph_y_to_value(50, 10, 60, 100)); // 60 - (50 * (60-10) / 100) = 60 - (50*50/100) = 60 - 25 = 35

    // Zero drawable height
    TEST_ASSERT_EQUAL(0, graph_y_to_value(50, 0, 100, 0)); // Returns min_value

    // Max <= Min
    TEST_ASSERT_EQUAL(0, graph_y_to_value(50, 0, 0, 100)); // Returns min_value
}

void test_interpolation_steps_for_segment(void) {
    TEST_ASSERT_EQUAL(0, interpolation_steps_for_segment(0, 0, 1)); // No distance
    TEST_ASSERT_EQUAL(0, interpolation_steps_for_segment(0, 1, 1)); // dist=1, spacing=1 -> (1/1)-1 = 0
    TEST_ASSERT_EQUAL(1, interpolation_steps_for_segment(0, 2, 1)); // dist=2, spacing=1 -> (2/1)-1 = 1
    TEST_ASSERT_EQUAL(2, interpolation_steps_for_segment(0, 3, 1)); // dist=3, spacing=1 -> (3/1)-1 = 2 (Corrected expectation)
    TEST_ASSERT_EQUAL(0, interpolation_steps_for_segment(0, 5, 5)); // dist=5, spacing=5 -> (5/5)-1 = 0
    TEST_ASSERT_EQUAL(1, interpolation_steps_for_segment(0, 11, 5)); // dist=11, spacing=5 -> (11/5)-1 = 2-1 = 1
    TEST_ASSERT_EQUAL(32, interpolation_steps_for_segment(0, 1000, 1)); // Clamped to 32
}

void test_smoothstep_interpolate(void) {
    TEST_ASSERT_EQUAL(0, smoothstep_interpolate(0, 100, 0, 10)); // Start
    TEST_ASSERT_EQUAL(100, smoothstep_interpolate(0, 100, 10, 10)); // End

    // Test a midpoint (t_scaled = 500)
    // smooth = (500*500*(3000 - 2*500))/1000/1000 = (250000 * 2000) / 1000000 = 500
    // result = 0 + (100 * 500) / 1000 = 50
    TEST_ASSERT_EQUAL(50, smoothstep_interpolate(0, 100, 5, 10));

    // Test with negative values
    TEST_ASSERT_EQUAL(-50, smoothstep_interpolate(-100, 0, 5, 10));
    TEST_ASSERT_EQUAL(0, smoothstep_interpolate(-100, 100, 5, 10)); // Midpoint of -100 to 100 is 0
}

void test_should_suppress_value(void) {
    GraphSeriesConfig config_no_suppress = {.suppress_exact_zero_value = false};
    GraphSeriesConfig config_suppress = {.suppress_exact_zero_value = true};

    TEST_ASSERT_FALSE(should_suppress_value(&config_no_suppress, 0));
    TEST_ASSERT_FALSE(should_suppress_value(&config_no_suppress, 10));
    TEST_ASSERT_TRUE(should_suppress_value(&config_suppress, 0));
    TEST_ASSERT_FALSE(should_suppress_value(&config_suppress, 10));
}

// --- Test graph_color_for_stops ---

void test_graph_color_for_stops_no_stops(void) {
    GraphSeriesConfig config = {
        .default_color = GColorBlack,
        .color_stops = NULL,
        .color_stop_count = 0,
    };
    TEST_ASSERT_EQUAL_GCOLOR(GColorBlack, graph_color_for_stops(&config, 10, 0, 0, false));
}

void test_graph_color_for_stops_single_stop(void) {
    GraphColorStop stops[] = {{.value = 50, .color = GColorRed}};
    GraphSeriesConfig config = {
        .default_color = GColorBlack,
        .color_stops = stops,
        .color_stop_count = 1,
    };
    TEST_ASSERT_EQUAL_GCOLOR(GColorRed, graph_color_for_stops(&config, 10, 0, 0, false));
    TEST_ASSERT_EQUAL_GCOLOR(GColorRed, graph_color_for_stops(&config, 50, 0, 0, false));
    TEST_ASSERT_EQUAL_GCOLOR(GColorRed, graph_color_for_stops(&config, 100, 0, 0, false));
}

void test_graph_color_for_stops_multiple_stops_no_interpolation(void) {
    GraphColorStop stops[] = {
        {.value = 0, .color = GColorBlue},
        {.value = 50, .color = GColorGreen},
        {.value = 100, .color = GColorRed}
    };
    GraphSeriesConfig config = {
        .default_color = GColorBlack,
        .color_stops = stops,
        .color_stop_count = 3,
        .interpolate_color_stops = false,
    };

    TEST_ASSERT_EQUAL_GCOLOR(GColorBlue, graph_color_for_stops(&config, -10, 0, 0, false));
    TEST_ASSERT_EQUAL_GCOLOR(GColorBlue, graph_color_for_stops(&config, 0, 0, 0, false));
    TEST_ASSERT_EQUAL_GCOLOR(GColorBlue, graph_color_for_stops(&config, 49, 0, 0, false));
    TEST_ASSERT_EQUAL_GCOLOR(GColorGreen, graph_color_for_stops(&config, 50, 0, 0, false));
    TEST_ASSERT_EQUAL_GCOLOR(GColorGreen, graph_color_for_stops(&config, 99, 0, 0, false));
    TEST_ASSERT_EQUAL_GCOLOR(GColorRed, graph_color_for_stops(&config, 100, 0, 0, false));
    TEST_ASSERT_EQUAL_GCOLOR(GColorRed, graph_color_for_stops(&config, 110, 0, 0, false));
}

void test_graph_color_for_stops_multiple_stops_with_interpolation(void) {
    GraphColorStop stops[] = {
        {.value = 0, .color = GColorBlack}, // 0b11000000
        {.value = 100, .color = GColorWhite} // 0b11111111
    };
    GraphSeriesConfig config = {
        .default_color = GColorBlack,
        .color_stops = stops,
        .color_stop_count = 2,
        .interpolate_color_stops = true,
    };

    TEST_ASSERT_EQUAL_GCOLOR(GColorBlack, graph_color_for_stops(&config, -10, 0, 0, false));
    TEST_ASSERT_EQUAL_GCOLOR(GColorBlack, graph_color_for_stops(&config, 0, 0, 0, false));
    TEST_ASSERT_EQUAL_GCOLOR((GColor){.argb = 0b11010101}, graph_color_for_stops(&config, 50, 0, 0, false)); // Interpolated mid
    TEST_ASSERT_EQUAL_GCOLOR(GColorWhite, graph_color_for_stops(&config, 100, 0, 0, false));
    TEST_ASSERT_EQUAL_GCOLOR(GColorWhite, graph_color_for_stops(&config, 110, 0, 0, false));
}

// --- Test graph_color_for_value ---

void test_graph_color_for_value_default_color(void) {
    GraphSeriesConfig config = {
        .default_color = GColorBlack,
        .color_stops = NULL,
        .color_stop_count = 0,
    };
    TEST_ASSERT_EQUAL_GCOLOR(GColorBlack, graph_color_for_value(&config, 10, 0, 100, 0, 0, false));
}

// --- Test draw_square_dot ---

void test_draw_square_dot_basic(void) {
    GContext *ctx = (GContext *) 1;
    GRect bounds = GRect(0, 0, 100, 100);
    draw_square_dot(ctx, bounds, 10, 20, 5, GColorRed);

    TEST_ASSERT_EQUAL(1, get_graphics_fill_rect_call_count());
    GraphicsFillRectCall *call = get_graphics_fill_rect_calls();
    TEST_ASSERT_TRUE(GRect_equal(call[0].rect, GRect(10, 20, 5, 5)));
    TEST_ASSERT_EQUAL_GCOLOR(GColorRed, call[0].fill_color);
    TEST_ASSERT_EQUAL(0, call[0].corner_radius);
    TEST_ASSERT_EQUAL(GCornerNone, call[0].corners);
}

void test_draw_square_dot_out_of_bounds(void) {
    GContext *ctx = (GContext *) 1;
    GRect bounds = GRect(0, 0, 10, 10);
    draw_square_dot(ctx, bounds, 8, 8, 5, GColorRed); // Partially out
    draw_square_dot(ctx, bounds, 10, 10, 1, GColorRed); // Exactly on edge
    draw_square_dot(ctx, bounds, -1, 0, 1, GColorRed); // Out left
    draw_square_dot(ctx, bounds, 0, -1, 1, GColorRed); // Out top
    draw_square_dot(ctx, bounds, 10, 0, 1, GColorRed); // Out right
    draw_square_dot(ctx, bounds, 0, 10, 1, GColorRed); // Out bottom

    TEST_ASSERT_EQUAL(0, get_graphics_fill_rect_call_count()); // Should not draw
}

void test_draw_square_dot_clear_color(void) {
    GContext *ctx = (GContext *) 1;
    GRect bounds = GRect(0, 0, 100, 100);
    draw_square_dot(ctx, bounds, 10, 20, 5, GColorClear);

    TEST_ASSERT_EQUAL(0, get_graphics_fill_rect_call_count()); // Should not draw
}

// --- Test graph_instance_draw ---

void test_graph_instance_draw_null_inputs(void) {
    GContext *ctx = (GContext *) 1;
    GRect bounds = GRect(0, 0, 100, 100);

    graph_instance_draw(NULL, ctx, bounds);

    GraphInstance inst = {0};
    graph_instance_draw(&inst, ctx, bounds);

    TEST_ASSERT_EQUAL(0, get_graphics_fill_rect_call_count());
    TEST_ASSERT_EQUAL(0, get_graphics_draw_line_call_count());
}

void test_graph_instance_draw_points_basic(void) {
    GContext *ctx = (GContext *) 1;
    GRect bounds = GRect(0, 0, 100, 100);
    int values[] = {10, 50, 90};
    GraphDataSeries data = { .values = values, .value_count = 3 };

    GraphSeriesConfig s_config = {
        .graph_type = GRAPH_TYPE_POINTS,
        .dot_size = 2,
        .default_color = GColorGreen,
        .min_interpolated_dot_distance_px = 1 // Explicitly set to match expected 67 calls
    };

    GraphDrawConfig d_config = {
        .series = &s_config,
        .series_count = 1,
        .axis = {0}
    };

    GraphInstance instance;
    graph_instance_init(&instance, &data, 1, &d_config);

    graph_instance_draw(&instance, ctx, bounds);

    // Expected 67 calls based on previous run with min_interpolated_dot_distance_px = 1
    TEST_ASSERT_EQUAL(67, get_graphics_fill_rect_call_count());
    GraphicsFillRectCall *calls = get_graphics_fill_rect_calls();

    // Verify first point (value 10)
    TEST_ASSERT_TRUE(GRect_equal(calls[0].rect, GRect(0, 98, 2, 2)));
    TEST_ASSERT_EQUAL_GCOLOR(GColorGreen, calls[0].fill_color);
}

void test_graph_instance_draw_points_suppress_zero(void) {
    GContext *ctx = (GContext *) 1;
    GRect bounds = GRect(0, 0, 100, 100);
    int values[] = {10, 0, 20};
    GraphDataSeries data = { .values = values, .value_count = 3 };

    GraphSeriesConfig s_config = {
        .graph_type = GRAPH_TYPE_POINTS,
        .dot_size = 2,
        .default_color = GColorGreen,
        .suppress_exact_zero_value = true,
        .min_interpolated_dot_distance_px = 1 // Explicitly set for consistent interpolation
    };

    GraphDrawConfig d_config = {
        .series = &s_config,
        .series_count = 1,
        .axis = {0}
    };

    GraphInstance instance;
    graph_instance_init(&instance, &data, 1, &d_config);

    graph_instance_draw(&instance, ctx, bounds);

    // Expected 62 calls based on previous run.
    TEST_ASSERT_EQUAL(62, get_graphics_fill_rect_call_count());
}

void test_graph_instance_draw_line_basic(void) {
    GContext *ctx = (GContext *) 1;
    GRect bounds = GRect(0, 0, 100, 100);
    int values[] = {10, 50, 90};
    GraphDataSeries data = { .values = values, .value_count = 3 };

    GraphSeriesConfig s_config = {
        .graph_type = GRAPH_TYPE_LINE,
        .dot_size = 1, // Line graph uses dot_size for line thickness
        .default_color = GColorBlue,
        .min_interpolated_dot_distance_px = 1000 // No interpolation
    };

    GraphDrawConfig d_config = {
        .series = &s_config,
        .series_count = 1,
        .axis = {0}
    };

    GraphInstance instance;
    graph_instance_init(&instance, &data, 1, &d_config);

    graph_instance_draw(&instance, ctx, bounds);

    TEST_ASSERT_EQUAL(0, get_graphics_fill_rect_call_count()); // Expect 0 for now
    TEST_ASSERT_EQUAL(2, get_graphics_draw_line_call_count()); // 2 lines

    GraphicsDrawLineCall *line_calls = get_graphics_draw_line_calls();

    // Line 1: (0, 88 + 0) to (49, 44 + 0)
    TEST_ASSERT_TRUE(GPoint_equal(line_calls[0].p0, GPoint(0, 88)));
    TEST_ASSERT_TRUE(GPoint_equal(line_calls[0].p1, GPoint(49, 44)));
    TEST_ASSERT_EQUAL_GCOLOR(GColorBlue, line_calls[0].stroke_color);

    // Line 2: (49, 44 + 0) to (99, 0 + 0)
    TEST_ASSERT_TRUE(GPoint_equal(line_calls[1].p0, GPoint(49, 44)));
    TEST_ASSERT_TRUE(GPoint_equal(line_calls[1].p1, GPoint(99, 0)));
    TEST_ASSERT_EQUAL_GCOLOR(GColorBlue, line_calls[1].stroke_color);
}

void test_graph_instance_draw_line_interpolation(void) {
    GContext *ctx = (GContext *) 1;
    GRect bounds = GRect(0, 0, 10, 10); // Small bounds to force interpolation
    int values[] = {0, 100};
    GraphDataSeries data = { .values = values, .value_count = 2 };

    GraphSeriesConfig s_config = {
        .graph_type = GRAPH_TYPE_LINE,
        .dot_size = 1,
        .default_color = GColorRed,
        .min_interpolated_dot_distance_px = 1 // Force interpolation
    };

    GraphDrawConfig d_config = {
        .series = &s_config,
        .series_count = 1,
        .axis = {0}
    };

    GraphInstance instance;
    graph_instance_init(&instance, &data, 1, &d_config);

    graph_instance_draw(&instance, ctx, bounds);

    TEST_ASSERT_EQUAL(0, get_graphics_fill_rect_call_count()); // Expect 0
    TEST_ASSERT_EQUAL(9, get_graphics_draw_line_call_count());
}

void test_graph_instance_draw_bar_basic(void) {
    GContext *ctx = (GContext *) 1;
    GRect bounds = GRect(0, 0, 100, 100);
    int values[] = {10, 50, 90};
    GraphDataSeries data = { .values = values, .value_count = 3 };

    GraphSeriesConfig s_config = {
        .graph_type = GRAPH_TYPE_BAR,
        .dot_size = 5,
        .default_color = GColorYellow,
        .min_interpolated_dot_distance_px = 1000 // No interpolation
    };

    GraphDrawConfig d_config = {
        .series = &s_config,
        .series_count = 1,
        .axis = {0}
    };

    GraphInstance instance;
    graph_instance_init(&instance, &data, 1, &d_config);

    graph_instance_draw(&instance, ctx, bounds);

    // Expected 27 calls (2+9+16 for values 10, 50, 90 respectively)
    TEST_ASSERT_EQUAL(27, get_graphics_fill_rect_call_count());
    TEST_ASSERT_EQUAL(0, get_graphics_draw_line_call_count());

    GraphicsFillRectCall *calls = get_graphics_fill_rect_calls();

    // Verify first bar (value 10)
    TEST_ASSERT_TRUE(GRect_equal(calls[0].rect, GRect(0, 85, 5, 5)));
    TEST_ASSERT_TRUE(GRect_equal(calls[1].rect, GRect(0, 91, 5, 5)));
    TEST_ASSERT_EQUAL_GCOLOR(GColorYellow, calls[0].fill_color);
}

void test_graph_instance_draw_bar_negative_values(void) {
    GContext *ctx = (GContext *) 1;
    GRect bounds = GRect(0, 0, 100, 100);
    int values[] = {-10, 0, 10};
    GraphDataSeries data = { .values = values, .value_count = 3 };

    GraphSeriesConfig s_config = {
        .graph_type = GRAPH_TYPE_BAR,
        .dot_size = 5,
        .default_color = GColorMagenta,
        .min_interpolated_dot_distance_px = 1000 // No interpolation
    };

    GraphDrawConfig d_config = {
        .series = &s_config,
        .series_count = 1,
        .axis = {0}
    };

    GraphInstance instance;
    graph_instance_init(&instance, &data, 1, &d_config);

    graph_instance_draw(&instance, ctx, bounds);

    // Expected 18 calls (8+1+9 for values -10, 0, 10 respectively)
    TEST_ASSERT_EQUAL(18, get_graphics_fill_rect_call_count());
}

void test_graph_instance_draw_line_single_point(void) {
    GContext *ctx = (GContext *) 1;
    GRect bounds = GRect(0, 0, 100, 100);
    int values[] = {50};
    GraphDataSeries data = { .values = values, .value_count = 1 };

    GraphSeriesConfig s_config = {
        .graph_type = GRAPH_TYPE_LINE,
        .dot_size = 2,
        .default_color = GColorCyan
    };

    GraphDrawConfig d_config = {
        .series = &s_config,
        .series_count = 1,
        .axis = {0}
    };

    GraphInstance instance;
    graph_instance_init(&instance, &data, 1, &d_config);

    graph_instance_draw(&instance, ctx, bounds);

    TEST_ASSERT_EQUAL(1, get_graphics_fill_rect_call_count()); // Should draw a single dot
    TEST_ASSERT_EQUAL(0, get_graphics_draw_line_call_count());

    GraphicsFillRectCall *call = get_graphics_fill_rect_calls();
    TEST_ASSERT_TRUE(GRect_equal(call[0].rect, GRect(0, 0, 2, 2)));
    TEST_ASSERT_EQUAL_GCOLOR(GColorCyan, call[0].fill_color);
}

void test_graph_instance_draw_line_single_point_suppress_zero(void) {
    GContext *ctx = (GContext *) 1;
    GRect bounds = GRect(0, 0, 100, 100);
    int values[] = {0};
    GraphDataSeries data = { .values = values, .value_count = 1 };

    GraphSeriesConfig s_config = {
        .graph_type = GRAPH_TYPE_LINE,
        .dot_size = 2,
        .default_color = GColorCyan,
        .suppress_exact_zero_value = true
    };

    GraphDrawConfig d_config = {
        .series = &s_config,
        .series_count = 1,
        .axis = {0}
    };

    GraphInstance instance;
    graph_instance_init(&instance, &data, 1, &d_config);

    graph_instance_draw(&instance, ctx, bounds);

    TEST_ASSERT_EQUAL(0, get_graphics_fill_rect_call_count()); // Should draw nothing
    TEST_ASSERT_EQUAL(0, get_graphics_draw_line_call_count());
}

void test_graph_instance_y_axis_scaling(void) {
    GContext *ctx = (GContext *) 1;
    GRect bounds = GRect(0, 0, 100, 100);
    int values[] = {10, 50, 90};
    GraphDataSeries data = { .values = values, .value_count = 3 };

    int scale_steps[] = {20, 50, 100, 200};

    GraphYAxisScalingConfig scale_config = {
        .has_y_axis_range = true,
        .y_min = 0,
        .y_max = 0,
        .y_axis_max_scale_steps = scale_steps,
        .y_axis_max_scale_step_count = 4
    };

    GraphSeriesConfig s_config = {
        .graph_type = GRAPH_TYPE_LINE,
        .dot_size = 1,
        .default_color = GColorBlue,
        .min_interpolated_dot_distance_px = 1000,
        .y_axis_scaling = &scale_config
    };

    GraphDrawConfig d_config = {
        .series = &s_config,
        .series_count = 1,
        .axis = {0}
    };

    GraphInstance instance;
    graph_instance_init(&instance, &data, 1, &d_config);

    graph_instance_draw(&instance, ctx, bounds);

    // Because max is 90, scale should snap to 100.
    // min is 0 (from y_min)
    // 10 -> (10-0)/(100-0) * 99 = 9, y = 99 - 9 = 90
    // 50 -> (50-0)/(100-0) * 99 = 49.5 (49), y = 99 - 49 = 50
    // 90 -> (90-0)/(100-0) * 99 = 89.1 (89), y = 99 - 89 = 10

    TEST_ASSERT_EQUAL(2, get_graphics_draw_line_call_count());

    GraphicsDrawLineCall *line_calls = get_graphics_draw_line_calls();

    TEST_ASSERT_TRUE(GPoint_equal(line_calls[0].p0, GPoint(0, 90)));
    TEST_ASSERT_TRUE(GPoint_equal(line_calls[0].p1, GPoint(49, 50)));
    TEST_ASSERT_EQUAL_GCOLOR(GColorBlue, line_calls[0].stroke_color);

    TEST_ASSERT_TRUE(GPoint_equal(line_calls[1].p0, GPoint(49, 50)));
    TEST_ASSERT_TRUE(GPoint_equal(line_calls[1].p1, GPoint(99, 10)));
    TEST_ASSERT_EQUAL_GCOLOR(GColorBlue, line_calls[1].stroke_color);
}

void test_graph_instance_draw_axis_ticks(void) {
    GContext *ctx = (GContext *) 1;
    GRect bounds = GRect(0, 0, 100, 100);
    int values[] = {10, 50, 90, 40, 20}; // 5 points
    GraphDataSeries data = { .values = values, .value_count = 5 };

    GraphSeriesConfig s_config = {
        .graph_type = GRAPH_TYPE_LINE,
        .dot_size = 2,
        .default_color = GColorBlue,
        .min_interpolated_dot_distance_px = 1000 // no interpolation
    };

    GraphDrawConfig d_config = {
        .series = &s_config,
        .series_count = 1,
        .axis = {
            .tick_interval_x = 2,
            .tick_color_x = GColorRed,
            .tick_length_y = 5
        }
    };

    GraphInstance instance;
    graph_instance_init(&instance, &data, 1, &d_config);

    graph_instance_draw(&instance, ctx, bounds);

    // value_count = 5, tick_interval = 2 -> i=0, 2, 4 (3 ticks)
    // + 4 lines from the series
    TEST_ASSERT_EQUAL(7, get_graphics_draw_line_call_count());

    GraphicsDrawLineCall *line_calls = get_graphics_draw_line_calls();

    // Ticks are drawn first
    TEST_ASSERT_TRUE(GPoint_equal(line_calls[0].p0, GPoint(1, 100)));
    TEST_ASSERT_TRUE(GPoint_equal(line_calls[0].p1, GPoint(1, 95)));
    TEST_ASSERT_EQUAL_GCOLOR(GColorRed, line_calls[0].stroke_color);

    TEST_ASSERT_TRUE(GPoint_equal(line_calls[1].p0, GPoint(50, 100)));
    TEST_ASSERT_TRUE(GPoint_equal(line_calls[1].p1, GPoint(50, 95)));
    TEST_ASSERT_EQUAL_GCOLOR(GColorRed, line_calls[1].stroke_color);

    TEST_ASSERT_TRUE(GPoint_equal(line_calls[2].p0, GPoint(99, 100)));
    TEST_ASSERT_TRUE(GPoint_equal(line_calls[2].p1, GPoint(99, 95)));
    TEST_ASSERT_EQUAL_GCOLOR(GColorRed, line_calls[2].stroke_color);
}

int main() {
    UNITY_BEGIN();

    // Test static helper functions
    RUN_TEST(test_clamp_int);
    RUN_TEST(test_channel2_to_8);
    RUN_TEST(test_channel8_to_2);
    RUN_TEST(test_interpolate_color);
    RUN_TEST(test_compute_min_max);
    RUN_TEST(test_graph_value_to_y);
    RUN_TEST(test_x_for_index);
    RUN_TEST(test_graph_y_to_value);
    RUN_TEST(test_interpolation_steps_for_segment);
    RUN_TEST(test_smoothstep_interpolate);
    RUN_TEST(test_should_suppress_value);

    // Test graph_color_for_stops
    RUN_TEST(test_graph_color_for_stops_no_stops);
    RUN_TEST(test_graph_color_for_stops_single_stop);
    RUN_TEST(test_graph_color_for_stops_multiple_stops_no_interpolation);
    RUN_TEST(test_graph_color_for_stops_multiple_stops_with_interpolation);

    // Test graph_color_for_value
    RUN_TEST(test_graph_color_for_value_default_color);

    // Test draw_square_dot
    RUN_TEST(test_draw_square_dot_basic);
    RUN_TEST(test_draw_square_dot_out_of_bounds);
    RUN_TEST(test_draw_square_dot_clear_color);

    // Test graph_instance_draw
    RUN_TEST(test_graph_instance_draw_null_inputs);
    RUN_TEST(test_graph_instance_draw_points_basic);
    RUN_TEST(test_graph_instance_draw_points_suppress_zero);
    RUN_TEST(test_graph_instance_draw_line_basic);
    RUN_TEST(test_graph_instance_draw_line_interpolation);
    RUN_TEST(test_graph_instance_draw_bar_basic);
    RUN_TEST(test_graph_instance_draw_bar_negative_values);
    RUN_TEST(test_graph_instance_draw_line_single_point);
    RUN_TEST(test_graph_instance_draw_line_single_point_suppress_zero);
    RUN_TEST(test_graph_instance_y_axis_scaling);
    RUN_TEST(test_graph_instance_draw_axis_ticks);

    return UNITY_END();
}
