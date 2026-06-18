#include "unity/unity.h"
#include <stdbool.h>
#include <string.h>
#include "pebble/pebble.h"

// Mock dependencies
#include "../../src/c/settings/clay_settings.h"
#include "../../src/c/ui/theme.h"
#include "../../src/c/ui/layer/dotted_text_layer.h"
#include "../../src/c/ui/layer_factory.h"
#include "../../src/c/ui/ui_state.h"

ClaySettings s_settings;
ClaySettings *clay_get_settings() { return &s_settings; }

static Theme s_theme = {
    .StepcountTextColor = {.argb = 0b11111111}
};
Theme *theme_get_theme() { return &s_theme; }

// health_listener.h mocks
int s_step_count = 0;

// DottedTextLayer mocks
static char s_last_dotted_text[32];
void dotted_text_layer_set_text(DottedTextLayer *dotted_text_layer, char *text) {
    if (text) strncpy(s_last_dotted_text, text, sizeof(s_last_dotted_text));
}
void dotted_text_layer_set_auto_scale(DottedTextLayer *dotted_text_layer, bool enabled) {}
void dotted_text_layer_set_scale_factor(DottedTextLayer *dotted_text_layer, float scale) {}
void dotted_text_layer_destroy(DottedTextLayer *dotted_text_layer) { free(dotted_text_layer); }

// ui_state.h mocks
static Layer* s_mock_layers[5];
static WidgetId s_mock_widgets[5];
static int s_row_count = 0;
int ui_state_get_row_count() { return s_row_count; }
WidgetId ui_state_get_widget_id(int row) { return s_mock_widgets[row]; }
Layer* ui_state_get_layer(int row) { return s_mock_layers[row]; }

// layer_factory.h mocks
DottedTextLayer *layer_factory_create_dotted_text_layer(LayerBuilder builder, GColor color, HorizontalAlignment h_align, VerticalAlignment v_align, const char *text) {
    DottedTextLayer *layer = layer_create(builder.bounds);
    return layer;
}

#include "../../src/c/ui/layer/stepcount.c"

void setUp(void) {
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.DotScaleFactor = 1.0f;
    s_row_count = 0;
    s_step_count = 0;
    s_mock_step_count = 0;
    memset(s_last_dotted_text, 0, sizeof(s_last_dotted_text));
}

void tearDown(void) {}

void test_stepcount_layer_create_destroy(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_stepcount_layer(builder);
    TEST_ASSERT_NOT_NULL(layer);
    destroy_stepcount_layer(layer);
}

void test_update_stepcount_with_data(void) {
    s_mock_step_count = 5000;
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_stepcount_layer(builder);
    
    TEST_ASSERT_EQUAL_STRING("5000", s_last_dotted_text);
    
    destroy_stepcount_layer(layer);
}

void test_update_stepcount_triggers_from_ui_state(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_stepcount_layer(builder);
    
    s_row_count = 1;
    s_mock_widgets[0] = WIDGET_STEPCOUNT;
    s_mock_layers[0] = layer;
    
    s_step_count = 6000;
    update_stepcount();
    TEST_ASSERT_EQUAL_STRING("6000", s_last_dotted_text);
    
    destroy_stepcount_layer(layer);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_stepcount_layer_create_destroy);
    RUN_TEST(test_update_stepcount_with_data);
    RUN_TEST(test_update_stepcount_triggers_from_ui_state);
    return UNITY_END();
}
