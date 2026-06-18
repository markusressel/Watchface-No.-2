import {logger} from './logger.js';

/**
 * Sends a dictionary to the watch and logs the result.
 * @param dictionary {object}
 * @param successMessage {string}
 * @param errorMessage {string}
 */
export function sendDictToWatch(dictionary, successMessage, errorMessage) {
    Pebble.sendAppMessage(
        dictionary,
        function (e) {
            logger.info(successMessage);
        },
        function (e) {
            logger.info(errorMessage);
        }
    );
}

/**
 * Encodes an array of numbers into a comma-separated string for transmission via Pebble's AppMessage system, which does not support arrays.
 * @param {number[]} values - The array of numbers to encode.
 * @returns {string} The encoded string.
 */
export function encodeNumberArray(values) {
    return values.join(',');
}

/**
 * Encodes a decimal number into an integer by multiplying by a specified number of decimal places and rounding.
 * This allows us to send decimal values in an integer format, which is necessary because Pebble's AppMessage system does not support floating-point numbers.
 * @param {number|null} value - The number to convert.
 * @param {number} decimalPlaces - The number of decimal places to encode.
 * @returns {number} The converted integer value.
 */
export function encodeDecimalAsInt(value, decimalPlaces) {
    if (typeof value !== 'number') {
        return 0;
    }

    return Math.round(value * (decimalPlaces * 10));
}
