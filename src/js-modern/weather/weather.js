import {logger} from '../logger.js';
import * as config from '../config/config';
import {WidgetId} from '../config/config';
import * as appMessaging from '../app_messaging';
import Persistence, {StorageKeys} from '../persistence';
import timelineSimulation from './mock/timeline.json';
import * as openmeteo from './openmeteo/openmeteo';

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
     * @param forecastStartTimestamp {number}
     */
    constructor(
        temperatureCurrent, temperatureMin, temperatureMax, conditions,
        rainMm, popPercent,
        temperatureForecastSeries, rainForecastSeries,
        forecastStartTimestamp
    ) {
        this.WEATHER_TEMPERATURE_CURRENT = temperatureCurrent;
        this.WEATHER_TEMPERATURE_MIN = temperatureMin;
        this.WEATHER_TEMPERATURE_MAX = temperatureMax;
        this.WEATHER_CONDITION = conditions;
        this.WEATHER_RAIN_NEXT_HOUR_MM_X10 = oneDecimalToInt(rainMm);
        this.WEATHER_RAIN_POP_PERCENT = popPercent;
        this.WEATHER_TEMP_FORECAST_ENCODED = appMessaging.encodeNumberArray(temperatureForecastSeries);
        this.WEATHER_RAIN_FORECAST_MM_X10_ENCODED = appMessaging.encodeNumberArray(rainForecastSeries);
        this.WEATHER_FORECAST_START_TS = forecastStartTimestamp || 0;
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
            'WEATHER_RAIN_FORECAST_MM_X10_ENCODED': this.WEATHER_RAIN_FORECAST_MM_X10_ENCODED,
            'WEATHER_FORECAST_START_TS': this.WEATHER_FORECAST_START_TS
        };
    }
}

export function requestSimulatedWeatherData() {
    const weatherData = processTimelinePayload(timelineSimulation, 'simulation/timeline.json');
    cacheWeatherData(weatherData);
    sendWeatherToWatch(
        weatherData,
        'Weather data sent to Pebble successfully!',
        'Error sending weather data to Pebble!'
    );
}

/**
 *
 * @param weatherData {WeatherData}
 */
export function cacheWeatherData(weatherData) {
    if (!(weatherData instanceof WeatherData)) {
        throw new Error('cacheWeatherData expects a WeatherData object');
    }
    try {
        Persistence.putJson(StorageKeys.WEATHER_LAST_DATA_KEY, weatherData.toDict());
        Persistence.putString(StorageKeys.WEATHER_LAST_FETCH_KEY, String(Date.now()));
    } catch (e) {
        logger.error('Could not cache weather data: ' + e);
    }
}

/**
 *
 * @param json {object}
 * @param sourceLabel {string}
 */
export function processTimelinePayload(json, sourceLabel) {
    if (!json || !json.data || json.data.length === 0) {
        logger.info('No timeline data available from ' + sourceLabel + '.');
        return;
    }

    logger.info('Weather source: ' + sourceLabel);
    logger.info('JSON response is: ' + JSON.stringify(json));

    const timeline = json.data || [];
    const current = pickClosestEntryToNow(timeline) || timeline[0] || {};
    const minMax = dayMinMaxFromTimeline(timeline, current, json.timezone_offset || 0);
    const minKelvin = minMax.minKelvin;
    const maxKelvin = minMax.maxKelvin;

    // Temperature in Kelvin requires adjustment
    const temperatureCurrent = kelvinToCelsius(current.temp);
    logger.info('Current Temperature is ' + temperatureCurrent);
    const temperatureMin = kelvinToCelsius(minKelvin);
    logger.info('Min Temperature is ' + temperatureMin);
    const temperatureMax = kelvinToCelsius(maxKelvin);
    logger.info('Max Temperature is ' + temperatureMax);

    // Conditions
    // Ignore for now
    const conditions = ''

    // Rain forecast based on selected 15-minute entry
    const rainMm = (current.rain !== null) ? current.rain : 0;
    const popPercent = typeof current.pop === 'number' ? Math.round(current.pop * 100) : 0;
    logger.info('Rain from selected entry (mm/h): ' + rainMm + ', pop (%): ' + popPercent);

    const claySettings = config.getClaySettings();
    const forecastPointCount = claySettings.SliderWeatherForecastPreviewHoursCount * 4;

    const timelineSized = timeline.slice(0, forecastPointCount);
    const forecastStartTimestamp = timelineSized.length > 0 ? (timelineSized[0].dt || 0) : 0;

    const temperatureForecastSeries = buildCondensedSeries(
        timelineSized,
        function (entry) {
            return kelvinToCelsius(entry.temp);
        },
        forecastPointCount
    );

    const rainForecastSeries = buildCondensedSeries(
        timelineSized,
        function (entry) {
            const rain = entry.rain;
            return appMessaging.encodeDecimalAsInt(rain, 1);
        },
        forecastPointCount
    );

    logger.info('Temperature forecast series: ' + temperatureForecastSeries);
    logger.info('Rain forecast series: ' + rainForecastSeries);

    // Assemble a dictionary using our keys
    return new WeatherData(
        temperatureCurrent,
        temperatureMin,
        temperatureMax,
        conditions,
        rainMm,
        popPercent,
        temperatureForecastSeries,
        rainForecastSeries,
        forecastStartTimestamp
    );
}

