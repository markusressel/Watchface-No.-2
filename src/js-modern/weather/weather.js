import * as config from '../config/config';
import * as appMessaging from '../app_messaging';
import Persistence, {StorageKeys} from '../persistence';
import timelineSimulation from './mock/timeline.json';
import * as owm from './openweathermap';

const WEATHER_UPDATE_INTERVAL_MS = 30.0.minutes;
const FORECAST_POINT_COUNT = 100;

export class WeatherData {

    /**
     * @param temperatureCurrent {number}
     * @param temperatureMin {number}
     * @param temperatureMax {number}
     * @param conditions {string}
     * @param rainMm {number}
     * @param popPercent {number}
     * @param temperatureForecastSeries {number[]}
     * @param rainForecastSeries {number[]}
     */
    constructor(
        temperatureCurrent, temperatureMin, temperatureMax, conditions,
        rainMm, popPercent,
        temperatureForecastSeries, rainForecastSeries
    ) {
        this.WEATHER_TEMPERATURE_CURRENT = temperatureCurrent;
        this.WEATHER_TEMPERATURE_MIN = temperatureMin;
        this.WEATHER_TEMPERATURE_MAX = temperatureMax;
        this.WEATHER_CONDITION = conditions;
        this.WEATHER_RAIN_NEXT_HOUR_MM_X10 = one_decimal_to_int(rainMm);
        this.WEATHER_RAIN_POP_PERCENT = popPercent;
        this.WEATHER_TEMP_FORECAST_ENCODED = appMessaging.encode_number_array(temperatureForecastSeries);
        this.WEATHER_RAIN_FORECAST_MM_X10_ENCODED = appMessaging.encode_number_array(rainForecastSeries);
    }

    static fromDict(dict) {
        if (!dict) return null;
        const instance = Object.create(WeatherData.prototype);
        Object.assign(instance, dict);
        return instance;
    }

    toDict() {
        return {
            'WEATHER_TEMPERATURE_CURRENT': this.WEATHER_TEMPERATURE_CURRENT,
            'WEATHER_TEMPERATURE_MIN': this.WEATHER_TEMPERATURE_MIN,
            'WEATHER_TEMPERATURE_MAX': this.WEATHER_TEMPERATURE_MAX,
            'WEATHER_CONDITION': this.WEATHER_CONDITION,
            'WEATHER_RAIN_NEXT_HOUR_MM_X10': this.WEATHER_RAIN_NEXT_HOUR_MM_X10,
            'WEATHER_RAIN_POP_PERCENT': this.WEATHER_RAIN_POP_PERCENT,
            'WEATHER_TEMP_FORECAST_ENCODED': this.WEATHER_TEMP_FORECAST_ENCODED,
            'WEATHER_RAIN_FORECAST_MM_X10_ENCODED': this.WEATHER_RAIN_FORECAST_MM_X10_ENCODED
        };
    }
}

export function request_simulated_weather_data() {
    const weatherData = process_timeline_payload(timelineSimulation, 'simulation/timeline.json');
    cache_weather_data(weatherData);
    send_weather_to_watch(
        weatherData,
        'Weather data sent to Pebble successfully!',
        'Error sending weather data to Pebble!'
    );
}

/**
 *
 * @param weatherData {WeatherData}
 */
export function cache_weather_data(weatherData) {
    if (!(weatherData instanceof WeatherData)) {
        throw new Error('cache_weather_data expects a WeatherData object');
    }
    try {
        Persistence.putJson(StorageKeys.WEATHER_LAST_DATA_KEY, weatherData.toDict());
        Persistence.putString(StorageKeys.WEATHER_LAST_FETCH_KEY, String(Date.now()));
    } catch (e) {
        console.log('Could not cache weather data: ' + e);
    }
}

/**
 *
 * @param json {object}
 * @param sourceLabel {string}
 */
