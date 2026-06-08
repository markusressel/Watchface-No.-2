let config = require('./config');
let appMessaging = require('./app_messaging');
let owm = require("./openweathermap");
let Persistence = require('./persistence');
let timelineSimulation = require('./timeline.json');


let WEATHER_UPDATE_INTERVAL_MINUTES = 30;
let WEATHER_UPDATE_INTERVAL_MS = WEATHER_UPDATE_INTERVAL_MINUTES * 60 * 1000;
let WEATHER_LAST_FETCH_KEY = 'weather-last-fetch-ts';
let WEATHER_LAST_DATA_KEY = 'weather-last-data';
let FORECAST_POINT_COUNT = 100;


function request_simulated_weather_data() {
    let weatherData = process_timeline_payload(timelineSimulation, 'simulation/timeline.json');
    cache_weather_data(weatherData);
    send_weather_to_watch(
        weatherData,
        'Weather data sent to Pebble successfully!',
        'Error sending weather data to Pebble!'
    );
}

function cache_weather_data(dictionary) {
    try {
        Persistence.putJson(WEATHER_LAST_DATA_KEY, dictionary);
        Persistence.putString(WEATHER_LAST_FETCH_KEY, Date.now());
    } catch (e) {
        console.log('Could not cache weather data: ' + e);
    }
}

function process_timeline_payload(json, sourceLabel) {
    if (!json || !json.data || json.data.length === 0) {
        console.log('No timeline data available from ' + sourceLabel + '.');
        return;
    }

    console.log('Weather source: ' + sourceLabel);
    console.log('JSON response is: ' + JSON.stringify(json));

    let timeline = json.data || [];
    let current = pick_closest_entry_to_now(timeline) || timeline[0] || {};
    let minMax = day_min_max_from_timeline(timeline, current, json.timezone_offset || 0);
    let minKelvin = minMax.minKelvin;
    let maxKelvin = minMax.maxKelvin;

    // Temperature in Kelvin requires adjustment
    let temperatureCurrent = kelvin_to_celsius(current.temp);
    console.log('Current Temperature is ' + temperatureCurrent);
    let temperatureMin = kelvin_to_celsius(minKelvin);
    console.log('Min Temperature is ' + temperatureMin);
    let temperatureMax = kelvin_to_celsius(maxKelvin);
    console.log('Max Temperature is ' + temperatureMax);

    // Conditions
    let conditions = '';
    if (current.weather && current.weather.length > 0 && current.weather[0].main) {
        conditions = current.weather[0].main;
    }
    console.log('Conditions are ' + conditions);

    // Rain forecast based on selected 15-minute entry
    let rainMm = (current.rain && current.rain['1h']) ? current.rain['1h'] : 0;
    let popPercent = typeof current.pop === 'number' ? Math.round(current.pop * 100) : 0;
    console.log('Rain from selected entry (mm/h): ' + rainMm + ', pop (%): ' + popPercent);

    let temperatureForecastSeries = build_condensed_series(
        timeline,
        function (entry) {
            return kelvin_to_celsius(entry && entry.temp);
        },
        FORECAST_POINT_COUNT
    );

    let rainForecastSeries = build_condensed_series(
        timeline,
        function (entry) {
            let rain = (entry && entry.rain && entry.rain['1h']) ? entry.rain['1h'] : 0;
            return one_decimal_to_int(rain);
        },
        FORECAST_POINT_COUNT
    );

    // Assemble dictionary using our keys
    return createWeatherData(
        temperatureCurrent,
        temperatureMin,
        temperatureMax,
        conditions,
        rainMm,
        popPercent,
        temperatureForecastSeries,
        rainForecastSeries
    )
}

function pick_closest_entry_to_now(entries) {
    if (!entries || entries.length === 0) {
        return null;
    }

    let nowUtc = Math.floor(Date.now() / 1000);
    let best = entries[0];
    let bestDistance = Math.abs((best.dt || 0) - nowUtc);

    for (let i = 1; i < entries.length; i++) {
        let candidate = entries[i];
        let distance = Math.abs((candidate.dt || 0) - nowUtc);
        if (distance < bestDistance) {
            best = candidate;
            bestDistance = distance;
        }
    }

    return best;
}


function kelvin_to_celsius(kelvin) {
    if (typeof kelvin !== 'number') {
        return 0;
    }

    return Math.round(kelvin - 273.15);
}


function get_cached_weather_data() {
    return Persistence.getJson(WEATHER_LAST_DATA_KEY);
}

