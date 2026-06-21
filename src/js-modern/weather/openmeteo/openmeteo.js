import {logger} from '../../logger.js';

/**
 * Fetches weather data from the OpenMeteo API.
 * @param {number} latitude - The latitude coordinate for weather data.
 * @param {number} longitude - The longitude coordinate for weather data.
 * @param {number} forecast_hours - The number of hours to forecast.
 * @returns {Promise<JSON>} A promise that resolves with the weather data.
 */
export async function fetchWeatherData(latitude, longitude, forecast_hours) {
    const baseUrl = "https://api.open-meteo.com/v1/forecast";
    const queryParams = {
        latitude: latitude,
        longitude: longitude,
        minutely_15: "temperature_2m,apparent_temperature,precipitation,rain",
        models: "best_match",
        forecast_days: 1,
        forecast_minutely_15: 4 * forecast_hours,
        timezone: "auto"
    };
    const url = createUrl(baseUrl, queryParams);

    logger.info("Weather request URL is: " + url);

    // Send request to OpenMeteo
    const responseText = await xhrRequest(url, 'GET');
    return JSON.parse(responseText);
}


/**
 * Constructs a URL with query parameters.
 * @param {string} baseUrl - The base URL.
 * @param {object} queryParams - An object containing key-value pairs for query parameters.
 * @returns {string} The constructed URL.
 */
function createUrl(baseUrl, queryParams) {
    let url = baseUrl;
    const params = [];
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


const xhrRequest = async function (url, type) {
    return new Promise((resolve, reject) => {
        const xhr = new XMLHttpRequest();
        xhr.onload = function () {
            resolve(this.responseText);
        };
        xhr.onerror = function () {
            logger.error("Network error");
            reject(new Error("Network error"));
        };
        xhr.open(type, url);
        xhr.send();
    });
};
