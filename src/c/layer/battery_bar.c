#include "battery_bar.h"
#include "../battery.h"
#include "../theme.h"
#include "../clay_settings.h"
#include "../layer_factory.h"

#define MAX_BATTERY_LAYERS 5

// Registry of all created battery bar layers
typedef struct {
    Layer *layer;
    int current_battery_level;
} BatteryLayerInstance;

static BatteryLayerInstance s_battery_layers[MAX_BATTERY_LAYERS];
static int s_battery_layer_count = 0;

// temporary variable used for animation (global state - shared by all layers)
static int s_current_battery_level;

// battery charging animation callbacks
static AnimationImplementation batteryChargingAnimImpl;

static Animation *charging_animation;

// battery charging animation durations
static int s_battery_charging_animation_duration = 2000;
// delay between a full battery charging animation cycle
static int s_battery_charging_animation_delay = 600;
static const int s_battery_charging_animation_repeat_count = ANIMATION_DURATION_INFINITE;

static int scaled_dimension(int value, float scale_factor) {
    int scaled = (int) ((float) value * scale_factor + 0.5f);
    return scaled < 1 ? 1 : scaled;
}

static float auto_scale_for_height(const ClaySettings *settings, int available_height) {
    const int base_height = (5 * settings->DotHeight) + (4 * settings->DotVerticalGap);
    if (base_height <= 0 || available_height <= 0) {
        return 1.0f;
    }

    float scale = (float) available_height / (float) base_height;
    return scale > 0.0f ? scale : 1.0f;
}

static void fit_vertical_metrics_to_bounds(int *dot_height, int *gap_vertical, int max_height) {
    while ((5 * (*dot_height) + 4 * (*gap_vertical)) > max_height) {
        if (*gap_vertical > 1) {
            (*gap_vertical)--;
        } else if (*dot_height > 1) {
            (*dot_height)--;
        } else {
            break;
        }
    }
}

static void draw_battery_fill(int percent) {
    s_current_battery_level = percent;
    // Mark all battery layers as dirty
    for (int i = 0; i < s_battery_layer_count; i++) {
        layer_mark_dirty(s_battery_layers[i].layer);
    }
}

// update battery charging animation
static void batteryChargingAnimUpdate(Animation *animation, const AnimationProgress progress) {
    int progressPercent = (progress * 100 / ANIMATION_NORMALIZED_MAX);
    // calculate animation fill rate
    int fillPercent = s_battery_level + (progressPercent * (100 - s_battery_level) / 100);

    draw_battery_fill(fillPercent);
}

// draw the battery layer
static void battery_update_proc(Layer *layer, GContext *ctx) {
    const GRect bounds = layer_get_bounds(layer);

    ClaySettings *settings = clay_get_settings();
    float scale_factor = settings->DotAutoScale
        ? auto_scale_for_height(settings, bounds.size.h)
        : settings->DotScaleFactor;
    if (scale_factor <= 0.0f) {
        scale_factor = 1.0f;
    }

    const int dot_width = scaled_dimension(settings->DotWidth, scale_factor);
    int dot_height = scaled_dimension(settings->DotHeight, scale_factor);
    const int gap_horizontal = scaled_dimension(settings->DotHorizontalGap, scale_factor);
    int gap_vertical = scaled_dimension(settings->DotVerticalGap, scale_factor);
    fit_vertical_metrics_to_bounds(&dot_height, &gap_vertical, bounds.size.h);
    const int step_x = dot_width + gap_horizontal;
    const int step_y = dot_height + gap_vertical;

    int leftMargin = 0;
    int rightMargin = 0;

    // Draw battery outline
    graphics_context_set_stroke_color(ctx, theme_get_theme()->BatteryOutlineColor);
    graphics_context_set_fill_color(ctx, theme_get_theme()->BatteryOutlineColor);

    // Calculate how many matrix columns fit horizontally.
    const int available_width = bounds.size.w - leftMargin - rightMargin;
    const int widthDotsCount = (available_width + gap_horizontal) / step_x;
    if (widthDotsCount < 5) {
        return;
    }

    // upper row
    int x; // dot x position;
    int y; // dot y position;
    GRect currentDotBorder;
    for (int row = 0; row < 5; row++) {
        y = row * step_y;

        // single dot at the tip (representing +pole)
        if (row == 2) {
            x = ((widthDotsCount - 1) * step_x);
            currentDotBorder = GRect(
                bounds.size.w - rightMargin - dot_width - x,
                y,
                dot_width,
                dot_height);
            graphics_fill_rect(ctx, currentDotBorder, 0, GCornerNone);
        }

        // upper and lower row
        if (row == 0 || row == 4) {
            for (int column = 0; column < widthDotsCount - 1; column++) {
                x = (column * step_x);
                currentDotBorder = GRect(
                    bounds.size.w - rightMargin - dot_width - x,
                    y,
                    dot_width,
                    dot_height);
                graphics_fill_rect(ctx, currentDotBorder, 0, GCornerNone);
            }
        } else {
            // single dot at beginning and end of the battery (left and right borders)

            x = ((widthDotsCount - 2) * step_x);
            currentDotBorder = GRect(
                bounds.size.w - rightMargin - dot_width - x,
                y,
                dot_width,
                dot_height);
            graphics_fill_rect(ctx, currentDotBorder, 0, GCornerNone);

            x = 0;
            currentDotBorder = GRect(
                bounds.size.w - rightMargin - dot_width - x,
                y,
                dot_width,
                dot_height);
            graphics_fill_rect(ctx, currentDotBorder, 0, GCornerNone);
        }
    }

    // Draw the bar inside
    graphics_context_set_fill_color(ctx, theme_get_theme()->BatteryFillColor);

    int fillDotsCount = (s_current_battery_level * (widthDotsCount - 5)) / 100;
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "widthDotsCount: %d, batteryLevel: %d, fillDotsCount: %d",
    //        widthDotsCount, s_current_battery_level, fillDotsCount);

    const int row = 2;
    for (int column = 0; column < fillDotsCount; column++) {
        x = ((column + 2) * step_x);
        y = row * step_y;
        currentDotBorder = GRect(
            bounds.size.w - rightMargin - dot_width - x,
            y,
            dot_width,
            dot_height);
        graphics_fill_rect(ctx, currentDotBorder, 0, GCornerNone);
    }
}