/**
 * Retrieves the timestamp of the last weather fetch from local storage.
 * @returns {number|null} - The timestamp of the last fetch, or null if not found.
 */
function get_last_fetch_timestamp() {
    let lastFetchRaw = localStorage.getItem(WEATHER_LAST_FETCH_KEY);
    if (!lastFetchRaw) {
        return null;
    }
    let lastFetch = parseInt(lastFetchRaw, 10);
    if (isNaN(lastFetch)) {
        return null;
    }
    return lastFetch;
}

/**
 * Calculates whether the time since the last weather fetch in milliseconds exceeds the given duration.
 * @param durationMs {number} - The duration in milliseconds to compare against.
 * @returns {boolean} - True if the time since the last fetch exceeds the duration, or no fetch has occurred yet. False otherwise.
 */
function time_since_last_fetch_exceeds(durationMs) {
    let lastFetchTimestamp = get_last_fetch_timestamp()
    if (!lastFetchTimestamp) {
        return true;
    }
    return Date.now() - lastFetchTimestamp >= durationMs;
}

function should_fetch_weather_from_api() {
    let cached = get_cached_weather_data();
    if (!cached) {
        return true;
    }

    return time_since_last_fetch_exceeds(WEATHER_UPDATE_INTERVAL_MS);
}

/**
 * Sends a dictionary to the watch and logs the result.
 * @param dictionary {object}
 * @param successMessage {string}
 * @param errorMessage {string}
 */
function send_weather_to_watch(dictionary, successMessage, errorMessage) {
    appMessaging.send_dict_to_watch(dictionary, successMessage, errorMessage);
}

function local_day_index(utcTimestamp, timezoneOffsetSeconds) {
    return Math.floor((utcTimestamp + timezoneOffsetSeconds) / 86400);
}

