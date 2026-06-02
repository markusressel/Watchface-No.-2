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

static GColor interpolate_color(const GColor from, const GColor to, const int t_scaled) {
#if defined(PBL_COLOR)
    const int t = clamp_int(t_scaled, 0, 1000);
    const int from_a = (from.argb >> 6) & 0x3;
    const int from_r = (from.argb >> 4) & 0x3;
    const int from_g = (from.argb >> 2) & 0x3;
    const int from_b = from.argb & 0x3;

    const int to_a = (to.argb >> 6) & 0x3;
    const int to_r = (to.argb >> 4) & 0x3;
    const int to_g = (to.argb >> 2) & 0x3;
    const int to_b = to.argb & 0x3;

    const int out_a = from_a + ((to_a - from_a) * t) / 1000;
    const int out_r = from_r + ((to_r - from_r) * t) / 1000;
    const int out_g = from_g + ((to_g - from_g) * t) / 1000;
    const int out_b = from_b + ((to_b - from_b) * t) / 1000;

    GColor out = GColorBlack;
    out.argb = (uint8_t) ((out_a << 6) | (out_r << 4) | (out_g << 2) | out_b);
    return out;
#else
    (void) to;
    (void) t_scaled;
    return from;
#endif
}

static GColor graph_color_for_stops(
    const GraphDrawConfig *config,
    const int value
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

    for (int i = 0; i < last; i++) {
        const GraphColorStop left = config->color_stops[i];
        const GraphColorStop right = config->color_stops[i + 1];
        if (value < left.value || value > right.value) {
            continue;
        }

        if (!config->interpolate_color_stops || right.value <= left.value) {
            return left.color;
        }

        const int t = ((value - left.value) * 1000) / (right.value - left.value);
        return interpolate_color(left.color, right.color, t);
    }

    return config->default_color;
}

static GColor graph_color_for_value(
    const GraphDrawConfig *config,
    const int value,
    const int min_value,
    const int max_value
) {
    if (config->color_stops && config->color_stop_count > 0) {
        return graph_color_for_stops(config, value);
    }

    if (config->color_for_value) {
        return config->color_for_value(value, min_value, max_value, config->color_context);
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

static void draw_line_fill_column(
    GContext *ctx,
    const GRect bounds,
    const int x,
    const int y_start,
    const int min_value,
    const int max_value,
    const int drawable_height,
    const GraphDrawConfig *config
) {
    if (!config->fill_area_under_line || x < 0 || x >= bounds.size.w) {
        return;
    }

    int y = clamp_int(y_start, 0, drawable_height);
    while (y <= drawable_height) {
        const int value_at_y = graph_y_to_value(y, min_value, max_value, drawable_height);
        const GColor color = graph_color_for_value(config, value_at_y, min_value, max_value);
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
    const GraphDrawConfig *config
) {
    const int step_size = dot_size + 1;
    const int drawable_height = bounds.size.h - dot_size;
    if (drawable_height < 0) {
        return;
    }

    const int y_zero = graph_value_to_y(0, min_value, max_value, drawable_height);
    const int y_value = graph_value_to_y(value, min_value, max_value, drawable_height);
    const int y_start = y_zero < y_value ? y_zero : y_value;
    const int y_end = y_zero > y_value ? y_zero : y_value;
    const GColor color = graph_color_for_value(config, value, min_value, max_value);

    for (int y = y_start; y <= y_end; y += step_size) {
        draw_square_dot(ctx, bounds, x, y, dot_size, color);
    }
}

void graph_draw_series(
    GContext *ctx,
    const GRect bounds,
    const int *values,
    const int value_count,
    const GraphDrawConfig *config
) {
    if (!ctx || !values || !config || value_count <= 0 || bounds.size.w <= 0 || bounds.size.h <= 0) {
        return;
    }

    const int dot_size = config->dot_size > 1 ? config->dot_size : 1;
    const int min_interpolated_dot_distance_px = config->min_interpolated_dot_distance_px >= 0
                                                     ? config->min_interpolated_dot_distance_px
                                                     : 2;

    int min_value = 0;
    int max_value = 0;
    compute_min_max(values, value_count, &min_value, &max_value);

    // Bar and line graphs should keep a true zero baseline so positive values
    // grow upward from zero and negative values grow downward from zero.
    if (config->graph_type == GRAPH_TYPE_BAR || config->graph_type == GRAPH_TYPE_LINE) {
        if (min_value > 0) {
            min_value = 0;
        }
        if (max_value < 0) {
            max_value = 0;
        }
        if (max_value == min_value) {
            max_value = min_value + 1;
        }
    }

    int drawable_height = bounds.size.h - dot_size;
    if (drawable_height < 1) {
        drawable_height = 1;
    }

    if (config->graph_type == GRAPH_TYPE_LINE) {
        bool has_previous = false;
        GPoint previous_point = GPoint(0, 0);
        int previous_value = 0;

        for (int i = 0; i < value_count - 1; i++) {
            const int x0 = x_for_index(i, value_count, bounds.size.w, dot_size);
            const int x1 = x_for_index(i + 1, value_count, bounds.size.w, dot_size);
            const int v0 = values[i];
            const int v1 = values[i + 1];
            const int interpolation_steps = interpolation_steps_for_segment(
                x0,
                x1,
                min_interpolated_dot_distance_px
            );

            for (int step = 0; step <= interpolation_steps; step++) {
                const int x = x0 + ((x1 - x0) * step) / (interpolation_steps + 1);
                const int value = smoothstep_interpolate(v0, v1, step, interpolation_steps + 1);
                const int y = graph_value_to_y(value, min_value, max_value, drawable_height);
                const GPoint point = GPoint(x, y + (dot_size / 2));

                draw_line_fill_column(
                    ctx,
                    bounds,
                    x,
                    y,
                    min_value,
                    max_value,
                    drawable_height,
                    config
                );

                if (has_previous) {
                    const int mid_value = (value + previous_value) / 2;
                    graphics_context_set_stroke_color(ctx, graph_color_for_value(config, mid_value, min_value, max_value));
                    graphics_draw_line(ctx, previous_point, point);
                }

                previous_point = point;
                previous_value = value;
                has_previous = true;
            }
        }

        if (value_count == 1) {
            const int y = graph_value_to_y(values[0], min_value, max_value, drawable_height);
            draw_square_dot(
                ctx,
                bounds,
                0,
                y,
                dot_size,
                graph_color_for_value(config, values[0], min_value, max_value)
            );
            return;
        }

        const int x_last = x_for_index(value_count - 1, value_count, bounds.size.w, dot_size);
        const int y_last = graph_value_to_y(values[value_count - 1], min_value, max_value, drawable_height);
        const GPoint final_point = GPoint(x_last, y_last + (dot_size / 2));

        draw_line_fill_column(
            ctx,
            bounds,
            x_last,
            y_last,
            min_value,
            max_value,
            drawable_height,
            config
        );

        if (has_previous) {
            graphics_context_set_stroke_color(
                ctx,
                graph_color_for_value(config, values[value_count - 1], min_value, max_value)
            );
            graphics_draw_line(ctx, previous_point, final_point);
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
        draw_square_dot(
            ctx,
            bounds,
            x,
            y,
            dot_size,
            graph_color_for_value(config, value, min_value, max_value)
        );

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
                const int y_interp = graph_value_to_y(value_interp, min_value, max_value, drawable_height);
                draw_square_dot(
                    ctx,
                    bounds,
                    x_interp,
                    y_interp,
                    1,
                    graph_color_for_value(config, value_interp, min_value, max_value)
                );
            }
        }
    }
}

