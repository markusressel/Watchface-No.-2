/**
 * @enum {string}
 */
export const StorageKeys = Object.freeze({
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
        const stringified = JSON.stringify(value);
        localStorage.setItem(key, stringified);
    }

    /**
     * @param key {StorageKeys}
     */
    getJson(key) {
        try {
            const rawValue = localStorage.getItem(key)
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
        const stringValue = String(value);
        localStorage.setItem(String(key), stringValue);
    }

    /**
     * @param key {StorageKeys}
     * @return {number|null}
     */
    getInt(key) {
        const lastFetchRaw = localStorage.getItem(key);
        if (!lastFetchRaw) {
            return null;
        }
        const parsed = parseInt(lastFetchRaw, 10);
        if (isNaN(parsed)) {
            return null;
        }
        return parsed;
    }

}

export default new Persistence();