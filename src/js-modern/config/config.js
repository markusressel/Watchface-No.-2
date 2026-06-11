import Settings from '../generated/settings';
import Persistence, {StorageKeys} from '../persistence';

/**
 *
 *                 "options": [
 *                     {"label": "Weather", "value": 0},
 *                     {"label": "Date", "value": 1},
 *                     {"label": "Time", "value": 2},
 *                     {"label": "Stepcount", "value": 3},
 *                     {"label": "Battery", "value": 4},
 *                     {"label": "Heartrate", "value": 5},
 *                     {"label": "Weather Forecast", "value": 6},
 *                 ]
 *
 * @enum {integer}
 */
export const WidgetId = Object.freeze({
    Weather: 0,
    Date: 1,
    Time: 2,
    Stepcount: 3,
    Battery: 4,
    Heartrate: 5,
    WeatherForecast: 6,
})

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

export function isWeatherSimulationEnabled() {
    return !!getClaySettings().WeatherUseSimulation;
}
