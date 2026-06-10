#pragma once
#include <pebble.h>
#include "../ui/layer_factory.h"
#include "../settings/persist_keys.h"

// Persistent storage key
#define WEATHER_DATA_KEY PERSIST_KEY_WEATHER_DATA
#define WEATHER_FORECAST_MAX_POINTS 100

typedef struct WeatherData {
    int CurrentTemperature;
    int MaxTemperature;
    int MinTemperature;
    int RainNextHourMmX10; // millimeters * 10
    int RainPopPercent; // 0..100
    int TemperatureForecastCount;
    int *TemperatureForecast;
    int RainForecastMmX10Count;
    int *RainForecastMmX10;
    bool is_temp_forecast_dynamic_alloc;
    bool is_rain_forecast_dynamic_alloc;
    char CurrentConditions[48];
} WeatherData;

WeatherData *weather_get_data();

// update all weather layer instances (backward compatible wrapper)
void update_weather();

// create the layer (returns the created layer)
Layer *create_weather_layer(LayerBuilder builder);

// destroy the layer
void destroy_weather_layer(Layer * layer);

void deinit_weather_data();
