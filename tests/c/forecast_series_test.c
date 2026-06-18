#include "unity/unity.h"
#include <time.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "../../src/c/app_messaging/forecast_series.h"
#include "../../src/c/app_messaging/forecast_series.c"

void setUp(void) {
}

void tearDown(void) {
}

void test_forecast_parse_int_series_basic(void) {
    const char *encoded = "10,20,30,40";
    int values[10];
    int count = forecast_parse_int_series(encoded, strlen(encoded) + 1, values, 10);

    TEST_ASSERT_EQUAL_INT(4, count);
    TEST_ASSERT_EQUAL_INT(10, values[0]);
    TEST_ASSERT_EQUAL_INT(20, values[1]);
    TEST_ASSERT_EQUAL_INT(30, values[2]);
    TEST_ASSERT_EQUAL_INT(40, values[3]);
}

void test_forecast_parse_int_series_negative_and_spaces(void) {
    const char *encoded = "-5, 10 ,-15, 20";
    int values[10];
    int count = forecast_parse_int_series(encoded, strlen(encoded) + 1, values, 10);

    TEST_ASSERT_EQUAL_INT(4, count);
    TEST_ASSERT_EQUAL_INT(-5, values[0]);
    TEST_ASSERT_EQUAL_INT(10, values[1]);
    TEST_ASSERT_EQUAL_INT(-15, values[2]);
    TEST_ASSERT_EQUAL_INT(20, values[3]);
}

void test_forecast_parse_int_series_malformed(void) {
    const char *encoded = "10,abc,20,30def,40";
    int values[10];
    int count = forecast_parse_int_series(encoded, strlen(encoded) + 1, values, 10);

    // "abc" should be skipped. "30def" should parse 30 and skip "def".
    TEST_ASSERT_EQUAL_INT(4, count);
    TEST_ASSERT_EQUAL_INT(10, values[0]);
    TEST_ASSERT_EQUAL_INT(20, values[1]);
    TEST_ASSERT_EQUAL_INT(30, values[2]);
    TEST_ASSERT_EQUAL_INT(40, values[3]);
}

void test_forecast_parse_int_series_max_values(void) {
    const char *encoded = "1,2,3,4,5";
    int values[3];
    int count = forecast_parse_int_series(encoded, strlen(encoded) + 1, values, 3);

    TEST_ASSERT_EQUAL_INT(3, count);
    TEST_ASSERT_EQUAL_INT(1, values[0]);
    TEST_ASSERT_EQUAL_INT(2, values[1]);
    TEST_ASSERT_EQUAL_INT(3, values[2]);
}

void test_forecast_parse_int_series_null_or_empty(void) {
    int values[10];
    TEST_ASSERT_EQUAL_INT(0, forecast_parse_int_series(NULL, 10, values, 10));
    TEST_ASSERT_EQUAL_INT(0, forecast_parse_int_series("", 1, values, 10));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_forecast_parse_int_series_basic);
    RUN_TEST(test_forecast_parse_int_series_negative_and_spaces);
    RUN_TEST(test_forecast_parse_int_series_malformed);
    RUN_TEST(test_forecast_parse_int_series_max_values);
    RUN_TEST(test_forecast_parse_int_series_null_or_empty);
    return UNITY_END();
}