#include "graph_utils.h"

static int clamp_int(const int value, const int min_value, const int max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

#if defined(PBL_COLOR)
static int channel2_to_8(const int value2) {
    return clamp_int(value2, 0, 3) * 85;
}

static int channel8_to_2(const int value8) {
    const int clamped = clamp_int(value8, 0, 255);
    // Round to the nearest Pebble 2-bit channel bucket.
    return (clamped + 42) / 85;
}

static const uint8_t s_bayer_matrix_8x8[8][8] = {
    {0, 32, 8, 40, 2, 34, 10, 42},
    {48, 16, 56, 24, 50, 18, 58, 26},
    {12, 44, 4, 36, 14, 46, 6, 38},
    {60, 28, 52, 20, 62, 30, 54, 22},
    {3, 35, 11, 43, 1, 33, 9, 41},
    {51, 19, 59, 27, 49, 17, 57, 25},
    {15, 47, 7, 39, 13, 45, 5, 37},
    {63, 31, 55, 23, 61, 29, 53, 21}
};
#endif

static GColor interpolate_color(const GColor from, const GColor to, const int t_scaled, bool dither, int x, int y) {
#if defined(PBL_COLOR)
    int dither_offset = 0;
    if (dither) {
        dither_offset = (s_bayer_matrix_8x8[y % 8][x % 8] * 85) / 64 - 42;
    }

    const int t = clamp_int(t_scaled, 0, 1000);
    const int from_a = channel2_to_8((from.argb >> 6) & 0x3);
    const int from_r = channel2_to_8((from.argb >> 4) & 0x3);
    const int from_g = channel2_to_8((from.argb >> 2) & 0x3);
    const int from_b = channel2_to_8(from.argb & 0x3);

    const int to_a = channel2_to_8((to.argb >> 6) & 0x3);
    const int to_r = channel2_to_8((to.argb >> 4) & 0x3);
    const int to_g = channel2_to_8((to.argb >> 2) & 0x3);
    const int to_b = channel2_to_8(to.argb & 0x3);

    const int out_a = from_a + ((to_a - from_a) * t) / 1000;
    const int out_r = from_r + ((to_r - from_r) * t) / 1000;
    const int out_g = from_g + ((to_g - from_g) * t) / 1000;
    const int out_b = from_b + ((to_b - from_b) * t) / 1000;

    GColor out = GColorBlack;
    out.argb = (uint8_t) (
        (channel8_to_2(out_a + dither_offset) << 6) |
        (channel8_to_2(out_r + dither_offset) << 4) |
        (channel8_to_2(out_g + dither_offset) << 2) |
        (channel8_to_2(out_b + dither_offset))
    );
    return out;
#else
    (void) to;
    (void) t_scaled;
    (void) dither;
    (void) x;
    (void) y;
    return from;
#endif
}

static GColor graph_color_for_stops(
    const GraphSeriesConfig *config,
    const int value,
    int x,
    int y,
    bool dither
) {
    if (!config->color_stops || config->color_stop_count <= 0) {
        return config->default_color;
    }

    if (config->color_stop_count == 1) {
        return config->color_stops[0].color;
    }

    if (value <= config->color_stops[0].value) {
        return config->color_stops[0].color;
    }

    const int last = config->color_stop_count - 1;
    if (value >= config->color_stops[last].value) {
        return config->color_stops[last].color;
    }

    // Exact stop value should always resolve to the exact configured color.
    for (int s = 0; s <= last; s++) {
        if (value == config->color_stops[s].value) {
            return config->color_stops[s].color;
        }
    }

    for (int i = 0; i < last; i++) {
        const GraphColorStop left = config->color_stops[i];
        const GraphColorStop right = config->color_stops[i + 1];
        if (value <= left.value || value >= right.value) {
            continue;
        }

        if (!config->interpolate_color_stops || right.value <= left.value) {
            return left.color;
        }

        const int t = ((value - left.value) * 1000) / (right.value - left.value);
        return interpolate_color(left.color, right.color, t, dither, x, y);
    }

    return config->default_color;
}

static GColor graph_color_for_value(
    const GraphSeriesConfig *config,
    const int value,
    const int min_value,
    const int max_value,
    int x,
    int y,
    bool dither
) {
    if (config->color_stops && config->color_stop_count > 0) {
        return graph_color_for_stops(config, value, x, y, dither);
    }

    return config->default_color;
}

static void draw_square_dot(
    GContext *ctx,
    const GRect bounds,
    const int x,
    const int y,
    const int dot_size,
    const GColor color
) {
    if (x < 0 || y < 0 || x + dot_size > bounds.size.w || y + dot_size > bounds.size.h) {
        return;
    }

    // Use gcolor_equal to compare unions
    if (gcolor_equal(color, GColorClear)) {
        return;
    }

    graphics_context_set_fill_color(ctx, color);
    graphics_fill_rect(ctx, GRect(x, y, dot_size, dot_size), 0, GCornerNone);
}

static int graph_value_to_y(
    const int value,
    const int min_value,
    const int max_value,
    const int drawable_height
) {
    if (max_value <= min_value) {
        return drawable_height / 2;
    }

    const int normalized = (value - min_value) * drawable_height / (max_value - min_value);
    return drawable_height - normalized;
}

static void compute_min_max(const int *values, const int value_count, int *out_min, int *out_max) {
    int min_value = values[0];
    int max_value = values[0];

    for (int i = 1; i < value_count; i++) {
        if (values[i] < min_value) {
            min_value = values[i];
        }
        if (values[i] > max_value) {
            max_value = values[i];
        }
    }

    *out_min = min_value;
    *out_max = max_value;
}

static int smoothstep_interpolate(const int start, const int end, const int step, const int steps_per_segment) {
    if (steps_per_segment <= 0) {
        return start;
    }

    // Smooth interpolation between two points using smoothstep (cubic Hermite).
    const int t_scaled = (step * 1000) / steps_per_segment;
    const int t2 = (t_scaled * t_scaled) / 1000;
    const int smooth = (t2 * (3000 - (2 * t_scaled))) / 1000;
    return start + ((end - start) * smooth) / 1000;
}

static int x_for_index(
    const int index,
    const int total,
    const int width,
    const int dot_size
) {
    if (total <= 1) {
        return 0;
    }

    return (index * (width - dot_size)) / (total - 1);
}

static int graph_y_to_value(
    const int y,
    const int min_value,
    const int max_value,
    const int drawable_height
) {
    if (max_value <= min_value || drawable_height <= 0) {
        return min_value;
    }

    const int y_clamped = clamp_int(y, 0, drawable_height);
    return max_value - ((y_clamped * (max_value - min_value)) / drawable_height);
}

static int interpolation_steps_for_segment(const int x_start, const int x_end, const int min_distance_px) {
    const int distance = x_end >= x_start ? (x_end - x_start) : (x_start - x_end);
    const int spacing = min_distance_px > 0 ? min_distance_px : 1;

    if (distance < spacing * 2) {
        return 0;
    }

    int steps = (distance / spacing) - 1;
    return clamp_int(steps, 0, 32);
}

static bool should_suppress_value(const GraphSeriesConfig *config, const int value) {
    return config->suppress_exact_zero_value && value == 0;
}

static void draw_line_fill_column(
    GContext *ctx,
    const GRect bounds,
    const int x,
    const int y_start,
    const int min_value,
    const int max_value,
    const int drawable_height,
    const GraphSeriesConfig *config
) {
    if (!config->fill_area_under_line || x < 0 || x >= bounds.size.w) {
        return;
    }

    if (should_suppress_value(config, graph_y_to_value(y_start, min_value, max_value, drawable_height))) {
        return;
    }

    int y = clamp_int(y_start, 0, drawable_height);

    while (y <= drawable_height) {
        const int value_at_y = graph_y_to_value(y, min_value, max_value, drawable_height);
        const GColor color = graph_color_for_value(config, value_at_y, min_value, max_value, x, y, config->dither_fill_colors);
        draw_square_dot(ctx, bounds, x, y, 1, color);
        y++;
    }
}

static void draw_bar_at_x(
    GContext *ctx,
    const GRect bounds,
    const int x,
    const int value,
    const int min_value,
    const int max_value,
    const int dot_size,
    const GraphSeriesConfig *config
) {
    if (should_suppress_value(config, value)) {
        return;
    }

    const int step_size = dot_size + 1;
    const int drawable_height = bounds.size.h - dot_size;
    if (drawable_height < 0) {
        return;
    }

    const int y_zero = graph_value_to_y(0, min_value, max_value, drawable_height);
    const int y_value = graph_value_to_y(value, min_value, max_value, drawable_height);
    const int y_start = y_zero < y_value ? y_zero : y_value;
    const int y_end = y_zero > y_value ? y_zero : y_value;

    for (int y = y_start; y <= y_end; y += step_size) {
        const GColor color = graph_color_for_value(config, value, min_value, max_value, x, y, config->dither_fill_colors);
        draw_square_dot(ctx, bounds, x, y, dot_size, color);
    }
}

static void draw_single_series(
    GContext *ctx,
    const GRect bounds,
    const int *values,
    const int value_count,
    const GraphSeriesConfig *config,
    const int min_value,
    const int max_value
) {
    if (!ctx || !values || !config || value_count <= 0 || bounds.size.w <= 0 || bounds.size.h <= 0) {
        return;
    }

    const int dot_size = config->dot_size > 1 ? config->dot_size : 1;
    const int min_interpolated_dot_distance_px = config->min_interpolated_dot_distance_px >= 0
                                                     ? config->min_interpolated_dot_distance_px
                                                     : 2;

    int drawable_height = bounds.size.h - dot_size;
    if (drawable_height < 1) {
        drawable_height = 1;
    }

    if (config->graph_type == GRAPH_TYPE_LINE) {
        // Pass 1: Filled Area (drawn first, below the line)
        if (config->fill_area_under_line) {
            for (int i = 0; i < value_count - 1; i++) {
                const int x0 = x_for_index(i, value_count, bounds.size.w, dot_size);
                const int x1 = x_for_index(i + 1, value_count, bounds.size.w, dot_size);
                const int v0 = values[i];
                const int v1 = values[i + 1];
                const int interpolation_steps = interpolation_steps_for_segment(
                    x0, x1, min_interpolated_dot_distance_px
                );

                for (int step = 0; step <= interpolation_steps; step++) {
                    const int x = x0 + ((x1 - x0) * step) / (interpolation_steps + 1);
                    const int value = smoothstep_interpolate(v0, v1, step, interpolation_steps + 1);
                    const int y = graph_value_to_y(value, min_value, max_value, drawable_height);
                    const bool drawable = !should_suppress_value(config, value);

                    if (drawable) {
                        draw_line_fill_column(ctx, bounds, x, y, min_value, max_value, drawable_height, config);
                    }
                }
            }

            if (value_count > 1) {
                const int x_last = x_for_index(value_count - 1, value_count, bounds.size.w, dot_size);
                const int last_value = values[value_count - 1];
                const int y_last = graph_value_to_y(last_value, min_value, max_value, drawable_height);
                const bool final_drawable = !should_suppress_value(config, last_value);

                if (final_drawable) {
                    draw_line_fill_column(ctx, bounds, x_last, y_last, min_value, max_value, drawable_height, config);
                }
            }
        }

        // Pass 2: Line Stroke and Single Point
        if (value_count == 1) {
            if (!should_suppress_value(config, values[0])) {
                const int y = graph_value_to_y(values[0], min_value, max_value, drawable_height);
                draw_square_dot(
                    ctx, bounds, 0, y, dot_size,
                    graph_color_for_value(config, values[0], min_value, max_value, 0, y, false)
                );
            }
            return;
        }

        bool has_previous = false;
        bool previous_drawable = false;
        GPoint previous_point = GPoint(0, 0);
        int previous_value = 0;

        for (int i = 0; i < value_count - 1; i++) {
            const int x0 = x_for_index(i, value_count, bounds.size.w, dot_size);
            const int x1 = x_for_index(i + 1, value_count, bounds.size.w, dot_size);
            const int v0 = values[i];
            const int v1 = values[i + 1];
            const int interpolation_steps = interpolation_steps_for_segment(
                x0, x1, min_interpolated_dot_distance_px
            );

            for (int step = 0; step <= interpolation_steps; step++) {
                const int x = x0 + ((x1 - x0) * step) / (interpolation_steps + 1);
                const int value = smoothstep_interpolate(v0, v1, step, interpolation_steps + 1);
                const int y = graph_value_to_y(value, min_value, max_value, drawable_height);
                const GPoint point = GPoint(x, y + (dot_size / 2));
                const bool drawable = !should_suppress_value(config, value);

                if (has_previous && previous_drawable && drawable) {
                    const int mid_value = (value + previous_value) / 2;
                    // Draw lines without dithering so the stroke remains solid
                    const GColor stroke_color = graph_color_for_value(config, mid_value, min_value, max_value, (point.x + previous_point.x) / 2, (point.y + previous_point.y) / 2,
                                                                      false);
                    if (!gcolor_equal(stroke_color, GColorClear)) {
                        graphics_context_set_stroke_color(ctx, stroke_color);
                        graphics_draw_line(ctx, previous_point, point);
                    }
                }

                previous_point = point;
                previous_value = value;
                previous_drawable = drawable;
                has_previous = true;
            }
        }

        const int x_last = x_for_index(value_count - 1, value_count, bounds.size.w, dot_size);
        const int last_value = values[value_count - 1];
        const int y_last = graph_value_to_y(last_value, min_value, max_value, drawable_height);
        const GPoint final_point = GPoint(x_last, y_last + (dot_size / 2));
        const bool final_drawable = !should_suppress_value(config, last_value);

        if (has_previous && previous_drawable && final_drawable) {
            const GColor final_stroke_color = graph_color_for_value(
                config, last_value, min_value, max_value, final_point.x, final_point.y, false
            );
            if (!gcolor_equal(final_stroke_color, GColorClear)) {
                graphics_context_set_stroke_color(ctx, final_stroke_color);
                graphics_draw_line(ctx, previous_point, final_point);
            }
        }

        return;
    }

    for (int i = 0; i < value_count; i++) {
        const int x = x_for_index(i, value_count, bounds.size.w, dot_size);
        const int value = values[i];

        if (config->graph_type == GRAPH_TYPE_BAR) {
            draw_bar_at_x(ctx, bounds, x, value, min_value, max_value, dot_size, config);

            if (i < value_count - 1) {
                const int x_next = x_for_index(i + 1, value_count, bounds.size.w, dot_size);
                const int next_value = values[i + 1];
                const int interpolation_steps = interpolation_steps_for_segment(
                    x,
                    x_next,
                    min_interpolated_dot_distance_px
                );

                for (int step = 1; step <= interpolation_steps; step++) {
                    const int x_interp = x + ((x_next - x) * step) / (interpolation_steps + 1);
                    const int value_interp = smoothstep_interpolate(value, next_value, step, interpolation_steps + 1);
                    draw_bar_at_x(ctx, bounds, x_interp, value_interp, min_value, max_value, 1, config);
                }
            }

            continue;
        }

        const int y = graph_value_to_y(value, min_value, max_value, drawable_height);
        if (!should_suppress_value(config, value)) {
            draw_square_dot(
                ctx,
                bounds,
                x,
                y,
                dot_size,
                graph_color_for_value(config, value, min_value, max_value, x, y, false)
            );
        }

        if (i < value_count - 1) {
            const int x_next = x_for_index(i + 1, value_count, bounds.size.w, dot_size);
            const int next_value = values[i + 1];
            const int interpolation_steps = interpolation_steps_for_segment(
                x,
                x_next,
                min_interpolated_dot_distance_px
            );

            for (int step = 1; step <= interpolation_steps; step++) {
                const int x_interp = x + ((x_next - x) * step) / (interpolation_steps + 1);
                const int value_interp = smoothstep_interpolate(value, next_value, step, interpolation_steps + 1);
                if (should_suppress_value(config, value_interp)) {
                    continue;
                }
                const int y_interp = graph_value_to_y(value_interp, min_value, max_value, drawable_height);
                draw_square_dot(
                    ctx,
                    bounds,
                    x_interp,
                    y_interp,
                    1,
                    graph_color_for_value(config, value_interp, min_value, max_value, x_interp, y_interp, false)
                );
            }
        }
    }
}

void graph_instance_init(GraphInstance *instance, GraphDataSeries *data, const int data_count, const GraphDrawConfig *config) {
    if (!instance) return;
    instance->data = data;
    instance->data_count = data_count;
    if (config) {
        instance->config = *config;
    } else {
        instance->config.series = NULL;
        instance->config.series_count = 0;
        instance->config.axis.tick_interval_x = 0;
        instance->config.axis.tick_color_x = GColorClear;
        instance->config.axis.tick_length_y = 0;
    }
}

void graph_instance_draw(const GraphInstance *instance, GContext *ctx, const GRect bounds) {
    if (!instance || instance->data_count <= 0 || !instance->config.series || instance->config.series_count <= 0) return;

    // Determine max value_count to draw the x-axis properly
    int max_points = 0;
    for (int s = 0; s < instance->data_count; s++) {
        if (instance->data[s].value_count > max_points) {
            max_points = instance->data[s].value_count;
        }
    }

    // Draw x-axis ticks behind the series
    if (max_points > 0 && instance->config.axis.tick_interval_x > 0 && !gcolor_equal(instance->config.axis.tick_color_x, GColorClear)) {
        // Find a representative dot size. We can just take the largest dot size from the series.
        int max_dot_size = 1;
        for (int s = 0; s < instance->config.series_count; s++) {
            if (instance->config.series[s].dot_size > max_dot_size) {
                max_dot_size = instance->config.series[s].dot_size;
            }
        }

        graphics_context_set_stroke_color(ctx, instance->config.axis.tick_color_x);
        for (int i = 0; i < max_points; i++) {
            if ((i % instance->config.axis.tick_interval_x) == 0) {
                int x = x_for_index(i, max_points, bounds.size.w, max_dot_size) + (max_dot_size / 2);
                int y_bottom = bounds.size.h;
                int y_top = y_bottom - instance->config.axis.tick_length_y;
                if (y_top < 0) y_top = 0;
                graphics_draw_line(ctx, GPoint(x, y_bottom), GPoint(x, y_top));
            }
        }
    }

    // Draw all series
    int num_to_draw = instance->data_count < instance->config.series_count ? instance->data_count : instance->config.series_count;
    for (int s = 0; s < num_to_draw; s++) {
        const GraphSeriesConfig *series_cfg = &instance->config.series[s];
        const GraphYAxisScalingConfig *scale_cfg = series_cfg->y_axis_scaling;
        const int *values = instance->data[s].values;
        const int value_count = instance->data[s].value_count;

        int min_value = 0;
        int max_value = 0;

        if (!values || value_count <= 0) continue;

        if (scale_cfg && scale_cfg->has_y_axis_range && (!scale_cfg->y_axis_max_scale_steps || scale_cfg->y_axis_max_scale_step_count <= 0)) {
            min_value = scale_cfg->y_min;
            max_value = scale_cfg->y_max;
        } else {
            compute_min_max(values, value_count, &min_value, &max_value);

            if (scale_cfg && scale_cfg->y_axis_max_scale_steps && scale_cfg->y_axis_max_scale_step_count > 0) {
                int best_step = max_value;
                bool found_step = false;
                for (int i = 0; i < scale_cfg->y_axis_max_scale_step_count; i++) {
                    int step = scale_cfg->y_axis_max_scale_steps[i];
                    if (step >= max_value) {
                        if (!found_step || step < best_step) {
                            best_step = step;
                            found_step = true;
                        }
                    }
                }
                if (!found_step) {
                    best_step = scale_cfg->y_axis_max_scale_steps[0];
                    for (int i = 1; i < scale_cfg->y_axis_max_scale_step_count; i++) {
                        if (scale_cfg->y_axis_max_scale_steps[i] > best_step) {
                            best_step = scale_cfg->y_axis_max_scale_steps[i];
                        }
                    }
                    if (max_value > best_step) {
                        best_step = max_value;
                    }
                }
                max_value = best_step;
            }

            if (scale_cfg && scale_cfg->has_y_axis_range) {
                min_value = scale_cfg->y_min;
            } else {
                if (series_cfg->graph_type == GRAPH_TYPE_BAR || series_cfg->graph_type == GRAPH_TYPE_LINE) {
                    if (min_value > 0) min_value = 0;
                    if (max_value < 0) max_value = 0;
                    if (max_value == min_value) max_value = min_value + 1;
                }
            }
        }

        draw_single_series(ctx, bounds, values, value_count, series_cfg, min_value, max_value);
    }
}