function day_min_max_from_timeline(entries, referenceEntry, timezoneOffsetSeconds) {
    if (!entries || entries.length === 0) {
        return {minKelvin: undefined, maxKelvin: undefined};
    }

    let reference = referenceEntry || entries[0];
    let referenceDay = local_day_index(reference.dt || 0, timezoneOffsetSeconds || 0);
    let minKelvin;
    let maxKelvin;

    for (let i = 0; i < entries.length; i++) {
        let entry = entries[i];
        let t = entry && entry.temp;
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
        for (let j = 0; j < entries.length; j++) {
            let fallbackTemp = entries[j] && entries[j].temp;
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

function build_condensed_series(entries, extractor, maxCount) {
    let samples = [];
    if (!entries || entries.length === 0) {
        return samples;
    }

    if (entries.length <= maxCount) {
        for (let i = 0; i < entries.length; i++) {
            samples.push(extractor(entries[i]));
        }
        return samples;
    }

    for (let n = 0; n < maxCount; n++) {
        let idx = Math.round((n * (entries.length - 1)) / (maxCount - 1));
        samples.push(extractor(entries[idx]));
    }

    return samples;
}

let xhrRequest = function (url, type, callback) {
    let xhr = new XMLHttpRequest();
    xhr.onload = function () {
        callback(this.responseText);
    };
    xhr.onerror = function () {
        console.log("Network error");
    };
    xhr.open(type, url);
    xhr.send();
};

/**
 * Constructs a URL with query parameters.
 * @param {string} baseUrl - The base URL.
 * @param {object} queryParams - An object containing key-value pairs for query parameters.
 * @returns {string} The constructed URL.
 */
function createUrl(baseUrl, queryParams) {
    let url = baseUrl;
    let params = [];
    for (const key in queryParams) {
        if (queryParams.hasOwnProperty(key)) {
            params.push(`${key}=${queryParams[key]}`);
        }
    }
    if (params.length > 0) {
        url += '?' + params.join('&');
    }
    return url;
}

/**
 * Fetches weather data from the OpenWeatherMap API.
 * @param {number} latitude - The latitude coordinate for weather data.
 * @param {number} longitude - The longitude coordinate for weather data.
 */
function fetch_weather_data(latitude, longitude) {
    const baseUrl = "https://api.openweathermap.org/data/4.0/onecall/timeline/15min";
    const queryParams = {
        lat: latitude,
        lon: longitude,
        appid: config.getWeatherApiKey()
    };
    let url = createUrl(baseUrl, queryParams);

    console.log("Weather request URL is: " + url);

    // Send request to OpenWeatherMap
    xhrRequest(url, 'GET',
        function (responseText) {
            try {
                let json = JSON.parse(responseText);
                let weatherData = process_timeline_payload(json, 'api/openweathermap');
                cache_weather_data(weatherData);
                send_weather_to_watch(
                    weatherData,
                    'Weather data sent to Pebble successfully!',
                    'Error sending weather data to Pebble!'
                );
            } catch (error) {
                console.log('Error parsing API weather response: ' + error);
            }
        }
    );
}

/**
 * Returns the currently most up-to-date weather data.
 * Data is either served from cache, if the data is still new enough.
 * Otherwise latest data is fetched, either from the OpenWeatherMap API, or "timeline.json" file if simulation mode is enabled.
 * If no API key is configured, any existing weather data is cleared both in the cache and on the watchface.
 * Caches the last fetched weather data for [WEATHER_UPDATE_INTERVAL_MS] milliseconds to minimize unnecessary API calls.
 * Handles geolocation retrieval and API response processing, sending the relevant weather data to the Pebble watchface.
 */
function getWeather() {
    if (config.isWeatherSimulationEnabled()) {
        console.log('Simulation mode enabled. Using timeline.json weather data.');
        request_simulated_weather_data();
        return;
    }

    let apiKey = config.getWeatherApiKey();

    // If no API key is configured, clear any stale weather data
    if (!apiKey || apiKey.length === 0) {
        console.log("No OpenWeatherMap API key configured. Clearing weather data.");

        // Send empty weather data to clear the display on the watchface
        let clearDictionary = createWeatherData(
            0,
            0,
            0,
            '',
            0,
            0,
            [],
            []
        );

        cache_weather_data(clearDictionary);

        send_weather_to_watch(
            clearDictionary,
            "Weather data cleared successfully!",
            "Error clearing weather data!"
        );

        return;
    }

    if (!should_fetch_weather_from_api()) {
        let cachedDictionary = get_cached_weather_data();
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
        function (pos) {
            fetch_weather_data(pos.coords.latitude, pos.coords.longitude);
        },
        function (err) {
            console.log("Error requesting location!");
        },
        {timeout: 15000, maximumAge: 60000}
    );
}

function createWeatherData(
    temperatureCurrent, temperatureMin, temperatureMax, conditions,
    rainMm, popPercent,
    temperatureForecastSeries, rainForecastSeries
) {
    return {
        'WEATHER_TEMPERATURE_CURRENT': temperatureCurrent,
        'WEATHER_TEMPERATURE_MIN': temperatureMin,
        'WEATHER_TEMPERATURE_MAX': temperatureMax,
        'WEATHER_CONDITION': conditions,
        'WEATHER_RAIN_NEXT_HOUR_MM_X10': one_decimal_to_int(rainMm),
        'WEATHER_RAIN_POP_PERCENT': popPercent,
        'WEATHER_TEMP_FORECAST_ENCODED': encode_number_array(temperatureForecastSeries),
        'WEATHER_RAIN_FORECAST_MM_X10_ENCODED': encode_number_array(rainForecastSeries)
    }
}

/**
 * Converts a number with one decimal place (e.g. 1.2) to an integer (e.g. 12) by multiplying by 10 and rounding.
 * This allows us to send decimal values in an integer format, which is necessary because Pebble's AppMessage system does not support floating-point numbers.
 * @param {number} value - The number to convert.
 * @returns {number} The converted integer value.
 */
function one_decimal_to_int(value) {
    if (typeof value !== 'number') {
        return 0;
    }

    return Math.round(value * 10);
}

/**
 * Encodes an array of numbers into a comma-separated string for transmission via Pebble's AppMessage system, which does not support arrays.
 * @param {number[]} values - The array of numbers to encode.
 * @returns {string} The encoded string.
 */
function encode_number_array(values) {
    return values.join(',');
}


module.exports.getWeather = getWeather;
module.exports.setSimulationModeEnabled = config.setWeatherSimulationEnabled;

// Exporting for testing purposes
module.exports.kelvin_to_celsius = kelvin_to_celsius;
module.exports.one_decimal_to_int = one_decimal_to_int;
module.exports.process_timeline_payload = process_timeline_payload;
module.exports.cache_weather_data = cache_weather_data;
module.exports.send_weather_to_watch = send_weather_to_watch;
module.exports.pick_closest_entry_to_now = pick_closest_entry_to_now;
module.exports.day_min_max_from_timeline = day_min_max_from_timeline;
module.exports.build_condensed_series = build_condensed_series;
module.exports.encode_number_array = encode_number_array;
module.exports.get_last_fetch_timestamp = get_last_fetch_timestamp;
module.exports.time_since_last_fetch_exceeds = time_since_last_fetch_exceeds;
