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

static GColor graph_color_for_value(
    const GraphDrawConfig *config,
    const int value,
    const int min_value,
    const int max_value
) {
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
    const int interpolation_steps = clamp_int(config->interpolation_steps, 0, 16);

    int min_value = 0;
    int max_value = 0;
    compute_min_max(values, value_count, &min_value, &max_value);

    if (config->graph_type == GRAPH_TYPE_BAR && config->bars_from_zero) {
        if (max_value < 1) {
            max_value = 1;
        }
        min_value = 0;
    }

    int drawable_height = bounds.size.h - dot_size;
    if (drawable_height < 1) {
        drawable_height = 1;
    }

    if (config->graph_type == GRAPH_TYPE_LINE) {
        bool has_previous = false;
        GPoint previous_point = GPoint(0, 0);

        for (int i = 0; i < value_count - 1; i++) {
            const int x0 = x_for_index(i, value_count, bounds.size.w, dot_size);
            const int x1 = x_for_index(i + 1, value_count, bounds.size.w, dot_size);
            const int v0 = values[i];
            const int v1 = values[i + 1];

            for (int step = 0; step <= interpolation_steps; step++) {
                const int x = x0 + ((x1 - x0) * step) / (interpolation_steps + 1);
                const int value = smoothstep_interpolate(v0, v1, step, interpolation_steps + 1);
                const int y = graph_value_to_y(value, min_value, max_value, drawable_height);
                const GPoint point = GPoint(x, y + (dot_size / 2));

                if (has_previous) {
                    const int mid_value = (value + v0) / 2;
                    graphics_context_set_stroke_color(ctx, graph_color_for_value(config, mid_value, min_value, max_value));
                    graphics_draw_line(ctx, previous_point, point);
                }

                previous_point = point;
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
            int rows = bounds.size.h / (dot_size + 1);
            if (rows < 1) {
                rows = 1;
            }

            int bar_rows = 0;
            if (max_value > min_value) {
                bar_rows = ((value - min_value) * rows) / (max_value - min_value);
            }
            if (value > min_value && bar_rows == 0) {
                bar_rows = 1;
            }

            const GColor color = graph_color_for_value(config, value, min_value, max_value);
            for (int r = 0; r < bar_rows; r++) {
                const int y = bounds.size.h - dot_size - (r * (dot_size + 1));
                if (y < 0) {
                    break;
                }
                draw_square_dot(ctx, bounds, x, y, dot_size, color);
            }

            if (i < value_count - 1 && interpolation_steps > 0) {
                const int x_next = x_for_index(i + 1, value_count, bounds.size.w, dot_size);
                const int next_value = values[i + 1];
                for (int step = 1; step <= interpolation_steps; step++) {
                    const int x_interp = x + ((x_next - x) * step) / (interpolation_steps + 1);
                    const int value_interp = smoothstep_interpolate(value, next_value, step, interpolation_steps + 1);

                    int interp_rows = 0;
                    if (max_value > min_value) {
                        interp_rows = ((value_interp - min_value) * rows) / (max_value - min_value);
                    }
                    if (value_interp > min_value && interp_rows == 0) {
                        interp_rows = 1;
                    }

                    const GColor interp_color = graph_color_for_value(config, value_interp, min_value, max_value);
                    for (int r = 0; r < interp_rows; r++) {
                        const int y = bounds.size.h - dot_size - (r * (dot_size + 1));
                        if (y < 0) {
                            break;
                        }
                        draw_square_dot(ctx, bounds, x_interp, y, 1, interp_color);
                    }
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

        if (i < value_count - 1 && interpolation_steps > 0) {
            const int x_next = x_for_index(i + 1, value_count, bounds.size.w, dot_size);
            const int next_value = values[i + 1];
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

