#pragma once

#include <stdio.h>  // For printf
#include <stdint.h> // For uint32_t (if needed by other Pebble types)
#include <stdbool.h> // For bool
#include <string.h>

// Define PBL_COLOR for testing color-dependent functions
#define PBL_COLOR

// Mock Pebble APP_LOG for host testing
#define APP_LOG_LEVEL_DEBUG   1
#define APP_LOG_LEVEL_INFO    2
#define APP_LOG_LEVEL_WARNING 3
#define APP_LOG_LEVEL_ERROR   4

#define APP_LOG(level, fmt, ...) \
    do { \
        if (level >= APP_LOG_LEVEL_DEBUG) { \
            printf("[MOCK APP_LOG] " fmt "\n", ##__VA_ARGS__); \
        } \
    } while (0)

// Define common Pebble types if they are used in production code
// that includes pebble.h and is being tested.

// Mock GColor
typedef struct GColor {
    uint8_t argb;
} GColor;

// Mock GColor constants (using arbitrary hex values for testing equality)
#define GColorBlack     ((GColor){.argb = 0b11000000}) // Example ARGB
#define GColorWhite     ((GColor){.argb = 0b11111111})
#define GColorRed       ((GColor){.argb = 0b11110000})
#define GColorGreen     ((GColor){.argb = 0b11001100})
#define GColorBlue      ((GColor){.argb = 0b11000011})
#define GColorYellow    ((GColor){.argb = 0b11111100})
#define GColorCyan      ((GColor){.argb = 0b11001111})
#define GColorMagenta   ((GColor){.argb = 0b11110011})
#define GColorOrange    ((GColor){.argb = 0b11110100})
#define GColorPurple    ((GColor){.argb = 0b11010010})
#define GColorClear     ((GColor){.argb = 0b00000000}) // Mock GColorClear

// Mock gcolor_equal function
static inline bool gcolor_equal(GColor c1, GColor c2) {
    return c1.argb == c2.argb;
}

// Mock GFont
typedef void *GFont; // A simple void pointer for testing purposes

// Mock font keys
#define FONT_KEY_LECO_32_BOLD_NUMBERS ((const char*)1)
#define FONT_KEY_LECO_42_NUMBERS      ((const char*)2)

// Mock fonts_get_system_font function
static inline GFont fonts_get_system_font(const char *font_key) {
    (void) font_key; // Suppress unused parameter warning
    return (GFont) 123; // Return a dummy non-NULL value
}

// --- Stateful Mocks for Persistent Storage ---
typedef int32_t status_t;
#define S_SUCCESS 0
#define S_INTERNAL_ERROR (-1)

// The state of our mock storage
typedef struct {
    int version;
    char data[1024];
    int data_size;
    bool version_exists;
    bool data_exists;
} MockStorage;

static MockStorage global_mock_storage;

// Test helper functions to manipulate the mock storage
static void mock_storage_reset() {
    memset(&global_mock_storage, 0, sizeof(MockStorage));
}

static void mock_storage_set_version(int version) {
    global_mock_storage.version = version;
    global_mock_storage.version_exists = true;
}

static int mock_storage_get_version() {
    return global_mock_storage.version;
}

static void mock_storage_set_data(void *data, size_t size) {
    if (size <= sizeof(global_mock_storage.data)) {
        memcpy(global_mock_storage.data, data, size);
        global_mock_storage.data_size = size;
        global_mock_storage.data_exists = true;
    }
}

// The mock implementations of the Pebble SDK functions
static bool persist_exists(const uint32_t key) {
    if (key == 2) return global_mock_storage.version_exists;
    if (key == 1) return global_mock_storage.data_exists;
    return false;
}

static int persist_read_int(const uint32_t key) {
    if (key == 2) return global_mock_storage.version;
    return 0;
}

static int persist_read_data(const uint32_t key, void *buffer, const size_t size) {
    if (key == 1 && global_mock_storage.data_exists) {
        size_t to_copy = size < global_mock_storage.data_size ? size : global_mock_storage.data_size;
        memcpy(buffer, global_mock_storage.data, to_copy);
        return to_copy;
    }
    return 0;
}

static int persist_write_data(const uint32_t key, const void *data, const size_t size) {
    if (key == 1) {
        if (size <= sizeof(global_mock_storage.data)) {
            memcpy(global_mock_storage.data, data, size);
            global_mock_storage.data_size = size;
            global_mock_storage.data_exists = true;
            return size;
        }
    }
    return 0;
}

static status_t persist_write_int(const uint32_t key, const int32_t value) {
    if (key == 2) {
        global_mock_storage.version = value;
        global_mock_storage.version_exists = true;
        return S_SUCCESS;
    }
    return S_INTERNAL_ERROR;
}

// --- End of Storage Mocks ---


// AppMessage mocks (if needed for app_messaging.c tests)
typedef int AppMessageResult;
#define APP_MSG_OK 0
#define APP_MSG_INTERNAL_ERROR 1

typedef struct Tuple {
    uint32_t message_key;
    uint8_t type;

    union {
        int32_t int32;
        char cstring[100]; // Example size
    } value;

    uint16_t length;
} Tuple;

#define TUPLE_CSTRING 1
#define TUPLE_INT 2

typedef struct DictionaryIterator DictionaryIterator; // Opaque type

static Tuple *dict_find(DictionaryIterator *iter, uint32_t key) {
    (void) iter;
    (void) key;
    return NULL; // Mock: always return NULL for now, implement if needed
}

static AppMessageResult app_message_outbox_begin(DictionaryIterator **iter) {
    (void) iter;
    return APP_MSG_OK;
}

static void dict_write_int(DictionaryIterator *iter, uint32_t key, const void *value, size_t size, bool signed_val) {
    (void) iter;
    (void) key;
    (void) value;
    (void) size;
    (void) signed_val;
}

static AppMessageResult app_message_outbox_send() {
    return APP_MSG_OK;
}

typedef void (*AppMessageInboxReceived)(DictionaryIterator *iterator, void *context);

typedef void (*AppMessageInboxDropped)(AppMessageResult reason, void *context);

typedef void (*AppMessageOutboxFailed)(DictionaryIterator *iterator, AppMessageResult reason, void *context);

typedef void (*AppMessageOutboxSent)(DictionaryIterator *iterator, void *context);

static void app_message_register_inbox_received(AppMessageInboxReceived handler) { (void) handler; }
static void app_message_register_inbox_dropped(AppMessageInboxDropped handler) { (void) handler; }
static void app_message_register_outbox_failed(AppMessageOutboxFailed handler) { (void) handler; }
static void app_message_register_outbox_sent(AppMessageOutboxSent handler) { (void) handler; }

static AppMessageResult app_message_open(uint32_t inbox_size, uint32_t outbox_size) {
    (void) inbox_size;
    (void) outbox_size;
    return APP_MSG_OK;
}

// AppTimer mocks
typedef void AppTimer;

typedef void (*AppTimerCallback)(void *data);

static AppTimer *app_timer_register(uint32_t timeout_ms, AppTimerCallback callback, void *data) {
    (void) timeout_ms;
    (void) callback;
    (void) data;
    return (AppTimer *) 1; // Return a dummy non-NULL timer
}

static bool app_timer_cancel(AppTimer *timer) {
    (void) timer;
    return true;
}

// Layer mocks
typedef void Layer;
typedef void GContext;

// Corrected GPoint and GSize definitions
typedef struct GPoint {
    int16_t x;
    int16_t y;
} GPoint;

typedef struct GSize {
    int16_t w;
    int16_t h;
} GSize;

typedef struct GRect {
    GPoint origin;
    GSize size;
} GRect;

static GPoint GPoint_construct(int16_t x, int16_t y) {
    return (GPoint){.x = x, .y = y};
}

#define GPoint(x, y) GPoint_construct(x, y)

static GRect GRect_construct(int16_t x, int16_t y, int16_t w, int16_t h) {
    return (GRect){.origin = {.x = x, .y = y}, .size = {.w = w, .h = h}};
}
#define GRect(x, y, w, h) GRect_construct(x, y, w, h)


static GRect layer_get_bounds(Layer *layer) {
    (void) layer;
    return (GRect){.origin = {.x = 0, .y = 0}, .size = {.w = 144, .h = 168}}; // Dummy bounds
}

static void layer_mark_dirty(Layer *layer) { (void) layer; }
static void layer_destroy(Layer *layer) { (void) layer; }

// Graphics mocks for pixel_matrix_drawer and graph_utils
typedef uint8_t GCorners;
#define GCornerNone 0

// --- Mocks for graphics_fill_rect ---
#define MAX_GRAPHICS_FILL_RECT_CALLS 2000 // Increased buffer size

typedef struct {
    GRect rect;
    uint16_t corner_radius;
    GCorners corners;
    GColor fill_color; // Added to record fill color
} GraphicsFillRectCall;

static GraphicsFillRectCall s_graphics_fill_rect_calls[MAX_GRAPHICS_FILL_RECT_CALLS];
static int s_graphics_fill_rect_call_count = 0;
static GColor s_current_fill_color = {.argb = 0}; // Default to black/clear

static void graphics_context_set_fill_color(GContext *ctx, GColor color) {
    (void) ctx;
    s_current_fill_color = color;
}

static void graphics_fill_rect(GContext *ctx, GRect rect, uint16_t corner_radius, GCorners corners) {
    (void) ctx;
    if (s_graphics_fill_rect_call_count < MAX_GRAPHICS_FILL_RECT_CALLS) {
        s_graphics_fill_rect_calls[s_graphics_fill_rect_call_count++] = (GraphicsFillRectCall){
            .rect = rect,
            .corner_radius = corner_radius,
            .corners = corners,
            .fill_color = s_current_fill_color // Record the current fill color
        };
    } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "graphics_fill_rect mock buffer overflow!");
    }
}