function processOpenMeteoPayload(json, sourceLabel) {
    if (!json || (!json.minutely_15 && !json.hourly)) {
        logger.info('No timeline data available from ' + sourceLabel + '.');
        return;
    }

    const timelineMap = {};

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

    return processTimelinePayload({data: timeline, timezone_offset: json.utc_offset_seconds}, sourceLabel);
}

/**
 * @param entries {object[]|null}
 * @return {*|null}
 */
export function pickClosestEntryToNow(entries) {
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
export function kelvinToCelsius(kelvin) {
    if (typeof kelvin !== 'number') {
        return 0;
    }

    return Math.round(kelvin - 273.15);
}

function getCachedWeatherData() {
    const dict = Persistence.getJson(StorageKeys.WEATHER_LAST_DATA_KEY);
    return WeatherData.fromDict(dict);
}

/**
 * Retrieves the timestamp of the last weather fetch from local storage.
 * @returns {number|null} - The timestamp of the last fetch, or null if not found.
 */
export function getLastFetchTimestamp() {
    const rawValue = Persistence.getInt(StorageKeys.WEATHER_LAST_FETCH_KEY)
    if (rawValue === null || isNaN(rawValue)) {
        return null;
    }
    return rawValue;
}

/**
 * Checks if the weather cache has expired based on the 15-minute update cycle.
 * The cycle boundaries are at 0, 15, 30, and 45 minutes past the hour, plus a 30-second buffer.
 * @returns {boolean} - True if the cache has expired or no fetch has occurred yet, false otherwise.
 */
export function isWeatherCacheExpired() {
    const lastFetchTimestamp = getLastFetchTimestamp();
    if (!lastFetchTimestamp) {
        return true;
    }

    const now = Date.now();
    const date = new Date(now);

    // Find the most recent boundary (0, 15, 30, 45)
    const minutes = date.getMinutes();
    const boundaryMinutes = Math.floor(minutes / 15) * 15;

    const boundaryDate = new Date(now);
    boundaryDate.setMinutes(boundaryMinutes);
    boundaryDate.setSeconds(30);
    boundaryDate.setMilliseconds(0);

    // If current time is before the 30s mark of the current 15min block, 
    // the previous boundary was 15 minutes ago.
    if (now < boundaryDate.getTime()) {
        boundaryDate.setTime(boundaryDate.getTime() - 15 * 60 * 1000);
    }

    return lastFetchTimestamp < boundaryDate.getTime();
}

function shouldFetchWeatherFromApi() {
    const cached = getCachedWeatherData();
    if (!cached) {
        return true;
    }

    return isWeatherCacheExpired();
}

/**
 * Sends a dictionary to the watch and logs the result.
 * @param weatherData {WeatherData}
 * @param successMessage {string}
 * @param errorMessage {string}
 */
export function sendWeatherToWatch(weatherData, successMessage, errorMessage) {
    if (!(weatherData instanceof WeatherData)) {
        throw new Error('sendWeatherToWatch expects a WeatherData object');
    }
    appMessaging.sendDictToWatch(weatherData.toDict(), successMessage, errorMessage);
}

function localDayIndex(utcTimestamp, timezoneOffsetSeconds) {
    return Math.floor((utcTimestamp + timezoneOffsetSeconds) / 86400);
}

export function dayMinMaxFromTimeline(entries, referenceEntry, timezoneOffsetSeconds) {
    if (!entries || entries.length === 0) {
        return {minKelvin: undefined, maxKelvin: undefined};
    }

    const reference = referenceEntry || entries[0];
    const referenceDay = localDayIndex(reference.dt || 0, timezoneOffsetSeconds || 0);
    let minKelvin = reference.temp;
    let maxKelvin = reference.temp;

    for (let i = 0; i < entries.length; i++) {
        const entry = entries[i];
        const t = entry && entry.temp;
        if (typeof t !== 'number') {
            continue;
        }

        if (localDayIndex(entry.dt || 0, timezoneOffsetSeconds || 0) !== referenceDay) {
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

export function buildCondensedSeries(entries, extractor, maxCount) {
    const samples = [];
    if (!entries || entries.length === 0) {
        return samples;
    }

    // Now we just map the entries directly, assuming they are already sized.
    for (let i = 0; i < entries.length; i++) {
        samples.push(extractor(entries[i]));
    }

    return samples;
}


/**
 * Fetches weather data from the selected API.
 * @param {number} latitude - The latitude coordinate for weather data.
 * @param {number} longitude - The longitude coordinate for weather data.
 * @returns {Promise<WeatherData|null>} A promise that resolves with the weather data.
 */
async function fetchWeatherData(latitude, longitude) {
    try {
        const claySettings = config.getClaySettings();
        const forecast_hours = claySettings.SliderWeatherForecastPreviewHoursCount
        const json = await openmeteo.fetchWeatherData(latitude, longitude, forecast_hours);
        return processOpenMeteoPayload(json, 'api/openmeteo');
    } catch (error) {
        logger.error('Error during API weather request or processing: ' + error);
    }
    return null;
}

export function clearWeatherData() {
    clearCache();

    // Send empty weather data to clear the display on the watchface
    const clearDictionary = new WeatherData(
        0,
        0,
        0,
        '',
        0,
        0,
        [],
        [],
        0
    );
    sendWeatherToWatch(
        clearDictionary,
        "Weather data cleared successfully!",
        "Error clearing weather data!"
    );
}

export function clearCache() {
    Persistence.remove(StorageKeys.WEATHER_LAST_DATA_KEY);
    Persistence.remove(StorageKeys.WEATHER_LAST_FETCH_KEY);
}

export function isAnyWeatherWidgetActive() {
    const claySettings = config.getClaySettings();
    logger.info('Checking if any weather widget is active. Settings: ' + JSON.stringify(claySettings));
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
 *
 * @param {boolean} force - If true, forces a weather data fetch regardless of cache age.
 */
export function getWeather(force = false) {
    // check if any row is set to a widget that requires weather data
    if (!isAnyWeatherWidgetActive()) {
        logger.info('No weather widgets active, skipping weather data fetch.');
        return;
    }

    if (config.isWeatherSimulationEnabled()) {
        logger.info('Simulation mode enabled. Using timeline.json weather data.');
        requestSimulatedWeatherData();
        return;
    }

    if (!force && !shouldFetchWeatherFromApi()) {
        const cachedDictionary = getCachedWeatherData();
        if (cachedDictionary) {
            logger.info('Using cached weather data, skipping API call.');
            sendWeatherToWatch(
                cachedDictionary,
                'Cached weather data sent to Pebble successfully!',
                'Error sending cached weather data to Pebble!'
            );
            return;
        }
    }

    navigator.geolocation.getCurrentPosition(
        function (pos) {
            fetchWeatherData(pos.coords.latitude, pos.coords.longitude)
                .then(weatherData => {
                    logger.info('Weather fetch process completed.');
                    if (weatherData) {
                        cacheWeatherData(weatherData);
                        sendWeatherToWatch(
                            weatherData,
                            'Weather data sent to Pebble successfully!',
                            'Error sending weather data to Pebble!'
                        );
                    }
                })
                .catch(e => {
                    logger.error('Error in weather fetch process: ' + e)
                });
        },
        function (err) {
            logger.error("Error requesting location!");
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
export function oneDecimalToInt(value) {
    return appMessaging.encodeDecimalAsInt(value, 1)
}
