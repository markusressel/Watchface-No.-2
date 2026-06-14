#include "battery_bar.h"

#include "dotted_text_layer.h"
#include "../../system/battery.h"
#include "../../ui/theme.h"
#include "../../settings/clay_settings.h"
#include "../../ui/layer_factory.h"
#include "../ui_state.h"

typedef struct {
    uint8_t horizontal_alignment;
    uint8_t vertical_alignment;
    uint8_t rendering_mode;
} BatteryLayerData;

static int s_active_battery_layers = 0;

// temporary variable used for animation (global state - shared by all layers)
static int s_current_battery_level;

// battery charging animation callbacks
static AnimationImplementation battery_charging_anim_impl;

static Animation *charging_animation;

// battery charging animation durations
static int s_battery_charging_animation_duration_ms = 2000;
// delay between a full battery charging animation cycle
static int s_battery_charging_animation_delay_ms = 600;
static const int s_battery_charging_animation_repeat_count = ANIMATION_DURATION_INFINITE;

static float auto_scale_for_height(
    const ClaySettings *settings,
    int available_height,
    DottedTextRenderingMode mode
) {
    const int base_height = (5 * settings->DotHeight) + (4 * settings->DotVerticalGap);
    if (base_height <= 0 || available_height <= 0) {
        return 1.0f;
    }

    float scale = (float) available_height / (float) base_height;
    if (scale <= 0.0f) {
        return 1.0f;
    }

    if (mode == DOTTED_TEXT_RENDERING_MODE_PIXEL_PERFECT) {
        // Calculate the total height with current scale using rounding.
        int scaled_h = (int) ((float) settings->DotHeight * scale + 0.5f);
        int scaled_g = (int) ((float) settings->DotVerticalGap * scale + 0.5f);
        int total_h = 5 * scaled_h + 4 * scaled_g;

        if (total_h > available_height) {
            // Apply safe algebraic lower bound. Max error for 9 segments is 4.5 pixels.
            scale = (float) (available_height - 4.5f) / (float) base_height;
            if (scale <= 0.0f) {
                scale = 0.1f;
            }
        }
    }

    return scale;
}

static void draw_battery_fill(int percent) {
    s_current_battery_level = percent;
    // Mark all battery layers as dirty
    for (int i = 0; i < ui_state_get_row_count(); i++) {
        if (ui_state_get_widget_id(i) == WIDGET_BATTERY_BAR) {
            Layer *layer = ui_state_get_layer(i);
            if (layer) {
                layer_mark_dirty(layer);
            }
        }
    }
}

// update battery charging animation
static void batteryChargingAnimUpdate(Animation *animation, const AnimationProgress progress) {
    const int progress_percent = (progress * 100 / ANIMATION_NORMALIZED_MAX);
    // calculate animation fill rate
    const int fill_percent = s_battery_level + (progress_percent * (100 - s_battery_level) / 100);

    draw_battery_fill(fill_percent);
}