// Helper to reset the mock call count
static void reset_graphics_fill_rect_calls() {
    s_graphics_fill_rect_call_count = 0;
    s_current_fill_color = (GColor){.argb = 0}; // Reset fill color
}

// Helper to get the recorded calls
static GraphicsFillRectCall *get_graphics_fill_rect_calls() {
    return s_graphics_fill_rect_calls;
}

static int get_graphics_fill_rect_call_count() {
    return s_graphics_fill_rect_call_count;
}

// --- Mocks for graphics_draw_line ---
#define MAX_GRAPHICS_DRAW_LINE_CALLS 2000 // Increased buffer size

typedef struct {
    GPoint p0;
    GPoint p1;
    GColor stroke_color; // Added to record stroke color
} GraphicsDrawLineCall;

static GraphicsDrawLineCall s_graphics_draw_line_calls[MAX_GRAPHICS_DRAW_LINE_CALLS];
static int s_graphics_draw_line_call_count = 0;
static GColor s_current_stroke_color = {.argb = 0}; // Default to black/clear

static void graphics_context_set_stroke_color(GContext *ctx, GColor color) {
    (void) ctx;
    s_current_stroke_color = color;
}

static void graphics_draw_line(GContext *ctx, GPoint p0, GPoint p1) {
    (void) ctx;
    if (s_graphics_draw_line_call_count < MAX_GRAPHICS_DRAW_LINE_CALLS) {
        s_graphics_draw_line_calls[s_graphics_draw_line_call_count++] = (GraphicsDrawLineCall){
            .p0 = p0,
            .p1 = p1,
            .stroke_color = s_current_stroke_color // Record the current stroke color
        };
    } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "graphics_draw_line mock buffer overflow!");
    }
}

static void reset_graphics_draw_line_calls() {
    s_graphics_draw_line_call_count = 0;
    s_current_stroke_color = (GColor){.argb = 0}; // Reset stroke color
}

static GraphicsDrawLineCall *get_graphics_draw_line_calls() {
    return s_graphics_draw_line_calls;
}

static int get_graphics_draw_line_call_count() {
    return s_graphics_draw_line_call_count;
}
