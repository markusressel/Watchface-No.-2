import Settings from './generated/settings';

var CLAY_SETTINGS_KEY = 'clay-settings';

function getClaySettings() {
    return Settings(
        loadSettingsFromPersistence()
    )
}

/**
 * Loads the raw dictionary data from the localstorage.
 * Use getClaySettings() to get a wrapper object that exposes all available settings.
 *
 * @returns {{}|any|{}}
 */
function loadSettingsFromPersistence() {
    try {
        return JSON.parse(localStorage.getItem(CLAY_SETTINGS_KEY)) || {};
    } catch (e) {
        return {};
    }
}

function saveClaySettings(settings) {
    try {
        let settingsJson = settings.toJSON();
        localStorage.setItem(CLAY_SETTINGS_KEY, settingsJson);
    } catch (e) {
        console.log('Could not save clay settings: ' + e);
    }
}

function getWeatherApiKey() {
    return getClaySettings().WeatherApiKey || '';
}

function isWeatherSimulationEnabled() {
    return !!getClaySettings().WeatherUseSimulation;
}

function setWeatherSimulationEnabled(enabled) {
    var settings = getClaySettings();
    settings.WeatherUseSimulation = !!enabled;
    saveClaySettings(settings);
}

function toggleWeatherSimulation() {
    var nextState = !isWeatherSimulationEnabled();
    setWeatherSimulationEnabled(nextState);
    return nextState;
}

module.exports.getWeatherApiKey = getWeatherApiKey;
module.exports.isWeatherSimulationEnabled = isWeatherSimulationEnabled;
module.exports.setWeatherSimulationEnabled = setWeatherSimulationEnabled;
module.exports.toggleWeatherSimulation = toggleWeatherSimulation;
