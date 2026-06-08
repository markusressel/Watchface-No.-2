class Persistence {
    constructor() {
    }

    /**
     * @param key {string}
     * @param value {string}
     */
    putJson(key, value) {
        let stringified = JSON.stringify(value);
        localStorage.setItem(key, stringified);
    }

    /**
     * @param key {string}
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

    putString(key, value) {
        let stringValue = String(value);
        localStorage.setItem(key, stringValue);
    }

}

module.exports = new Persistence();
