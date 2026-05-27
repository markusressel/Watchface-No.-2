#pragma once
#include <pebble.h>
#include "layer_factory.h"

// Persistent storage key
#define WEATHER_DATA_KEY 2

typedef struct WeatherData {
  int CurrentTemperature;
  int MaxTemperature;
  int MinTemperature;
  char CurrentConditions[32];
} __attribute__((__packed__)) WeatherData;

WeatherData *weather_get_data();

// update layer information
void update_weather();

// create the layer
void create_weather_layer(LayerBuilder builder);

// destroy the layer
void destroy_weather_layer();
