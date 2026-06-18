#pragma once

#include <stdio.h>  // For printf
#include <stdint.h> // For uint32_t (if needed by other Pebble types)
#include <stdbool.h> // For bool
#include <string.h>
#include <time.h>
#include <stdlib.h>
typedef struct tm tm;

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
#define GColorPictonBlue ((GColor){.argb = 0b11001011}) // Mock Picton Blue
#define GColorDarkGray   ((GColor){.argb = 0b11010101}) // Mock Dark Gray
#define GColorBlueMoon   ((GColor){.argb = 0b11000110}) // Mock Blue Moon
#define GColorChromeYellow ((GColor){.argb = 0b11111000}) // Mock Chrome Yellow
#define GColorVividCerulean ((GColor){.argb = 0b11001011}) // Mock Vivid Cerulean
#define GColorLightGray  ((GColor){.argb = 0b11101010}) // Mock Light Gray
#define GColorDukeBlue   ((GColor){.argb = 0b11000010}) // Mock Duke Blue
#define GColorClear     ((GColor){.argb = 0b00000000}) // Mock GColorClear

// Mock gcolor_equal function
static inline bool gcolor_equal(GColor c1, GColor c2) {
    return c1.argb == c2.argb;
}

static inline GColor GColorFromHEX(int hex) {
    return (GColor){.argb = (uint8_t) hex};
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

static inline GPoint GPoint_construct(int16_t x, int16_t y) {
    return (GPoint){.x = x, .y = y};
}

#define GPoint(x, y) GPoint_construct(x, y)

static inline GRect GRect_construct(int16_t x, int16_t y, int16_t w, int16_t h) {
    return (GRect){.origin = {.x = x, .y = y}, .size = {.w = w, .h = h}};
}

#define GRect(x, y, w, h) GRect_construct(x, y, w, h)


// Layer mocks
typedef void Layer;
typedef void GContext;
typedef void Window;

typedef void (*LayerUpdateProc)(Layer *layer, GContext *ctx);

static inline void window_set_background_color(Window *window, GColor color) {
    (void) window;
    (void) color;
}

static inline Layer *layer_create(GRect bounds) {
    return (Layer *) malloc(1); // Return dummy heap pointer
}

static inline Layer *layer_create_with_data(GRect bounds, size_t data_size) {
    return (Layer *) malloc(data_size > 0 ? data_size : 1);
}

static inline void layer_set_update_proc(Layer *layer, LayerUpdateProc update_proc) {
    (void) layer;
    (void) update_proc;
}

static inline void layer_add_child(Layer *parent, Layer *child) {
    (void) parent;
    (void) child;
}

static inline void *layer_get_data(const Layer *layer) {
    return (void *) layer;
}

static inline GRect layer_get_bounds(const Layer *layer) {
    (void) layer;
    return (GRect){.origin = {.x = 0, .y = 0}, .size = {.w = 144, .h = 168}}; // Dummy bounds
}

static inline void layer_mark_dirty(Layer *layer) { (void) layer; }
static inline void layer_destroy(Layer *layer) { (void) free(layer); }

// TextLayer mocks
typedef enum {
    GTextAlignmentLeft,
    GTextAlignmentCenter,
    GTextAlignmentRight,
} GTextAlignment;

typedef Layer TextLayer;

#define text_layer_get_layer(text_layer) (Layer *)(text_layer)
#define bitmap_layer_get_layer(bitmap_layer) (Layer *)(bitmap_layer)

static inline TextLayer *text_layer_create(GRect bounds) {
    return (TextLayer *) layer_create(bounds);
}

static inline void text_layer_set_background_color(TextLayer *layer, GColor color) {
    (void) layer;
    (void) color;
}

static inline void text_layer_set_text_color(TextLayer *layer, GColor color) {
    (void) layer;
    (void) color;
}

static inline void text_layer_set_font(TextLayer *layer, GFont font) {
    (void) layer;
    (void) font;
}

static inline void text_layer_set_text_alignment(TextLayer *layer, GTextAlignment alignment) {
    (void) layer;
    (void) alignment;
}

static inline void text_layer_destroy(TextLayer *layer) { free(layer); }


// --- Stateful Mocks for Persistent Storage ---
typedef int32_t status_t;

//! Status codes. See \ref status_t
typedef enum StatusCode {
    //! Operation completed successfully.
    S_SUCCESS = 0,
    E_ERROR = -1,
    E_UNKNOWN = -2,
    E_INTERNAL = -3,
    E_INVALID_ARGUMENT = -4,
    E_OUT_OF_MEMORY = -5,
    E_OUT_OF_STORAGE = -6,
    E_OUT_OF_RESOURCES = -7,
    E_RANGE = -8,
    E_DOES_NOT_EXIST = -9,
    E_INVALID_OPERATION = -10,
    E_BUSY = -11,
    E_AGAIN = -12,
    S_TRUE = 1,
    S_FALSE = 0,
    S_NO_MORE_ITEMS = 2,
    S_NO_ACTION_REQUIRED = 3,
} StatusCode;

// The state of our mock storage
#define MAX_MOCK_STORAGE_ENTRIES 64

typedef struct {
    uint32_t key;
    char data[512]; // Buffer for test data, but limit is enforced in write
    size_t size;
    bool exists;
} MockStorageEntry;

typedef struct {
    MockStorageEntry entries[MAX_MOCK_STORAGE_ENTRIES];
} MockStorage;

static MockStorage global_mock_storage;

// Test helper functions to manipulate the mock storage
static inline void mock_storage_reset() {
    memset(&global_mock_storage, 0, sizeof(MockStorage));
}

static inline MockStorageEntry *find_entry(uint32_t key, bool create_if_missing) {
    for (int i = 0; i < MAX_MOCK_STORAGE_ENTRIES; i++) {
        if (global_mock_storage.entries[i].exists && global_mock_storage.entries[i].key == key) {
            return &global_mock_storage.entries[i];
        }
    }
    if (create_if_missing) {
        for (int i = 0; i < MAX_MOCK_STORAGE_ENTRIES; i++) {
            if (!global_mock_storage.entries[i].exists) {
                global_mock_storage.entries[i].key = key;
                global_mock_storage.entries[i].exists = true;
                return &global_mock_storage.entries[i];
            }
        }
    }
    return NULL;
}

static inline void mock_storage_set_version(int version) {
    MockStorageEntry *entry = find_entry(2, true); // PERSIST_KEY_SETTINGS_VERSION
    if (entry) {
        memcpy(entry->data, &version, sizeof(int));
        entry->size = sizeof(int);
    }
}

static inline int mock_storage_get_version() {
    MockStorageEntry *entry = find_entry(2, false);
    if (entry && entry->size == sizeof(int)) {
        int val;
        memcpy(&val, entry->data, sizeof(int));
        return val;
    }
    return 0;
}

static inline void mock_storage_set_data(void *data, size_t size) {
    MockStorageEntry *entry = find_entry(1, true); // PERSIST_KEY_SETTINGS
    if (entry) {
        memcpy(entry->data, data, size);
        entry->size = size;
    }
}

// The mock implementations of the Pebble SDK functions
static inline bool persist_exists(const uint32_t key) {
    return find_entry(key, false) != NULL;
}

static inline status_t persist_delete(const uint32_t key) {
    MockStorageEntry *entry = find_entry(key, false);
    if (entry) {
        entry->exists = false;
        return S_TRUE;
    }
    return S_FALSE;
}

static inline int persist_read_int(const uint32_t key) {
    MockStorageEntry *entry = find_entry(key, false);
    if (entry && entry->size == sizeof(int32_t)) {
        int32_t val;
        memcpy(&val, entry->data, sizeof(int32_t));
        return val;
    }
    return 0;
}

static inline int persist_read_data(const uint32_t key, void *buffer, const size_t size) {
    MockStorageEntry *entry = find_entry(key, false);
    if (entry) {
        size_t to_copy = size < entry->size ? size : entry->size;
        memcpy(buffer, entry->data, to_copy);
        return (int) to_copy;
    }
    return 0;
}

static inline int persist_write_data(const uint32_t key, const void *data, const size_t size) {
    if (size > 256) return E_OUT_OF_RESOURCES; // Simulate Pebble limit!
    MockStorageEntry *entry = find_entry(key, true);
    if (entry) {
        memcpy(entry->data, data, size);
        entry->size = size;
        return (int) size;
    }
    return 0;
}

static inline status_t persist_write_int(const uint32_t key, const int32_t value) {
    MockStorageEntry *entry = find_entry(key, true);
    if (entry) {
        memcpy(entry->data, &value, sizeof(int32_t));
        entry->size = sizeof(int32_t);
        return S_SUCCESS;
    }
    return E_INTERNAL;
}

// --- End of Storage Mocks ---


// AppMessage mocks
typedef int AppMessageResult;
#define APP_MSG_OK 0
#define APP_MSG_INTERNAL_ERROR 1

typedef struct Tuple {
    uint32_t message_key;
    uint8_t type;
    uint16_t length;

    struct {
        int32_t int32;
        char cstring[512]; // Larger buffer for forecast strings
    } value[1]; // Use array of size 1 to allow tuple->value->field access
} Tuple;

#define TUPLE_CSTRING 1
#define TUPLE_INT 2

typedef struct DictionaryIterator DictionaryIterator; // Opaque type

typedef Tuple *(*DictFindFunc)(const DictionaryIterator *, uint32_t);

static DictFindFunc s_dict_find_func = NULL;

static inline Tuple *dict_find(const DictionaryIterator *iter, uint32_t key) {
    if (s_dict_find_func) return s_dict_find_func(iter, key);
    return NULL;
}

static inline AppMessageResult app_message_outbox_begin(DictionaryIterator **iter) {
    if (iter) *iter = (DictionaryIterator *) 1;
    return APP_MSG_OK;
}

static inline void dict_write_int(DictionaryIterator *iter, uint32_t key, const void *value, size_t size, bool signed_val) {
    (void) iter;
    (void) key;
    (void) value; (void) size; (void) signed_val;
}

static inline void dict_write_uint8(DictionaryIterator *iter, uint32_t key, uint8_t value) {
    (void) iter; (void) key; (void) value;
}

static inline void dict_write_end(DictionaryIterator *iter) {
    (void) iter;
}

static int s_app_message_outbox_send_count = 0;

static inline AppMessageResult app_message_outbox_send() {
    s_app_message_outbox_send_count++;
    return APP_MSG_OK;
}

// Tick Timer Service mocks
typedef enum {
    SECOND_UNIT = 1 << 0,
    MINUTE_UNIT = 1 << 1,
    HOUR_UNIT = 1 << 2,
    DAY_UNIT = 1 << 3,
    MONTH_UNIT = 1 << 4,
    YEAR_UNIT = 1 << 5
} TimeUnits;

typedef void (*TickHandler)(struct tm *tick_time, TimeUnits units_changed);

static TickHandler s_tick_handler = NULL;
static TimeUnits s_tick_units = 0;

static inline void tick_timer_service_subscribe(TimeUnits tick_units, TickHandler handler) {
    s_tick_handler = handler;
    s_tick_units = tick_units;
}

static inline void tick_timer_service_unsubscribe() {
    s_tick_handler = NULL;
    s_tick_units = 0;
}

// Connection Service mocks
typedef void (*PebbleAppConnectionHandler)(bool connected);

typedef struct {
    PebbleAppConnectionHandler pebble_app_connection_handler;
    void *pebblekit_connection_handler; // unused in this project
} ConnectionHandlers;

static PebbleAppConnectionHandler s_connection_handler = NULL;
static bool s_mock_connected = true;

static inline bool connection_service_peek_pebble_app_connection() {
    return s_mock_connected;
}

static inline void connection_service_subscribe(ConnectionHandlers handlers) {
    s_connection_handler = handlers.pebble_app_connection_handler;
}

static inline void connection_service_unsubscribe() {
    s_connection_handler = NULL;
}

// Battery mocks
typedef struct BatteryChargeState {
    uint8_t charge_percent;
    bool is_charging;
    bool is_plugged;
} BatteryChargeState;

typedef void (*BatteryHandler)(BatteryChargeState charge);

static BatteryHandler s_battery_handler = NULL;
static BatteryChargeState s_battery_state = {100, false, false};

static inline void battery_state_service_subscribe(BatteryHandler handler) {
    s_battery_handler = handler;
}

static inline void battery_state_service_unsubscribe() {
    s_battery_handler = NULL;
}

static inline BatteryChargeState battery_state_service_peek() {
    return s_battery_state;
}

// Health mocks
#define PBL_HEALTH 1

typedef enum {
    HealthEventSignificantUpdate,
    HealthEventMovementUpdate,
    HealthEventSleepUpdate,
    HealthEventHeartRateUpdate,
    HealthEventMetricAlert
} HealthEventType;

typedef enum {
    HealthMetricHeartRateBPM,
    HealthMetricStepCount
} HealthMetric;

typedef void (*HealthEventHandler)(HealthEventType event, void *context);

static HealthEventHandler s_health_handler = NULL;
static int s_mock_step_count = 0;
static int s_mock_heart_rate = 0;

static inline bool health_service_events_subscribe(HealthEventHandler handler, void *context) {
    s_health_handler = handler;
    return true;
}

static inline void health_service_events_unsubscribe() {
    s_health_handler = NULL;
}

static inline int health_service_peek_current_value(HealthMetric metric) {
    if (metric == HealthMetricHeartRateBPM) return s_mock_heart_rate;
    if (metric == HealthMetricStepCount) return s_mock_step_count;
    return 0;
}

static inline int health_service_sum_today(HealthMetric metric) {
    if (metric == HealthMetricStepCount) return s_mock_step_count;
    return 0;
}

typedef void (*AppMessageInboxReceived)(DictionaryIterator *iterator, void *context);
typedef void (*AppMessageInboxDropped)(AppMessageResult reason, void *context);
typedef void (*AppMessageOutboxFailed)(DictionaryIterator *iterator, AppMessageResult reason, void *context);
typedef void (*AppMessageOutboxSent)(DictionaryIterator *iterator, void *context);

static inline void app_message_register_inbox_received(AppMessageInboxReceived handler) { (void) handler; }
static inline void app_message_register_inbox_dropped(AppMessageInboxDropped handler) { (void) handler; }
static inline void app_message_register_outbox_failed(AppMessageOutboxFailed handler) { (void) handler; }
static inline void app_message_register_outbox_sent(AppMessageOutboxSent handler) { (void) handler; }

static inline AppMessageResult app_message_open(uint32_t inbox_size, uint32_t outbox_size) {
    (void) inbox_size; (void) outbox_size;
    return APP_MSG_OK;
}

// AppTimer mocks
typedef void AppTimer;
typedef void (*AppTimerCallback)(void *data);

static inline AppTimer *app_timer_register(uint32_t timeout_ms, AppTimerCallback callback, void *data) {
    (void) timeout_ms; (void) callback; (void) data;
    return (AppTimer *) 1; // Return a dummy non-NULL timer
}

static inline bool app_timer_cancel(AppTimer *timer) {
    (void) timer;
    return true;
}

// Graphics mocks for pixel_matrix_drawer and graph_utils
typedef uint8_t GCorners;
#define GCornerNone 0

// --- Mocks for graphics_fill_rect ---
#define MAX_GRAPHICS_FILL_RECT_CALLS 2000

typedef struct {
    GRect rect;
    uint16_t corner_radius;
    GCorners corners;
    GColor fill_color;
} GraphicsFillRectCall;

static GraphicsFillRectCall s_graphics_fill_rect_calls[MAX_GRAPHICS_FILL_RECT_CALLS];
static int s_graphics_fill_rect_call_count = 0;
static GColor s_current_fill_color = {.argb = 0};

static inline void graphics_context_set_fill_color(GContext *ctx, GColor color) {
    (void) ctx;
    s_current_fill_color = color;
}

static inline void graphics_fill_rect(GContext *ctx, GRect rect, uint16_t corner_radius, GCorners corners) {
    (void) ctx;
    if (s_graphics_fill_rect_call_count < MAX_GRAPHICS_FILL_RECT_CALLS) {
        s_graphics_fill_rect_calls[s_graphics_fill_rect_call_count++] = (GraphicsFillRectCall){
            .rect = rect, .corner_radius = corner_radius, .corners = corners, .fill_color = s_current_fill_color
        };
    }
}

static inline void graphics_draw_rect(GContext *ctx, GRect rect) {
    (void) ctx; (void) rect;
}

static inline void reset_graphics_fill_rect_calls() {
    s_graphics_fill_rect_call_count = 0;
    s_current_fill_color = (GColor){.argb = 0};
}

static inline GraphicsFillRectCall *get_graphics_fill_rect_calls() { return s_graphics_fill_rect_calls; }
static inline int get_graphics_fill_rect_call_count() { return s_graphics_fill_rect_call_count; }

// --- Mocks for graphics_draw_line ---
#define MAX_GRAPHICS_DRAW_LINE_CALLS 2000

typedef struct {
    GPoint p0;
    GPoint p1;
    GColor stroke_color;
} GraphicsDrawLineCall;

static GraphicsDrawLineCall s_graphics_draw_line_calls[MAX_GRAPHICS_DRAW_LINE_CALLS];
static int s_graphics_draw_line_call_count = 0;
static GColor s_current_stroke_color = {.argb = 0};

static inline void graphics_context_set_stroke_color(GContext *ctx, GColor color) {
    (void) ctx;
    s_current_stroke_color = color;
}

static inline void graphics_draw_line(GContext *ctx, GPoint p0, GPoint p1) {
    (void) ctx;
    if (s_graphics_draw_line_call_count < MAX_GRAPHICS_DRAW_LINE_CALLS) {
        s_graphics_draw_line_calls[s_graphics_draw_line_call_count++] = (GraphicsDrawLineCall){
            .p0 = p0, .p1 = p1, .stroke_color = s_current_stroke_color
        };
    }
}

static inline void reset_graphics_draw_line_calls() {
    s_graphics_draw_line_call_count = 0;
    s_current_stroke_color = (GColor){.argb = 0};
}

static inline GraphicsDrawLineCall *get_graphics_draw_line_calls() { return s_graphics_draw_line_calls; }
static inline int get_graphics_draw_line_call_count() { return s_graphics_draw_line_call_count; }

// Animation mocks
typedef void Animation;
typedef uint32_t AnimationProgress;
#define ANIMATION_NORMALIZED_MAX 65535
#define ANIMATION_DURATION_INFINITE 0xFFFFFFFF

typedef enum {
    AnimationCurveLinear,
    AnimationCurveEaseIn,
    AnimationCurveEaseOut,
    AnimationCurveEaseInOut,
} AnimationCurve;

typedef void (*AnimationUpdateImplementation)(Animation *animation, const AnimationProgress progress);

typedef struct {
    AnimationUpdateImplementation update;
} AnimationImplementation;

static inline Animation *animation_create() { return (Animation *)malloc(1); }
static inline void animation_destroy(Animation *animation) { if (animation) free(animation); }
static inline void animation_set_duration(Animation *animation, uint32_t duration_ms) { (void)animation; (void)duration_ms; }
static inline void animation_set_delay(Animation *animation, uint32_t delay_ms) { (void)animation; (void)delay_ms; }
static inline void animation_set_curve(Animation *animation, AnimationCurve curve) { (void)animation; (void)curve; }
static inline void animation_set_implementation(Animation *animation, AnimationImplementation *implementation) { (void)animation; (void)implementation; }
static inline void animation_set_play_count(Animation *animation, uint32_t play_count) { (void)animation; (void)play_count; }
static inline void animation_schedule(Animation *animation) { (void)animation; }
static inline void animation_unschedule(Animation *animation) { (void)animation; }

// Clock mocks
static bool s_clock_is_24h_style = true;
static inline bool clock_is_24h_style() { return s_clock_is_24h_style; }

// Window mocks
typedef void Window;
static inline Window *window_create() { return (Window *)malloc(1); }
static inline void window_destroy(Window *window) { free(window); }
static inline Layer *window_get_root_layer(const Window *window) { return (Layer *)window; }
static inline void window_stack_push(Window *window, bool animated) { (void)window; (void)animated; }
typedef struct {
    void (*load)(Window *window);
    void (*unload)(Window *window);
    void (*appear)(Window *window);
    void (*disappear)(Window *window);
} WindowHandlers;
static inline void window_set_window_handlers(Window *window, WindowHandlers handlers) { (void)window; (void)handlers; }

// App lifecycle mocks
static inline void app_event_loop() {}
static inline void light_enable(bool enable) { (void)enable; }

