let Settings = require('./generated/settings');
let Persistence = require('./persistence');

function getClaySettings() {
    return Settings(
        loadSettingsFromPersistence()
    )
}

/**
 * Loads the raw dictionary data from the persistence.
 * Use getClaySettings() to get a wrapper object that exposes all available settings.
 *
 * @returns {{}|any|{}}
 */
function loadSettingsFromPersistence() {
    try {
        return Persistence.getJson(Persistence.StorageKeys.CLAY_SETTINGS_KEY) || {};
    } catch (e) {
        return {};
    }
}

function saveClaySettings(settings) {
    try {
        Persistence.putJson(Persistence.StorageKeys.CLAY_SETTINGS_KEY, settings.toJSON());
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
    let settings = getClaySettings();
    settings.WeatherUseSimulation = !!enabled;
    saveClaySettings(settings);
}

module.exports.getClaySettings = getClaySettings;
module.exports.getWeatherApiKey = getWeatherApiKey;
module.exports.isWeatherSimulationEnabled = isWeatherSimulationEnabled;
module.exports.setWeatherSimulationEnabled = setWeatherSimulationEnabled;
