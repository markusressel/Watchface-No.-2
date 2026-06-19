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

#include <time.h>

static time_t s_mock_time_val = 0;
static struct tm s_mock_tm_val;

static time_t mock_time(time_t *t) {
    if (t) *t = s_mock_time_val;
    return s_mock_time_val;
}

static struct tm *mock_localtime(const time_t *t) {
    return &s_mock_tm_val;
}

#define time mock_time
#define localtime mock_localtime
#include "../../src/c/ui/layer/date.c"
#undef time
#undef localtime

void setUp(void) {
    memset(&s_settings, 0, sizeof(ClaySettings));
    s_settings.DotScaleFactor = 1.0f;
    s_settings.DateZeroPadding = true;
    s_row_count = 0;
    memset(s_last_dotted_text, 0, sizeof(s_last_dotted_text));

    // Default mock date: January 1, 2026 (Thursday)
    memset(&s_mock_tm_val, 0, sizeof(struct tm));
    s_mock_tm_val.tm_mday = 1;
    s_mock_tm_val.tm_mon = 0; // January
    s_mock_tm_val.tm_year = 126; // 2026
    s_mock_tm_val.tm_wday = 4; // Thursday
}

void tearDown(void) {}

void test_date_layer_create_destroy(void) {
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);
    TEST_ASSERT_NOT_NULL(layer);
    destroy_date_layer(layer);
}

static void get_expected_date(char *dest, size_t dest_size, bool show_weekday, bool weekday_upper, bool show_year, bool zero_padding) {
    struct tm *tick_time = &s_mock_tm_val;
    char date_format[32] = "";
    if (show_weekday) {
        strcat(date_format, "%a ");
    }
    strcat(date_format, "%d.%m");
    if (show_year) {
        strcat(date_format, ".%y");
    }
    strftime(dest, dest_size, date_format, tick_time);
    if (show_weekday) {
        int idxToDel = 2;
        memmove(&dest[idxToDel], &dest[idxToDel + 1], strlen(dest) - idxToDel);
        if (weekday_upper) {
            for (int j = 0; j < 2; j++) {
                dest[j] = upper(dest[j]);
            }
        }
    }
    if (!zero_padding) {
        char *p = dest;
        if (show_weekday) {
            while (*p && *p != ' ') {
                p++;
            }
            if (*p == ' ') {
                p++;
            }
        }
        if (*p == '0') {
            memmove(p, p + 1, strlen(p + 1) + 1);
        }
        while (*p && *p != '.') {
            p++;
        }
        if (*p == '.') {
            p++;
            if (*p == '0') {
                memmove(p, p + 1, strlen(p + 1) + 1);
            }
        }
    }
}

void test_update_date_format_basic(void) {
    s_settings.ShowWeekdayAbbreviation = false;
    s_settings.ShowYear = false;
    s_settings.DateZeroPadding = true;
    
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);
    
    char expected[32];
    get_expected_date(expected, sizeof(expected), false, false, false, true);
    TEST_ASSERT_EQUAL_STRING(expected, s_last_dotted_text);
    
    destroy_date_layer(layer);
}

void test_update_date_format_with_year(void) {
    s_settings.ShowWeekdayAbbreviation = false;
    s_settings.ShowYear = true;
    s_settings.DateZeroPadding = true;
    
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);
    
    char expected[32];
    get_expected_date(expected, sizeof(expected), false, false, true, true);
    TEST_ASSERT_EQUAL_STRING(expected, s_last_dotted_text);
    
    destroy_date_layer(layer);
}

void test_update_date_format_with_weekday(void) {
    s_settings.ShowWeekdayAbbreviation = true;
    s_settings.ShowYear = false;
    s_settings.WeekdayAbbreviationUppercase = true;
    s_settings.DateZeroPadding = true;
    
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);
    
    char expected[32];
    get_expected_date(expected, sizeof(expected), true, true, false, true);
    TEST_ASSERT_EQUAL_STRING(expected, s_last_dotted_text);
    
    destroy_date_layer(layer);
}

