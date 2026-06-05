#include "pebble_mock.h" // Must be first to define HOST_TEST_SHIM_BUILD and APP_LOG mock

#include <stdio.h>   // For printf
#include <stdbool.h> // For bool
#include <string.h>  // For strcmp

#include "../../src/c/util.h" // Include the header for the functions being tested

// Now include the C implementation file directly.
// Because pebble_mock.h was included first, HOST_TEST_SHIM_BUILD is defined,
// which prevents src/c/util.c from including pebble.h and instead uses printf for APP_LOG.
#include "../../src/c/util.c"

// Simple test function for format_int_array
static bool test_format_int_array() {
    char buf[64];
    int arr1[] = {1, 2, 3};
    format_int_array(buf, sizeof(buf), arr1, 3);
    if (strcmp(buf, "[1, 2, 3]") != 0) {
        printf("test_format_int_array failed: Expected \"[1, 2, 3]\", got \"%s\"\n", buf);
        return false;
    }

    int arr2[] = {10};
    format_int_array(buf, sizeof(buf), arr2, 1);
    if (strcmp(buf, "[10]") != 0) {
        printf("test_format_int_array failed: Expected \"[10]\", got \"%s\"\n", buf);
        return false;
    }

    int arr3[] = {};
    format_int_array(buf, sizeof(buf), arr3, 0);
    if (strcmp(buf, "[]") != 0) {
        printf("test_format_int_array failed: Expected \"[]\", got \"%s\"\n", buf);
        return false;
    }

    // Test buffer overflow
    char small_buf[5]; // Should fit "[1,]"
    int arr4[] = {1, 2, 3, 4, 5};
    format_int_array(small_buf, sizeof(small_buf), arr4, 5);
    if (strcmp(small_buf, "[1,]") != 0) {
        printf("test_format_int_array failed (small buf): Expected \"[1,]\", got \"%s\"\n", small_buf);
        return false;
    }

    return true;
}

int main() {
    printf("Running C tests...\n");
    bool all_passed = true;

    if (test_format_int_array()) {
        printf("test_format_int_array: PASSED\n");
    } else {
        printf("test_format_int_array: FAILED\n");
        all_passed = false;
    }

    printf("C tests finished.\n");
    return all_passed ? 0 : 1;
}
