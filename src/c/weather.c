#include "weather.h"
#include "clay_settings.h"
#include "theme.h"
#include "dotted_text_layer.h"

static char s_buffer[32];

static WeatherData weatherData;

static ClaySettings *s_settings;

// Timer to update weather after given amount of time
static int s_weather_update_interval = 1800000;
//static int s_weather_update_interval = 30000;
static AppTimer *s_update_timer;

// Weather DottedTextLayer
static DottedTextLayer *s_dotted_text_layer;

WeatherData* weather_get_data() {
  return &weatherData;
}

static void restore_saved_weather_data() {
  // Read settings from persistent storage, if they exist
  persist_read_data(WEATHER_DATA_KEY, &weatherData, sizeof(weatherData));
}

static void save_current_weather_data() {
  // save WeatherData struct to persistent storage
  persist_write_data(WEATHER_DATA_KEY, &weatherData, sizeof(weatherData));
}

static void request_weather_update() {
  // Declare the dictionary's iterator
  DictionaryIterator *out_iter;

  // Prepare the outbox buffer for this message
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  if(result != APP_MSG_OK) {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
    return;
  }
  
  // Construct the message with a dummy value
  int value = 0;
  // Add an item to ask for weather data
  dict_write_int(out_iter, MESSAGE_KEY_RequestData, &value, sizeof(int), true);
  
  APP_LOG(APP_LOG_LEVEL_ERROR, "Sending RequestUpdate message for weather...");
  
  // Send this message
  result = app_message_outbox_send();

  // Check the result
  if(result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
  }
}

static void on_scheduled_update_triggered() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "scheduled weather update triggered!");
  
  if (s_update_timer) {
    // cancel weather update timer
    app_timer_cancel(s_update_timer);
  }
  
  // send AppMessage to trigger weather update via JS
  request_weather_update();
  
  //Register next execution
  s_update_timer = app_timer_register(s_weather_update_interval, (AppTimerCallback) on_scheduled_update_triggered, NULL);
}

void update_weather(){
  APP_LOG(APP_LOG_LEVEL_DEBUG, "updating weather with new data");
  
  // persist current data for fast access when opening the watchface
  save_current_weather_data();
  
  // Write the current temperature into a buffer
  snprintf(s_buffer, sizeof(s_buffer), "%d|%d|%d", weatherData.CurrentTemperature, weatherData.MinTemperature, weatherData.MaxTemperature);
  // update text layer
  dotted_text_layer_set_text(s_dotted_text_layer, s_buffer);
}

void create_weather_layer(Window *window){
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  s_settings = clay_get_settings();
  
  restore_saved_weather_data();
  
  // set bounds and offset for text layer
  int width = bounds.size.w - 5;
  int height = 27;
  int offsetX = 0;
  int offsetY = 5;
  
  GRect layer_bounds = GRect(offsetX, offsetY, width, height);
  
  s_dotted_text_layer = dotted_text_layer_create(layer_bounds);
  dotted_text_layer_set_color(s_dotted_text_layer, theme_get_theme()->WeatherTextColor);
  dotted_text_layer_set_text(s_dotted_text_layer, "---");
  dotted_text_layer_set_align_right(s_dotted_text_layer, true);
  
  update_weather();
  
  s_update_timer = app_timer_register(s_weather_update_interval, (AppTimerCallback) on_scheduled_update_triggered, NULL);
  
  layer_add_child(window_layer, s_dotted_text_layer);
}

void destroy_weather_layer(){
  if (s_update_timer) {
    // cancel weather update timer
    app_timer_cancel(s_update_timer);
  }
  
  dotted_text_layer_destroy(s_dotted_text_layer);
}