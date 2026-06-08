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


module.exports.send_dict_to_watch = send_dict_to_watch;