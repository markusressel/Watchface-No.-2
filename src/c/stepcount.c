#include <pebble.h>
#include "stepcount.h"
#include "health_listener.h"
#include "clay_settings.h"
#include "theme.h"
#include "dotted_text_layer.h"

static ClaySettings *s_settings;

// Date DottedTextLayer
static DottedTextLayer *s_dotted_text_layer;

void update_stepcount() {
  // Write the current day, month and year into a buffer
  static char s_buffer[16];
  snprintf(s_buffer, sizeof(s_buffer), "%d", (int)s_step_count);

  // Display this date on the DottedTextLayer
  dotted_text_layer_set_text(s_dotted_text_layer, s_buffer);
}

// creates the date layer
void create_stepcount_layer(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  s_settings = clay_get_settings();
  
  int width = bounds.size.w - 5;
  int height = 27;
  int offsetX = 0;
  // int offsetX = (bounds.size.w - width); // right aligned
  int offsetY = bounds.size.w - 27 - 5 - 5 - 2;
  
  GRect layer_bounds = GRect(offsetX, offsetY, width, height);
  
  s_dotted_text_layer = dotted_text_layer_create(layer_bounds);
  //dotted_text_layer_set_text(s_dotted_text_layer, "10000");
  dotted_text_layer_set_color(s_dotted_text_layer, GColorBlack);
  dotted_text_layer_set_align_right(s_dotted_text_layer, true);
  
  s_step_count = (int) health_service_sum_today(HealthMetricStepCount);
  update_stepcount();

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, s_dotted_text_layer);
}

// destroys the date layer
void destroy_stepcount_layer() {
  dotted_text_layer_destroy(s_dotted_text_layer);
}