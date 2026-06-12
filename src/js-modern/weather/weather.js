import * as config from '../config/config';
import {WidgetId} from '../config/config';
import * as appMessaging from '../app_messaging';
import Persistence, {StorageKeys} from '../persistence';
import timelineSimulation from './mock/timeline.json';
import * as openmeteo from './openmeteo/openmeteo';

const WEATHER_UPDATE_INTERVAL_MS = 30.0.minutes;

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
    const rainMm = (current.rain !== null) ? current.rain : 0;
    const popPercent = typeof current.pop === 'number' ? Math.round(current.pop * 100) : 0;
    console.log('Rain from selected entry (mm/h): ' + rainMm + ', pop (%): ' + popPercent);

    const claySettings = config.getClaySettings();
    const forecastPointCount = claySettings.SliderWeatherForecastPreviewHoursCount * 4;

    const temperatureForecastSeries = build_condensed_series(
        timeline,
        function (entry) {
            return kelvin_to_celsius(entry.temp);
        },
        forecastPointCount
    );

    const rainForecastSeries = build_condensed_series(
        timeline,
        function (entry) {
            const rain = entry.rain;
            return appMessaging.encode_decimal_as_int(rain, 1);
        },
        forecastPointCount
    );

    console.log('Temperature forecast series: ' + temperatureForecastSeries);
    console.log('Rain forecast series: ' + rainForecastSeries);

    // Assemble a dictionary using our keys
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

function process_openmeteo_payload(json, sourceLabel) {
    if (!json || (!json.minutely_15 && !json.hourly)) {
        console.log('No timeline data available from ' + sourceLabel + '.');
        return;
    }

    const timelineMap = {};

    if (json.hourly) {
        const data = json.hourly;
        for (let i = 0; i < data.time.length; i++) {
            const dt = Math.floor(new Date(data.time[i]).getTime() / 1000);
            timelineMap[dt] = {
                dt: dt,
                temp: data.temperature_2m[i] + 273.15, // Convert to Kelvin
                rain: data.rain[i],
                pop: (data.precipitation_probability ? data.precipitation_probability[i] / 100 : 0),
                weather: [{main: ''}] // Not available
            };
        }
    }

    if (json.minutely_15) {
        const data = json.minutely_15;
        for (let i = 0; i < data.time.length; i++) {
            const dt = Math.floor(new Date(data.time[i]).getTime() / 1000);
            // Prioritize minutely_15 for temperature and rain, keep pop from hourly if it overlaps
            timelineMap[dt] = {
                dt: dt,
                temp: data.temperature_2m[i] + 273.15, // Convert to Kelvin
                rain: data.rain[i],
                pop: timelineMap[dt] ? timelineMap[dt].pop : 0,
                weather: [{main: ''}] // Not available
            };
        }
    }

    const timeline = Object.keys(timelineMap)
        .map(function (key) {
            return timelineMap[key];
        })
        .sort(function (a, b) {
            return a.dt - b.dt;
        });

    return process_timeline_payload({data: timeline, timezone_offset: json.utc_offset_seconds}, sourceLabel);
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
    const rawValue = Persistence.getInt(StorageKeys.WEATHER_LAST_FETCH_KEY)
    if (rawValue === null || isNaN(rawValue)) {
        return null;
    }
    return rawValue;
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
 * Fetches weather data from the selected API.
 * @param {number} latitude - The latitude coordinate for weather data.
 * @param {number} longitude - The longitude coordinate for weather data.
 * @returns {Promise<WeatherData|null>} A promise that resolves with the weather data.
 */
async function fetch_weather_data(latitude, longitude) {
    try {
        const claySettings = config.getClaySettings();
        const forecast_hours = claySettings.SliderWeatherForecastPreviewHoursCount
        const json = await openmeteo.fetch_weather_data(latitude, longitude, forecast_hours);
        return process_openmeteo_payload(json, 'api/openmeteo');
    } catch (error) {
        console.log('Error during API weather request or processing: ' + error);
    }
    return null;
}

export function clearWeatherData() {
    clear_cache();

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
    send_weather_to_watch(
        clearDictionary,
        "Weather data cleared successfully!",
        "Error clearing weather data!"
    );
}

export function clear_cache() {
    Persistence.remove(StorageKeys.WEATHER_LAST_DATA_KEY);
    Persistence.remove(StorageKeys.WEATHER_LAST_FETCH_KEY);
}

export function isAnyWeatherWidgetActive() {
    const claySettings = config.getClaySettings();
    console.log('Checking if any weather widget is active. Settings: ' + JSON.stringify(claySettings));
    const weatherWidgetTypes = [WidgetId.Weather, WidgetId.WeatherForecast]
    let activeRowTypes = [
        claySettings.Row0Widget,
        claySettings.Row1Widget,
        claySettings.Row2Widget,
        claySettings.Row3Widget,
        claySettings.Row4Widget
    ];
    if (claySettings.LayoutRowCount > 5) activeRowTypes.push(claySettings.Row5Widget);
    if (claySettings.LayoutRowCount > 6) activeRowTypes.push(claySettings.Row6Widget);

    activeRowTypes = activeRowTypes.filter(option => option !== undefined && option !== null);

    return activeRowTypes.some(option => weatherWidgetTypes.includes(parseInt(option, 10)));
}

/**
 * Returns the currently most up-to-date weather data.
 * Data is either served from cache, if the data is still new enough.
 * Otherwise latest data is fetched, either from the selected API, or "timeline.json" file if simulation mode is enabled.
 * If no API key is configured for OpenWeatherMap, any existing weather data is cleared both in the cache and on the watchface.
 * Caches the last fetched weather data for [WEATHER_UPDATE_INTERVAL_MS] milliseconds to minimize unnecessary API calls.
 * Handles geolocation retrieval and API response processing, sending the relevant weather data to the Pebble watchface.
 */
export function getWeather() {
    // check if any row is set to a widget that requires weather data
    if (!isAnyWeatherWidgetActive()) {
        console.log('No weather widgets active, skipping weather data fetch.');
        return;
    }

    if (config.isWeatherSimulationEnabled()) {
        console.log('Simulation mode enabled. Using timeline.json weather data.');
        request_simulated_weather_data();
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
                .catch(e => {
                    console.log('Error in weather fetch process: ' + e)
                    clearWeatherData();
                });
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
