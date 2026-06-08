/**
 * Sends a dictionary to the watch and logs the result.
 * @param dictionary {object}
 * @param successMessage {string}
 * @param errorMessage {string}
 */
function send_dict_to_watch(dictionary, successMessage, errorMessage) {
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


/**
 * Encodes an array of numbers into a comma-separated string for transmission via Pebble's AppMessage system, which does not support arrays.
 * @param {number[]} values - The array of numbers to encode.
 * @returns {string} The encoded string.
 */
function encode_number_array(values) {
    return values.join(',');
}


/**
 * Encodes a decimal number into an integer by multiplying by a specified number of decimal places and rounding.
 * This allows us to send decimal values in an integer format, which is necessary because Pebble's AppMessage system does not support floating-point numbers.
 * @param {number|null} value - The number to convert.
 * @param {number} decimalPlaces - The number of decimal places to encode.
 * @returns {number} The converted integer value.
 */
function encode_decimal_as_int(value, decimalPlaces) {
    if (typeof value !== 'number') {
        return 0;
    }

    return Math.round(value * (decimalPlaces * 10));
}


module.exports.send_dict_to_watch = send_dict_to_watch;
module.exports.encode_number_array = encode_number_array;
module.exports.encode_decimal_as_int = encode_decimal_as_int;
