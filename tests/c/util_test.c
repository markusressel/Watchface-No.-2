#include "unity/unity.h"

#include <stdio.h>   // For printf (still useful for general output)
#include <stdbool.h> // For bool
#include <string.h>  // For strcmp

#include "../../src/c/util.h" // Include the header for the functions being tested
#include "../../src/c/util.c" // Include the C implementation file directly for testing

// Unity setup and teardown functions (required even if empty)
void setUp(void) {
}

void tearDown(void) {
}

// Test function for format_int_array
void test_format_int_array(void) {
    char buf[64];
    int arr1[] = {1, 2, 3};
    format_int_array(buf, sizeof(buf), arr1, 3);
    TEST_ASSERT_EQUAL_STRING("[1, 2, 3]", buf);

    int arr2[] = {10};
    format_int_array(buf, sizeof(buf), arr2, 1);
    TEST_ASSERT_EQUAL_STRING("[10]", buf);

    int arr3[] = {};
    format_int_array(buf, sizeof(buf), arr3, 0);
    TEST_ASSERT_EQUAL_STRING("[]", buf);

    // Test buffer overflow
    char small_buf[5]; // Should fit "[1,]"
    int arr4[] = {1, 2, 3, 4, 5};
    format_int_array(small_buf, sizeof(small_buf), arr4, 5);
    TEST_ASSERT_EQUAL_STRING("[1,]", small_buf);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_format_int_array);
    return UNITY_END();
}
