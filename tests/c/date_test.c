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
    .DateTextColor = {.argb = 0b11111111}
};
Theme *theme_get_theme() { return &s_theme; }

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

#include "../../src/c/ui/layer/date.c"

void setUp(void) {
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.DotScaleFactor = 1.0f;
    s_row_count = 0;
    memset(s_last_dotted_text, 0, sizeof(s_last_dotted_text));
}

void tearDown(void) {}

void test_date_layer_create_destroy(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);
    TEST_ASSERT_NOT_NULL(layer);
    destroy_date_layer(layer);
}

void test_update_date_format_basic(void) {
    s_settings.ShowWeekdayAbbreviation = false;
    s_settings.ShowYear = false;
    
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);
    
    // s_last_dotted_text should be "DD.MM" (5 chars)
    TEST_ASSERT_EQUAL_INT(5, strlen(s_last_dotted_text));
    TEST_ASSERT_EQUAL('.', s_last_dotted_text[2]);
    
    destroy_date_layer(layer);
}

void test_update_date_format_with_year(void) {
    s_settings.ShowWeekdayAbbreviation = false;
    s_settings.ShowYear = true;
    
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);
    
    // s_last_dotted_text should be "DD.MM.YY" (8 chars)
    TEST_ASSERT_EQUAL_INT(8, strlen(s_last_dotted_text));
    TEST_ASSERT_EQUAL('.', s_last_dotted_text[5]);
    
    destroy_date_layer(layer);
}

void test_update_date_format_with_weekday(void) {
    s_settings.ShowWeekdayAbbreviation = true;
    s_settings.ShowYear = false;
    s_settings.WeekdayAbbreviationUppercase = true;
    
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);
    
    // s_last_dotted_text should be "WW DD.MM" (8 chars if WW is 2 chars + space)
    TEST_ASSERT_TRUE(strlen(s_last_dotted_text) >= 8);
    
    destroy_date_layer(layer);
}

void test_update_date_triggers_from_ui_state(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);
    
    s_row_count = 1;
    s_mock_widgets[0] = WIDGET_DATE;
    s_mock_layers[0] = layer;
    
    update_date();
    TEST_ASSERT_TRUE(strlen(s_last_dotted_text) > 0);
    
    destroy_date_layer(layer);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_date_layer_create_destroy);
    RUN_TEST(test_update_date_format_basic);
    RUN_TEST(test_update_date_format_with_year);
    RUN_TEST(test_update_date_format_with_weekday);
    RUN_TEST(test_update_date_triggers_from_ui_state);
    return UNITY_END();
}
