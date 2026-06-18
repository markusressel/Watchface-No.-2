import {logger} from './logger.js';
import Clay from '@rebble/clay';
import clayConfig from './config/configPage';
import customClay from './config/custom-clay';
import * as config from './config/config';
import * as weather from './weather/weather';

const clay = new Clay(clayConfig, customClay);

/**
 * Indicated whether the Watch app is ready to receive data, aka. we have received an "AppReady" message from it.
 */
let isPebbleReady = false;

// When the config page is closed, Clay saves settings to 'clay-settings' in
// localStorage (and sends watch-bound keys to the watch). We add our own
// webviewclosed listener here to react to JS-only settings like WeatherApiKey.
Pebble.addEventListener('webviewclosed', function (e) {
    if (!e || !e.response) {
        return;
    }

    // Refresh weather
    if (isPebbleReady) {
        // Clay has already saved the new settings to LocalStorage, 
        // but we want to intercept that to apply our own logic,
        // like clearing weather data
        const settingsJson = JSON.parse(e.response);
        const settings = new config.getClaySettings().constructor(settingsJson);
        config.saveClaySettings(settings);

        if (settingsJson.ClearWeatherCache) {
            logger.info("ClearWeatherCache flag detected, clearing weather data.");
            weather.clearWeatherData();
        }

        weather.getWeather(true);
    }
});

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
    function (e) {
        logger.info("PebbleKit JS ready!");

        // ==========================================
        // TEMPORARY RESET CODE
        // ==========================================
        // logger.info("Wiping Clay settings...");
        // localStorage.clear();

        // We no longer send weather here immediately. We wait for the AppReady message,
        // to avoid overwhelming a not-yet-initialized watchface.
    }
);

function sendAllSettings() {
    const settings = config.getClaySettings()
    const settingsJson = settings.toJSON();
    if (settingsJson) {
        Pebble.sendAppMessage(settingsJson);
    }
}


// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
    function (e) {
        // Get the dictionary from the message
        const dict = e.payload;

        logger.info('AppMessage received: ' + JSON.stringify(dict));

        if ("AppReady" in dict) {
            onAppReady();
        }
        if ("RequestWeatherData" in dict) {
            onRequestWeatherData()
        }
        if ("RequestSettings" in dict) {
            onRequestSettings()
        }
    }
);

function onAppReady() {
    logger.info("Watchface is ready!");
    isPebbleReady = true;
}

function onRequestWeatherData() {
    if (isPebbleReady) {
        weather.getWeather(true);
    }
}

function onRequestSettings() {
    if (isPebbleReady) {
        sendAllSettings();
    }
}
