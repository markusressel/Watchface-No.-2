#include "time.h"
#include "theme.h"
#include "clay_settings.h"
#include "layer_factory.h"

// Time TextLayer
static TextLayer *s_time_layer;

static ClaySettings *s_settings;

static char s_time_format[16];

// Method to update the time textbuffer
void update_time() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the current hours and minutes into a buffer
    static char s_buffer[16];
    strftime(
        s_buffer,
        sizeof(s_buffer),
        s_time_format,
        tick_time
    );

    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, s_buffer);
}

// creates the time layer
void create_time_layer(LayerBuilder builder) {
    s_settings = clay_get_settings();

    strcpy(s_time_format, clock_is_24h_style() ? "%H:%M" : "%I:%M");
    if (s_settings->ShowSeconds) {
        strcat(s_time_format, ":%S");
    }

    s_time_layer = layer_factory_create_text_layer(builder, (TextLayerStyle){
        .background_color = GColorClear,
        .text_color = theme_get_theme()->TimeTextColor,
        .font = theme_get_theme()->TimeFont,
        .alignment = GTextAlignmentRight,
    });

    // update time value before rendering so it is shown right from the beginning
    update_time();
}

// destroys the time layer
void destroy_time_layer() {
    text_layer_destroy(s_time_layer);
}
