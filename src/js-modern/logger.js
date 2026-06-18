/**
 * @file logger.js
 * @description Custom logger that forwards to console methods in development and is a no-op in release builds.
 */

const isRelease = typeof __PEBBLE_RELEASE__ !== 'undefined' ? __PEBBLE_RELEASE__ : false;

export const logger = {
    /**
     * Log info messages.
     * @param {...*} args
     */
    info: (...args) => {
        if (!isRelease) {
            console.log(...args);
        }
    },

    /**
     * Log warning messages.
     * @param {...*} args
     */
    warn: (...args) => {
        if (!isRelease) {
            console.warn(...args);
        }
    },

    /**
     * Log error messages.
     * @param {...*} args
     */
    error: (...args) => {
        if (!isRelease) {
            console.error(...args);
        }
    },

    /**
     * Log debug messages.
     * @param {...*} args
     */
    debug: (...args) => {
        if (!isRelease) {
            console.debug(...args);
        }
    }
};
