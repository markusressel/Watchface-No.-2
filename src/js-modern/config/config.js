import Settings from '../generated/settings';
import Persistence, {StorageKeys} from '../persistence';

export function getClaySettings() {
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
        return Persistence.getJson(StorageKeys.CLAY_SETTINGS_KEY) || {};
    } catch (e) {
        return {};
    }
}

function saveClaySettings(settings) {
    try {
        Persistence.putJson(StorageKeys.CLAY_SETTINGS_KEY, settings.toJSON());
    } catch (e) {
        console.log('Could not save clay settings: ' + e);
    }
}

export function getWeatherApiKey() {
    return getClaySettings().WeatherApiKey || '';
}

export function isWeatherSimulationEnabled() {
    return !!getClaySettings().WeatherUseSimulation;
}

export function setWeatherSimulationEnabled(enabled) {
    const settings = getClaySettings();
    settings.WeatherUseSimulation = !!enabled;
    saveClaySettings(settings);
}
