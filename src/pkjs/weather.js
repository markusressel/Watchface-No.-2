var config = require('./config');

var WEATHER_UPDATE_INTERVAL_MINUTES = 30;
var WEATHER_UPDATE_INTERVAL_MS = WEATHER_UPDATE_INTERVAL_MINUTES * 60 * 1000;
var WEATHER_LAST_FETCH_KEY = 'weather-last-fetch-ts';
var WEATHER_LAST_DATA_KEY = 'weather-last-data';
var FORECAST_POINT_COUNT = 10;

function kelvin_to_celsius(kelvin) {
    if (typeof kelvin !== 'number') {
        return 0;
    }

    return Math.round(kelvin - 273.15);
}

function one_decimal_to_int(value) {
    if (typeof value !== 'number') {
        return 0;
    }

    return Math.round(value * 10);
}

function get_cached_weather_data() {
    try {
        var cached = localStorage.getItem(WEATHER_LAST_DATA_KEY);
        return cached ? JSON.parse(cached) : null;
    } catch (e) {
        return null;
    }
}

function cache_weather_data(dictionary) {
    try {
        localStorage.setItem(WEATHER_LAST_DATA_KEY, JSON.stringify(dictionary));
        localStorage.setItem(WEATHER_LAST_FETCH_KEY, String(Date.now()));
    } catch (e) {
        console.log('Could not cache weather data: ' + e);
    }
}

function should_fetch_weather_from_api() {
    var cached = get_cached_weather_data();
    if (!cached) {
        return true;
    }

    var lastFetchRaw = localStorage.getItem(WEATHER_LAST_FETCH_KEY);
    var lastFetch = parseInt(lastFetchRaw, 10);
    if (!lastFetchRaw || isNaN(lastFetch)) {
        return true;
    }

    return (Date.now() - lastFetch) >= WEATHER_UPDATE_INTERVAL_MS;
}

function send_weather_to_watch(dictionary, successMessage, errorMessage) {
    Pebble.sendAppMessage(
        dictionary,
        function (e) {
            console.log(successMessage);
        },
        function (e) {
            console.log(errorMessage);
        }
    );
}

function pick_closest_entry_to_now(entries) {
    if (!entries || entries.length === 0) {
        return null;
    }

    var nowUtc = Math.floor(Date.now() / 1000);
    var best = entries[0];
    var bestDistance = Math.abs((best.dt || 0) - nowUtc);

    for (var i = 1; i < entries.length; i++) {
        var candidate = entries[i];
        var distance = Math.abs((candidate.dt || 0) - nowUtc);
        if (distance < bestDistance) {
            best = candidate;
            bestDistance = distance;
        }
    }

    return best;
}

function local_day_index(utcTimestamp, timezoneOffsetSeconds) {
    return Math.floor((utcTimestamp + timezoneOffsetSeconds) / 86400);
}

function day_min_max_from_timeline(entries, referenceEntry, timezoneOffsetSeconds) {
    if (!entries || entries.length === 0) {
        return {minKelvin: undefined, maxKelvin: undefined};
    }

    var reference = referenceEntry || entries[0];
    var referenceDay = local_day_index(reference.dt || 0, timezoneOffsetSeconds || 0);
    var minKelvin;
    var maxKelvin;

    for (var i = 0; i < entries.length; i++) {
        var entry = entries[i];
        var t = entry && entry.temp;
        if (typeof t !== 'number') {
            continue;
        }

        if (local_day_index(entry.dt || 0, timezoneOffsetSeconds || 0) !== referenceDay) {
            continue;
        }

        if (typeof minKelvin !== 'number' || t < minKelvin) {
            minKelvin = t;
        }
        if (typeof maxKelvin !== 'number' || t > maxKelvin) {
            maxKelvin = t;
        }
    }

    if (typeof minKelvin !== 'number' || typeof maxKelvin !== 'number') {
        for (var j = 0; j < entries.length; j++) {
            var fallbackTemp = entries[j] && entries[j].temp;
            if (typeof fallbackTemp !== 'number') {
                continue;
            }
            if (typeof minKelvin !== 'number' || fallbackTemp < minKelvin) {
                minKelvin = fallbackTemp;
            }
            if (typeof maxKelvin !== 'number' || fallbackTemp > maxKelvin) {
                maxKelvin = fallbackTemp;
            }
        }
    }

    return {minKelvin: minKelvin, maxKelvin: maxKelvin};
}

function build_condensed_series(entries, extractor, count) {
    var samples = [];
    if (!entries || entries.length === 0) {
        return samples;
    }

    var targetCount = count;
    if (entries.length <= targetCount) {
        for (var i = 0; i < entries.length; i++) {
            samples.push(extractor(entries[i]));
        }
        return samples;
    }

    for (var n = 0; n < targetCount; n++) {
        var idx = Math.round((n * (entries.length - 1)) / (targetCount - 1));
        samples.push(extractor(entries[idx]));
    }

    return samples;
}

function encode_number_array(values) {
    return values.join(',');
}

var xhrRequest = function (url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
        callback(this.responseText);
    };
    xhr.open(type, url);
    xhr.send();
};