export function process_timeline_payload(json, sourceLabel) {
    if (!json || !json.data || json.data.length === 0) {
        console.log('No timeline data available from ' + sourceLabel + '.');
        return;
    }

    console.log('Weather source: ' + sourceLabel);
    console.log('JSON response is: ' + JSON.stringify(json));

    const timeline = json.data || [];
    const current = pick_closest_entry_to_now(timeline) || timeline[0] || {};
    const minMax = day_min_max_from_timeline(timeline, current, json.timezone_offset || 0);
    const minKelvin = minMax.minKelvin;
    const maxKelvin = minMax.maxKelvin;

    // Temperature in Kelvin requires adjustment
    const temperatureCurrent = kelvin_to_celsius(current.temp);
    console.log('Current Temperature is ' + temperatureCurrent);
    const temperatureMin = kelvin_to_celsius(minKelvin);
    console.log('Min Temperature is ' + temperatureMin);
    const temperatureMax = kelvin_to_celsius(maxKelvin);
    console.log('Max Temperature is ' + temperatureMax);

    // Conditions
    let conditions = '';
    if (current.weather && current.weather.length > 0 && current.weather[0].main) {
        conditions = current.weather[0].main;
    }
    console.log('Conditions are ' + conditions);

    // Rain forecast based on selected 15-minute entry
    const rainMm = (current.rain && current.rain['1h']) ? current.rain['1h'] : 0;
    const popPercent = typeof current.pop === 'number' ? Math.round(current.pop * 100) : 0;
    console.log('Rain from selected entry (mm/h): ' + rainMm + ', pop (%): ' + popPercent);

    const temperatureForecastSeries = build_condensed_series(
        timeline,
        function (entry) {
            return kelvin_to_celsius(entry && entry.temp);
        },
        FORECAST_POINT_COUNT
    );

    const rainForecastSeries = build_condensed_series(
        timeline,
        function (entry) {
            const rain = (entry && entry.rain && entry.rain['1h']) ? entry.rain['1h'] : 0;
            return one_decimal_to_int(rain);
        },
        FORECAST_POINT_COUNT
    );

    // Assemble dictionary using our keys
    return new WeatherData(
        temperatureCurrent,
        temperatureMin,
        temperatureMax,
        conditions,
        rainMm,
        popPercent,
        temperatureForecastSeries,
        rainForecastSeries
    );
}

/**
 * @param entries {object[]|null}
 * @return {*|null}
 */
export function pick_closest_entry_to_now(entries) {
    if (!entries || entries.length === 0) {
        return null;
    }

    const nowUtc = Math.floor(Date.now() / 1000);
    let best = entries[0];
    let bestDistance = Math.abs((best.dt || 0) - nowUtc);

    for (let i = 1; i < entries.length; i++) {
        const candidate = entries[i];
        const distance = Math.abs((candidate.dt || 0) - nowUtc);
        if (distance < bestDistance) {
            best = candidate;
            bestDistance = distance;
        }
    }

    return best;
}

/**
 * Converts a temperature in Kelvin to Celsius.
 * @param kelvin {number}
 * @return {number}
 */
export function kelvin_to_celsius(kelvin) {
    if (typeof kelvin !== 'number') {
        return 0;
    }

    return Math.round(kelvin - 273.15);
}

function get_cached_weather_data() {
    const dict = Persistence.getJson(StorageKeys.WEATHER_LAST_DATA_KEY);
    return WeatherData.fromDict(dict);
}

/**
 * Retrieves the timestamp of the last weather fetch from local storage.
 * @returns {number|null} - The timestamp of the last fetch, or null if not found.
 */
export function get_last_fetch_timestamp() {
    return Persistence.getInt(StorageKeys.WEATHER_LAST_FETCH_KEY)
}

/**
 * Calculates whether the time since the last weather fetch in milliseconds exceeds the given duration.
 * @param durationMs {number} - The duration in milliseconds to compare against.
 * @returns {boolean} - True if the time since the last fetch exceeds the duration, or no fetch has occurred yet. False otherwise.
 */
export function time_since_last_fetch_exceeds(durationMs) {
    const lastFetchTimestamp = get_last_fetch_timestamp()
    if (!lastFetchTimestamp) {
        return true;
    }
    return Date.now() - lastFetchTimestamp >= durationMs;
}

function should_fetch_weather_from_api() {
    const cached = get_cached_weather_data();
    if (!cached) {
        return true;
    }

    return time_since_last_fetch_exceeds(WEATHER_UPDATE_INTERVAL_MS);
}

/**
 * Sends a dictionary to the watch and logs the result.
 * @param weatherData {WeatherData}
 * @param successMessage {string}
 * @param errorMessage {string}
 */
