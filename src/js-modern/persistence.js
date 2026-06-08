/**
 * @enum {string}
 */
let StorageKeys = Object.freeze({
    CLAY_SETTINGS_KEY: 'clay-settings',

    WEATHER_LAST_DATA_KEY: 'weather-last-data',
    WEATHER_LAST_FETCH_KEY: 'weather-last-fetch-ts',

})

class Persistence {
    constructor() {
    }

    /**
     * @param key {StorageKeys}
     * @param value {object}
     */
    putJson(key, value) {
        let stringified = JSON.stringify(value);
        localStorage.setItem(key, stringified);
    }

    /**
     * @param key {StorageKeys}
     */
    getJson(key) {
        try {
            let rawValue = localStorage.getItem(key)
            return rawValue ? JSON.parse(rawValue) : null;
        } catch (e) {
            console.error('Error retrieving JSON from localStorage:', e);
            return null;
        }
    }

    /**
     * @param key {StorageKeys}
     * @param value {string}
     */
    putString(key, value) {
        let stringValue = String(value);
        localStorage.setItem(String(key), stringValue);
    }

    /**
     * @param key {StorageKeys}
     * @return {number|null}
     */
    getInt(key) {
        let lastFetchRaw = localStorage.getItem(key);
        if (!lastFetchRaw) {
            return null;
        }
        let parsed = parseInt(lastFetchRaw, 10);
        if (isNaN(parsed)) {
            return null;
        }
        return parsed;
    }

}

module.exports = new Persistence();
module.exports.StorageKeys = StorageKeys;