void test_update_date_format_with_weekday_lowercase(void) {
    s_settings.ShowWeekdayAbbreviation = true;
    s_settings.ShowYear = false;
    s_settings.WeekdayAbbreviationUppercase = false;
    s_settings.DateZeroPadding = true;
    
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);
    
    char expected[32];
    get_expected_date(expected, sizeof(expected), true, false, false, true);
    TEST_ASSERT_EQUAL_STRING(expected, s_last_dotted_text);
    
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

void test_update_date_format_unpadded_basic(void) {
    s_settings.ShowWeekdayAbbreviation = false;
    s_settings.ShowYear = false;
    s_settings.DateZeroPadding = false;

    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);

    char expected[32];
    get_expected_date(expected, sizeof(expected), false, false, false, false);
    TEST_ASSERT_EQUAL_STRING(expected, s_last_dotted_text);

    destroy_date_layer(layer);
}

void test_update_date_format_unpadded_with_year(void) {
    s_settings.ShowWeekdayAbbreviation = false;
    s_settings.ShowYear = true;
    s_settings.DateZeroPadding = false;

    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);

    char expected[32];
    get_expected_date(expected, sizeof(expected), false, false, true, false);
    TEST_ASSERT_EQUAL_STRING(expected, s_last_dotted_text);

    destroy_date_layer(layer);
}

void test_update_date_format_unpadded_with_weekday(void) {
    s_settings.ShowWeekdayAbbreviation = true;
    s_settings.ShowYear = false;
    s_settings.WeekdayAbbreviationUppercase = true;
    s_settings.DateZeroPadding = false;

    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);

    char expected[32];
    get_expected_date(expected, sizeof(expected), true, true, false, false);
    TEST_ASSERT_EQUAL_STRING(expected, s_last_dotted_text);

    destroy_date_layer(layer);
}

void test_update_date_format_unpadded_with_weekday_lowercase(void) {
    s_settings.ShowWeekdayAbbreviation = true;
    s_settings.ShowYear = false;
    s_settings.WeekdayAbbreviationUppercase = false;
    s_settings.DateZeroPadding = false;

    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);

    char expected[32];
    get_expected_date(expected, sizeof(expected), true, false, false, false);
    TEST_ASSERT_EQUAL_STRING(expected, s_last_dotted_text);

    destroy_date_layer(layer);
}

void test_update_date_format_unpadded_no_leading_zeros(void) {
    s_mock_tm_val.tm_mday = 25;
    s_mock_tm_val.tm_mon = 11; // December
    s_mock_tm_val.tm_year = 126; // 2026
    s_mock_tm_val.tm_wday = 5; // Friday

    s_settings.ShowWeekdayAbbreviation = true;
    s_settings.ShowYear = true;
    s_settings.WeekdayAbbreviationUppercase = true;
    s_settings.DateZeroPadding = false;

    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);

    char expected[32];
    get_expected_date(expected, sizeof(expected), true, true, true, false);
    TEST_ASSERT_EQUAL_STRING(expected, s_last_dotted_text);

    destroy_date_layer(layer);
}

void test_date_layer_create_auto_scale(void) {
    s_settings.DotAutoScale = true;
    LayerBuilder builder = { .bounds = GRect(0, 0, 144, 30) };
    Layer *layer = create_date_layer(builder);
    TEST_ASSERT_NOT_NULL(layer);
    destroy_date_layer(layer);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_date_layer_create_destroy);
    RUN_TEST(test_update_date_format_basic);
    RUN_TEST(test_update_date_format_with_year);
    RUN_TEST(test_update_date_format_with_weekday);
    RUN_TEST(test_update_date_format_with_weekday_lowercase);
    RUN_TEST(test_update_date_triggers_from_ui_state);
    RUN_TEST(test_update_date_format_unpadded_basic);
    RUN_TEST(test_update_date_format_unpadded_with_year);
    RUN_TEST(test_update_date_format_unpadded_with_weekday);
    RUN_TEST(test_update_date_format_unpadded_with_weekday_lowercase);
    RUN_TEST(test_update_date_format_unpadded_no_leading_zeros);
    RUN_TEST(test_date_layer_create_auto_scale);
    return UNITY_END();
}