function locationSuccess(pos) {
    // Construct URL
    var url = "https://api.openweathermap.org/data/4.0/onecall/timeline/15min?lat=" +
        pos.coords.latitude +
        "&lon=" + pos.coords.longitude +
        "&appid=" + config.getWeatherApiKey();

    console.log("Weather request URL is: " + url);

    // Send request to OpenWeatherMap
    xhrRequest(url, 'GET',
        function (responseText) {
            // responseText contains a JSON object with weather info
            var json = JSON.parse(responseText);

            console.log("JSON response is: " + JSON.stringify(json));

            var timeline = json.data || [];
            var current = pick_closest_entry_to_now(timeline) || timeline[0] || {};
            var minMax = day_min_max_from_timeline(timeline, current, json.timezone_offset || 0);
            var minKelvin = minMax.minKelvin;
            var maxKelvin = minMax.maxKelvin;

            // Temperature in Kelvin requires adjustment
            var temperatureCurrent = kelvin_to_celsius(current.temp);
            console.log("Current Temperature is " + temperatureCurrent);
            var temperatureMin = kelvin_to_celsius(minKelvin);
            console.log("Min Temperature is " + temperatureMin);
            var temperatureMax = kelvin_to_celsius(maxKelvin);
            console.log("Max Temperature is " + temperatureMax);

            // Conditions
            var conditions = "";
            if (current.weather && current.weather.length > 0 && current.weather[0].main) {
                conditions = current.weather[0].main;
            }
            console.log("Conditions are " + conditions);

            // Rain forecast based on selected 15-minute entry
            var rainMm = (current.rain && current.rain["1h"]) ? current.rain["1h"] : 0;
            var popPercent = typeof current.pop === 'number' ? Math.round(current.pop * 100) : 0;
            console.log("Rain from selected entry (mm/h): " + rainMm + ", pop (%): " + popPercent);

            var temperatureForecastSeries = build_condensed_series(
                timeline,
                function (entry) {
                    return kelvin_to_celsius(entry && entry.temp);
                },
                FORECAST_POINT_COUNT
            );

            var rainForecastSeries = build_condensed_series(
                timeline,
                function (entry) {
                    var rain = (entry && entry.rain && entry.rain['1h']) ? entry.rain['1h'] : 0;
                    return one_decimal_to_int(rain);
                },
                FORECAST_POINT_COUNT
            );

            // Assemble dictionary using our keys
            var dictionary = {
                "WEATHER_TEMPERATURE_CURRENT": temperatureCurrent,
                "WEATHER_TEMPERATURE_MIN": temperatureMin,
                "WEATHER_TEMPERATURE_MAX": temperatureMax,
                "WEATHER_CONDITION": conditions,
                "WEATHER_RAIN_NEXT_HOUR_MM_X10": one_decimal_to_int(rainMm),
                "WEATHER_RAIN_POP_PERCENT": popPercent,
                "WEATHER_TEMP_FORECAST_ENCODED": encode_number_array(temperatureForecastSeries),
                "WEATHER_RAIN_FORECAST_MM_X10_ENCODED": encode_number_array(rainForecastSeries)
            };

            cache_weather_data(dictionary);

            // Send to Pebble
            send_weather_to_watch(
                dictionary,
                "Weather info sent to Pebble successfully!",
                "Error sending weather info to Pebble!"
            );
        }
    );
}

function locationError(err) {
    console.log("Error requesting location!");
}

function getWeather() {
    var apiKey = config.getWeatherApiKey();

    // If no API key is configured, clear any stale weather data
    if (!apiKey || apiKey.length === 0) {
        console.log("No OpenWeatherMap API key configured. Clearing weather data.");

        // Send empty weather data to clear the display on the watchface
        var clearDictionary = {
            "WEATHER_TEMPERATURE_CURRENT": 0,
            "WEATHER_TEMPERATURE_MIN": 0,
            "WEATHER_TEMPERATURE_MAX": 0,
            "WEATHER_CONDITION": "",
            "WEATHER_RAIN_NEXT_HOUR_MM_X10": 0,
            "WEATHER_RAIN_POP_PERCENT": 0,
            "WEATHER_TEMP_FORECAST_ENCODED": "",
            "WEATHER_RAIN_FORECAST_MM_X10_ENCODED": ""
        };

        cache_weather_data(clearDictionary);

        send_weather_to_watch(
            clearDictionary,
            "Weather data cleared successfully!",
            "Error clearing weather data!"
        );

        return;
    }

    if (!should_fetch_weather_from_api()) {
        var cachedDictionary = get_cached_weather_data();
        if (cachedDictionary) {
            console.log('Using cached weather data, skipping API call.');
            send_weather_to_watch(
                cachedDictionary,
                'Cached weather data sent to Pebble successfully!',
                'Error sending cached weather data to Pebble!'
            );
            return;
        }
    }

    navigator.geolocation.getCurrentPosition(
        locationSuccess,
        locationError,
        {timeout: 15000, maximumAge: 60000}
    );
}

module.exports.getWeather = getWeather;