static void destroy_battery_charging_animation() {
    animation_unschedule(charging_animation);

    // destroy animation related data
    animation_destroy(charging_animation);
}

static void initialize_battery_charging_animation() {
    batteryChargingAnimImpl.update = batteryChargingAnimUpdate;

    charging_animation = animation_create();
    animation_set_duration(charging_animation, s_battery_charging_animation_duration);
    animation_set_delay(charging_animation, s_battery_charging_animation_delay);
    animation_set_curve(charging_animation, AnimationCurveLinear);
    animation_set_implementation(charging_animation, &batteryChargingAnimImpl);
    animation_set_play_count(charging_animation, s_battery_charging_animation_repeat_count);
    //animation_set_handlers(animation,(AnimationHandlers){
    //  .started = animationStartedHandler,
    //  .stopped = animationStoppedHandler
    //}, (void*)(uint32_t)i);

    animation_schedule(charging_animation);
}

// Backward compatible wrapper (called by battery listener)
void update_battery_bar() {
    if (s_battery_charging) {
        if (!charging_animation) {
            initialize_battery_charging_animation();
        }
    } else {
        // disable charging animation, if it was active before
        if (charging_animation) {
            destroy_battery_charging_animation();
        }
    }

    // update current battery level
    s_current_battery_level = s_battery_level;

    // Update all battery layers (and redraw)
    for (int i = 0; i < s_battery_layer_count; i++) {
        layer_mark_dirty(s_battery_layers[i].layer);
    }
}

void update_battery_bar_layer(Layer *layer) {
    update_battery_bar();
}

Layer *create_battery_bar_layer(LayerBuilder builder) {
    if (s_battery_layer_count >= MAX_BATTERY_LAYERS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Max battery layers exceeded!");
        return NULL;
    }

    BatteryLayerInstance *instance = &s_battery_layers[s_battery_layer_count];

    instance->layer = layer_factory_create_custom_layer(builder, battery_update_proc);
    instance->current_battery_level = 0;

    s_battery_layer_count++;

    // update on create
    update_battery_bar_layer(instance->layer);

    return instance->layer;
}

void destroy_battery_bar_layer(Layer *layer) {
    // Find and remove from registry
    for (int i = 0; i < s_battery_layer_count; i++) {
        if (s_battery_layers[i].layer == layer) {
            // Remove from array by shifting remaining elements
            for (int j = i; j < s_battery_layer_count - 1; j++) {
                s_battery_layers[j] = s_battery_layers[j + 1];
            }
            s_battery_layer_count--;
            break;
        }
    }

    // Only destroy animation if this is the last battery layer
    if (s_battery_layer_count == 0) {
        if (charging_animation) {
            destroy_battery_charging_animation();
        }
    }

    layer_destroy(layer);
}