// draw the battery layer
static void battery_update_proc(Layer *layer, GContext *ctx) {
    const GRect bounds = layer_get_bounds(layer);
    BatteryLayerData *data = layer_get_data(layer);

    const HorizontalAlignment horizontal_alignment = data ? data->horizontal_alignment : HORIZONTAL_ALIGN_RIGHT;
    const VerticalAlignment vertical_alignment = data ? data->vertical_alignment : VERTICAL_ALIGN_TOP;
    const DottedTextRenderingMode rendering_mode = data ? data->rendering_mode : DOTTED_TEXT_RENDERING_MODE_SUBPIXEL;

    ClaySettings *settings = clay_get_settings();
    float scale_factor = settings->DotAutoScale
                             ? auto_scale_for_height(settings, bounds.size.h, rendering_mode)
                             : settings->DotScaleFactor;
    if (scale_factor <= 0.0f) scale_factor = 1.0f;

    float dw = (float) settings->DotWidth * scale_factor;
    float dh = (float) settings->DotHeight * scale_factor;
    float gh = (float) settings->DotHorizontalGap * scale_factor;
    float gv = (float) settings->DotVerticalGap * scale_factor;

    if (rendering_mode == DOTTED_TEXT_RENDERING_MODE_PIXEL_PERFECT) {
        dw = (float) ((int) (dw + 0.5f));
        dh = (float) ((int) (dh + 0.5f));
        gh = (float) ((int) (gh + 0.5f));
        gv = (float) ((int) (gv + 0.5f));
    }

    const float sx = dw + gh;
    const float sy = dh + gv;

    const int width_dots_count = (int) (((float) bounds.size.w + gh) / sx);
    if (width_dots_count < 5) return;

    const float battery_width = dw + ((float) (width_dots_count - 1) * sx);
    const float battery_height = dh + (4.0f * sy);
    float start_x = 0, start_y = 0;

    if (horizontal_alignment == HORIZONTAL_ALIGN_CENTER) start_x = ((float) bounds.size.w - battery_width) / 2.0f;
    else if (horizontal_alignment == HORIZONTAL_ALIGN_RIGHT) start_x = (float) bounds.size.w - battery_width;

    if (vertical_alignment == VERTICAL_ALIGN_CENTER) start_y = ((float) bounds.size.h - battery_height) / 2.0f;
    else if (vertical_alignment == VERTICAL_ALIGN_BOTTOM) start_y = (float) bounds.size.h - battery_height;

    graphics_context_set_stroke_color(ctx, theme_get_theme()->BatteryOutlineColor);
    graphics_context_set_fill_color(ctx, theme_get_theme()->BatteryOutlineColor);

    for (int row = 0; row < 5; row++) {
        const float y_f = start_y + (float) row * sy;
        const int y = (int) (y_f + 0.5f);
        const int h = (rendering_mode == DOTTED_TEXT_RENDERING_MODE_SUBPIXEL) ? (int) (y_f + dh + 0.5f) - y : (int) (dh + 0.5f);

        if (row == 2) {
            // Tip pole on the LEFT
            const int x = (int) (start_x + 0.5f);
            const int w = (rendering_mode == DOTTED_TEXT_RENDERING_MODE_SUBPIXEL) ? (int) (start_x + dw + 0.5f) - x : (int) (dw + 0.5f);
            graphics_fill_rect(ctx, GRect(x, y, w, h), 0, GCornerNone);
        }

        if (row == 0 || row == 4) {
            for (int col = 0; col < width_dots_count - 1; col++) {
                const float x_f = start_x + battery_width - dw - (float) col * sx;
                const int x = (int) (x_f + 0.5f);
                const int w = (rendering_mode == DOTTED_TEXT_RENDERING_MODE_SUBPIXEL) ? (int) (x_f + dw + 0.5f) - x : (int) (dw + 0.5f);
                graphics_fill_rect(ctx, GRect(x, y, w, h), 0, GCornerNone);
            }
        } else {
            // Left & Right borders
            for (int i = 0; i < 2; i++) {
                const float x_f = (i == 0) ? start_x + sx : start_x + battery_width - dw;
                const int x = (int) (x_f + 0.5f);
                const int w = (rendering_mode == DOTTED_TEXT_RENDERING_MODE_SUBPIXEL) ? (int) (x_f + dw + 0.5f) - x : (int) (dw + 0.5f);
                graphics_fill_rect(ctx, GRect(x, y, w, h), 0, GCornerNone);
            }
        }
    }

    if (s_current_battery_level <= settings->LowBatteryThreshold) {
        graphics_context_set_fill_color(ctx, theme_get_theme()->BatteryLowColor);
    } else {
        graphics_context_set_fill_color(ctx, theme_get_theme()->BatteryFillColor);
    }
    const int fill_dots = (s_current_battery_level * (width_dots_count - 5)) / 100;
    const float y_f = start_y + 2.0f * sy;
    const int y = (int) (y_f + 0.5f);
    const int h = (rendering_mode == DOTTED_TEXT_RENDERING_MODE_SUBPIXEL) ? (int) (y_f + dh + 0.5f) - y : (int) (dh + 0.5f);
    for (int col = 0; col < fill_dots; col++) {
        const float x_f = start_x + battery_width - dw - (float) (col + 2) * sx;
        const int x = (int) (x_f + 0.5f);
        const int w = (rendering_mode == DOTTED_TEXT_RENDERING_MODE_SUBPIXEL) ? (int) (x_f + dw + 0.5f) - x : (int) (dw + 0.5f);
        graphics_fill_rect(ctx, GRect(x, y, w, h), 0, GCornerNone);
    }
}

