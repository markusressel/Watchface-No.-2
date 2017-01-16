#include <pebble.h>
#include "date.h"
#include "clay_settings.h"
#include "theme.h"
#include "dotted_text_layer.h"

static ClaySettings *s_settings;

// Date DottedTextLayer
static DottedTextLayer *s_dotted_text_layer;

static char *s_date_format;

void update_date() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current day, month and year into a buffer
  static char s_buffer[16];
  strftime(s_buffer, 
           sizeof(s_buffer),
           s_date_format,
           tick_time);

  // Remove the third character of weekday abbreviation
  if (s_settings->ShowWeekdayAbbreviation) {
    int idxToDel = 2; 
    memmove(&s_buffer[idxToDel], &s_buffer[idxToDel + 1], strlen(s_buffer) - idxToDel);
  }
  
  // Display this date on the DottedTextLayer
  dotted_text_layer_set_text(s_dotted_text_layer, s_buffer);
}

// creates the date layer
void create_date_layer(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  s_settings = clay_get_settings();
  
  if (s_settings->ShowWeekdayAbbreviation) {
    s_date_format = "%a ";
  } else {
    s_date_format = "";
  }
  
  strcat(s_date_format, "%d.%m");
  
  if (s_settings->ShowYear) {
    strcat(s_date_format, ".%y");
  }
  
  int width = bounds.size.w - 3;
  int height = 27;
  int offsetX = 0;
  // int offsetX = (bounds.size.w - width); // right aligned
  int offsetY = 27 + 5 + 5;
  
  GRect layer_bounds = GRect(offsetX, offsetY, width, height);
  
  s_dotted_text_layer = dotted_text_layer_create(layer_bounds);
  //dotted_text_layer_set_text(s_dotted_text_layer, "14.01.17");
  dotted_text_layer_set_color(s_dotted_text_layer, theme_get_theme()->DateTextColor);
  dotted_text_layer_set_align_right(s_dotted_text_layer, true);
  
  update_date();

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, s_dotted_text_layer);
}

// destroys the date layer
void destroy_date_layer() {
  dotted_text_layer_destroy(s_dotted_text_layer);
}