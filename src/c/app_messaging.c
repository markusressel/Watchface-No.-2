#include <pebble.h>
#include "app_messaging.h"
#include "weather.h"
#include "clay_settings.h"

static ClaySettings *s_settings;

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read clay configuration properties
  
  s_settings = clay_get_settings();
  
  // Read theme
  Tuple *theme_t = dict_find(iterator, MESSAGE_KEY_Theme);
  if(theme_t) {
    char *theme = theme_t->value->cstring;
    strcpy(s_settings->ThemeValue, theme);
  }
  
  // Read color preferences
  Tuple *bg_color_t = dict_find(iterator, MESSAGE_KEY_BackgroundColor);
  if(bg_color_t) {
    GColor bg_color = GColorFromHEX(bg_color_t->value->int32);
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "bg_color: old: %lu new: %lu", settings.BackgroundColor, bg_color);
    s_settings->BackgroundColor = bg_color;
  }
  
  // Time Layer Colors
  Tuple *text_color_time_t = dict_find(iterator, MESSAGE_KEY_TimeTextColor);
  if(text_color_time_t) {
    GColor text_color_time = GColorFromHEX(text_color_time_t->value->int32);
    s_settings->TimeTextColor = text_color_time;
  }
  
  // Date Layer Colors
  Tuple *text_color_date_t = dict_find(iterator, MESSAGE_KEY_DateTextColor);
  if(text_color_date_t) {
    GColor text_color_date = GColorFromHEX(text_color_date_t->value->int32);
    s_settings->DateTextColor = text_color_date;
  }
  
  // Battery Bar Layer Colors
  Tuple *battery_frame_color_t = dict_find(iterator, MESSAGE_KEY_BatteryFrameColor);
  if(battery_frame_color_t) {
    GColor battery_frame_color = GColorFromHEX(battery_frame_color_t->value->int32);
    s_settings->BatteryFrameColor = battery_frame_color;
  }
  
  Tuple *battery_fill_color_t = dict_find(iterator, MESSAGE_KEY_BatteryFillColor);
  if(battery_fill_color_t) {
    GColor battery_fill_color = GColorFromHEX(battery_fill_color_t->value->int32);
    s_settings->BatteryFillColor = battery_fill_color;
  }
  
  // Weather Layer Colors
  Tuple *weather_text_color_t = dict_find(iterator, MESSAGE_KEY_WeatherTextColor);
  if(weather_text_color_t) {
    GColor weather_text_color = GColorFromHEX(weather_text_color_t->value->int32);
    s_settings->WeatherTextColor = weather_text_color;
  }
  
  // Stepcount Layer Colors
  Tuple *stepcount_text_color_t = dict_find(iterator, MESSAGE_KEY_StepcountTextColor);
  if(stepcount_text_color_t) {
    GColor stepcount_text_color = GColorFromHEX(stepcount_text_color_t->value->int32);
    s_settings->StepcountTextColor = stepcount_text_color;
  }

  // Read boolean preferences
  Tuple *show_seconds_t = dict_find(iterator, MESSAGE_KEY_ShowSeconds);
  if(show_seconds_t) {
    bool show_seconds = show_seconds_t->value->int32 == 1;
    s_settings->ShowSeconds = show_seconds;
  }
  
  // Read weather data
  
  // Read tuples for data
  Tuple *temp_cur_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_TEMPERATURE_CURRENT);
  Tuple *temp_min_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_TEMPERATURE_MIN);
  Tuple *temp_max_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_TEMPERATURE_MAX);
  Tuple *condition_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_CONDITION);
  
  // If all data is available, use it
  if(temp_cur_tuple && temp_min_tuple && temp_max_tuple && condition_tuple) {
    WeatherData *weatherData = weather_get_data();
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "temp changed: old: %d new: %d",(int)  weatherData->CurrentTemperature, (int)temp_cur_tuple->value->int32);
    
    weatherData->CurrentTemperature = (int)temp_cur_tuple->value->int32;
    weatherData->MinTemperature = (int)temp_min_tuple->value->int32;
    weatherData->MaxTemperature = (int)temp_max_tuple->value->int32;
    strcpy(weatherData->CurrentConditions, condition_tuple->value->cstring);
    
    update_weather();
  }
  
  // persist data
  clay_save_settings();
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped! %d", (int) reason);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! %d", (int) reason);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void initialize_app_messaging() {
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  const int inbox_size = 256;
  const int outbox_size = 256;
  app_message_open(inbox_size, outbox_size);
}