export function send_weather_to_watch(weatherData, successMessage, errorMessage) {
    if (!(weatherData instanceof WeatherData)) {
        throw new Error('send_weather_to_watch expects a WeatherData object');
    }
    appMessaging.send_dict_to_watch(weatherData.toDict(), successMessage, errorMessage);
}

function local_day_index(utcTimestamp, timezoneOffsetSeconds) {
    return Math.floor((utcTimestamp + timezoneOffsetSeconds) / 86400);
}

export function day_min_max_from_timeline(entries, referenceEntry, timezoneOffsetSeconds) {
    if (!entries || entries.length === 0) {
        return {minKelvin: undefined, maxKelvin: undefined};
    }

    const reference = referenceEntry || entries[0];
    const referenceDay = local_day_index(reference.dt || 0, timezoneOffsetSeconds || 0);
    let minKelvin = reference.temp;
    let maxKelvin = reference.temp;

    for (let i = 0; i < entries.length; i++) {
        const entry = entries[i];
        const t = entry && entry.temp;
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
            const fallbackTemp = entries[j] && entries[j].temp;
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

export function build_condensed_series(entries, extractor, maxCount) {
    const samples = [];
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
        const idx = Math.round((n * (entries.length - 1)) / (maxCount - 1));
        samples.push(extractor(entries[idx]));
    }

    return samples;
}


/**
 * Fetches weather data from the OpenWeatherMap API.
 * @param {number} latitude - The latitude coordinate for weather data.
 * @param {number} longitude - The longitude coordinate for weather data.
 * @returns {Promise<WeatherData|null>} A promise that resolves with the weather data.
 */
async function fetch_weather_data(latitude, longitude) {
    try {
        const json = await owm.fetch_weather_data(latitude, longitude);
        return process_timeline_payload(json, 'api/openweathermap');
    } catch (error) {
        console.log('Error during API weather request or processing: ' + error);
    }
    return null;
}

/**
 * Returns the currently most up-to-date weather data.
 * Data is either served from cache, if the data is still new enough.
 * Otherwise latest data is fetched, either from the OpenWeatherMap API, or "timeline.json" file if simulation mode is enabled.
 * If no API key is configured, any existing weather data is cleared both in the cache and on the watchface.
 * Caches the last fetched weather data for [WEATHER_UPDATE_INTERVAL_MS] milliseconds to minimize unnecessary API calls.
 * Handles geolocation retrieval and API response processing, sending the relevant weather data to the Pebble watchface.
 */
export function getWeather() {
    if (config.isWeatherSimulationEnabled()) {
        console.log('Simulation mode enabled. Using timeline.json weather data.');
        request_simulated_weather_data();
        return;
    }

    const apiKey = config.getWeatherApiKey();

    // If no API key is configured, clear any stale weather data
    if (!apiKey || apiKey.length === 0) {
        console.log("No OpenWeatherMap API key configured. Clearing weather data.");

        // Send empty weather data to clear the display on the watchface
        const clearDictionary = new WeatherData(
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
        const cachedDictionary = get_cached_weather_data();
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
            fetch_weather_data(pos.coords.latitude, pos.coords.longitude)
                .then(weatherData => {
                    console.log('Weather fetch process completed.');
                    if (weatherData) {
                        cache_weather_data(weatherData);
                        send_weather_to_watch(
                            weatherData,
                            'Weather data sent to Pebble successfully!',
                            'Error sending weather data to Pebble!'
                        );
                    }
                })
                .catch(e => console.log('Error in weather fetch process: ' + e));
        },
        function (err) {
            console.log("Error requesting location!");
        },
        {timeout: 15.0.seconds, maximumAge: 60.0.seconds}
    );
}

/**
 * Converts a number with one decimal place (e.g. 1.2) to an integer (e.g. 12) by multiplying by 10 and rounding.
 * This allows us to send decimal values in an integer format, which is necessary because Pebble's AppMessage system does not support floating-point numbers.
 * @param {number|null} value - The number to convert.
 * @returns {number} The converted integer value.
 */
export function one_decimal_to_int(value) {
    return appMessaging.encode_decimal_as_int(value, 1)
}

export const setSimulationModeEnabled = config.setWeatherSimulationEnabled;
