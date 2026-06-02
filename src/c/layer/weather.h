#pragma once
#include <pebble.h>
#include "../layer_factory.h"
#include "../persist_keys.h"

// Persistent storage key
#define WEATHER_DATA_KEY PERSIST_KEY_WEATHER_DATA

typedef struct WeatherData {
    int CurrentTemperature;
    int MaxTemperature;
    int MinTemperature;
    int RainNextHourMmX10; // millimeters * 10
    int RainPopPercent; // 0..100
    char CurrentConditions[32];
} __attribute__((__packed__)

)
WeatherData;

WeatherData *weather_get_data();

// update all weather layer instances (backward compatible wrapper)
void update_weather();

// create the layer (returns the created layer)
Layer *create_weather_layer(LayerBuilder builder);

// destroy the layer
void destroy_weather_layer(Layer * layer);