static void destroy_battery_charging_animation() {
    animation_unschedule(charging_animation);

    // destroy animation related data
    animation_destroy(charging_animation);
    charging_animation = NULL;
}

static void initialize_battery_charging_animation() {
    battery_charging_anim_impl.update = batteryChargingAnimUpdate;

    charging_animation = animation_create();
    animation_set_duration(charging_animation, s_battery_charging_animation_duration_ms);
    animation_set_delay(charging_animation, s_battery_charging_animation_delay_ms);
    animation_set_curve(charging_animation, AnimationCurveLinear);
    animation_set_implementation(charging_animation, &battery_charging_anim_impl);
    animation_set_play_count(charging_animation, s_battery_charging_animation_repeat_count);

    animation_schedule(charging_animation);
}

void ensure_battery_animation_state(void) {
    if (s_battery_charging) {
        if (!charging_animation) {
            initialize_battery_charging_animation();
        }
    } else {
        if (charging_animation) {
            destroy_battery_charging_animation();
        }
    }
}

// Backward compatible wrapper (called by battery listener)
void update_battery_bar() {
    ensure_battery_animation_state();

    // update current battery level
    s_current_battery_level = s_battery_level;

    // Update all battery layers (and redraw)
    for (int i = 0; i < ui_state_get_row_count(); i++) {
        if (ui_state_get_widget_id(i) == WIDGET_BATTERY_BAR) {
            Layer *layer = ui_state_get_layer(i);
            if (layer) {
                layer_mark_dirty(layer);
            }
        }
    }
}

void update_battery_bar_layer(Layer *layer) {
    if (layer) {
        layer_mark_dirty(layer);
    }
}

Layer *create_battery_bar_layer(LayerBuilder builder) {
    Layer *layer = layer_factory_create_custom_layer_with_data(builder, battery_update_proc, sizeof(BatteryLayerData));
    if (!layer) {
        return NULL;
    }
    BatteryLayerData *data = layer_get_data(layer);

    data->horizontal_alignment = HORIZONTAL_ALIGN_RIGHT;
    data->vertical_alignment = VERTICAL_ALIGN_TOP;
    data->rendering_mode = DOTTED_TEXT_RENDERING_MODE_PIXEL_PERFECT;


    s_active_battery_layers++;

    // ensure animation state is correct
    ensure_battery_animation_state();

    // update on create
    update_battery_bar_layer(layer);

    return layer;
}

void battery_bar_layer_set_horizontal_alignment(Layer *layer, HorizontalAlignment alignment) {
    if (!layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Battery bar layer is NULL!");
        return;
    }
    if (alignment != HORIZONTAL_ALIGN_LEFT &&
        alignment != HORIZONTAL_ALIGN_CENTER &&
        alignment != HORIZONTAL_ALIGN_RIGHT) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid horizontal alignment!");
        return;
    }

    BatteryLayerData *data = layer_get_data(layer);
    if (!data) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Battery bar layer data not found!");
        return;
    }

    data->horizontal_alignment = alignment;
    layer_mark_dirty(layer);
}

void battery_bar_layer_set_vertical_alignment(Layer *layer, VerticalAlignment alignment) {
    if (!layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Battery bar layer is NULL!");
        return;
    }
    if (alignment != VERTICAL_ALIGN_TOP &&
        alignment != VERTICAL_ALIGN_CENTER &&
        alignment != VERTICAL_ALIGN_BOTTOM) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid vertical alignment!");
        return;
    }

    BatteryLayerData *data = layer_get_data(layer);
    if (!data) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Battery bar layer data not found!");
        return;
    }

    data->vertical_alignment = alignment;
    layer_mark_dirty(layer);
}

void battery_bar_layer_set_rendering_mode(Layer *layer, DottedTextRenderingMode mode) {
    if (!layer) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Battery bar layer is NULL!");
        return;
    }

    BatteryLayerData *data = layer_get_data(layer);
    if (!data) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Battery bar layer data not found!");
        return;
    }

    data->rendering_mode = mode;
    layer_mark_dirty(layer);
}

void destroy_battery_bar_layer(Layer *layer) {
    s_active_battery_layers--;

    // Only destroy animation if this is the last battery layer
    if (s_active_battery_layers <= 0) {
        if (charging_animation) {
            destroy_battery_charging_animation();
        }
    }

    layer_destroy(layer);
}