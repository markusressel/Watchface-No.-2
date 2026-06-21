// Number prototype extensions required by the Pebble environment
Object.defineProperty(Number.prototype, 'minutes', {
    get: function () {
        return this * 60000;
    }
});
Object.defineProperty(Number.prototype, 'seconds', {
    get: function () {
        return this * 1000;
    }
});

// Mock Pebble and localStorage
const mockPebble = {
    sendAppMessage: jest.fn((dictionary, successCallback, errorCallback) => {
        mockPebble.lastSentMessage = dictionary;
        if (successCallback) {
            successCallback();
        }
    })
};

const mockLocalStorage = (() => {
    let store = {};
    return {
        getItem: jest.fn(key => store[key] || null),
        setItem: jest.fn((key, value) => {
            store[key] = value.toString();
        }),
        clear: jest.fn(() => {
            store = {};
        }),
        _getStore: () => store // For inspection in tests
    };
})();

// Mock navigator.geolocation
const mockGeolocation = {
    getCurrentPosition: jest.fn((success, error, options) => {
        success({
            coords: {
                latitude: 12.34,
                longitude: 56.78
            }
        });
    })
};

// Set up globals for weather.js
global.Pebble = mockPebble;
global.localStorage = mockLocalStorage;
global.navigator = {geolocation: mockGeolocation};

// Mock the config module
const mockConfig = {
    isWeatherSimulationEnabled: jest.fn(() => false),
    getClaySettings: jest.fn(() => ({
        Row0Widget: "0", // Weather
        Row1Widget: "1", // Date
        Row2Widget: "2", // Time
        Row3Widget: "3", // Stepcount
        Row4Widget: "4", // Battery
        LayoutRowCount: 5,
        SliderWeatherForecastPreviewHoursCount: 6,
        WeatherUseApparentTemp: true,
    })),
};
jest.mock('../../../src/js-modern/config/config', () => ({
    __esModule: true,
    ...mockConfig,
    WidgetId: {
        Weather: 0,
        Date: 1,
        Time: 2,
        Stepcount: 3,
        Battery: 4,
        Heartrate: 5,
        WeatherForecast: 6,
    }
}));

// Mock the app_messaging module
const mockAppMessaging = {
    sendDictToWatch: jest.fn((dictionary, successMessage, errorMessage) => {
        mockPebble.sendAppMessage(dictionary, () => console.log(successMessage), () => console.log(errorMessage));
    }),
    encodeDecimalAsInt: jest.fn((value, decimalPlaces) => {
        if (typeof value !== 'number') {
            return 0;
        }
        return Math.round(value * (Math.pow(10, decimalPlaces)));
    }),
    encodeNumberArray: jest.fn(values => Array.isArray(values) ? values.join(',') : '')
};
jest.mock('../../../src/js-modern/app_messaging', () => ({
    __esModule: true,
    ...mockAppMessaging,
}));

// Mock the Persistence module
jest.mock('../../../src/js-modern/persistence', () => ({
    __esModule: true,
    default: {
        getJson: jest.fn(key => {
            const val = mockLocalStorage.getItem(key);
            return val ? JSON.parse(val) : null;
        }),
        putJson: jest.fn((key, val) => mockLocalStorage.setItem(key, JSON.stringify(val))),
        getInt: jest.fn(key => {
            const val = mockLocalStorage.getItem(key);
            return val ? parseInt(val, 10) : null;
        }),
        putString: jest.fn((key, val) => mockLocalStorage.setItem(key, val))
    },
    StorageKeys: {
        WEATHER_LAST_DATA_KEY: 'weather-last-data',
        WEATHER_LAST_FETCH_KEY: 'weather-last-fetch-ts'
    }
}));

// Mock OpenMeteo module
const mockOpenMeteo = {
    fetchWeatherData: jest.fn().mockResolvedValue({
        "minutely_15": {
            "time": ["2023-03-15T12:00:00Z", "2023-03-15T13:00:00Z"],
            "temperature_2m": [7, 8],
            "precipitation": [0.5, 1.2]
        },
        "utc_offset_seconds": 3600
    })
};
jest.mock('../../../src/js-modern/weather/openmeteo/openmeteo', () => ({
    __esModule: true,
    ...mockOpenMeteo
}));

// Now require weather.js
const weather = require('../../../src/js-modern/weather/weather');
const {WeatherData} = require("../../../src/js-modern/weather/weather");

describe('weather.js', () => {
    const originalDateNow = Date.now;

    beforeEach(() => {
        // Reset all mocks before each test
        jest.clearAllMocks();
        mockLocalStorage.clear();
        mockPebble.lastSentMessage = null;
        mockConfig.isWeatherSimulationEnabled.mockReturnValue(false);
        Date.now = originalDateNow; // Restore Date.now
    });

    // Test processOpenMeteoPayload
    test('processOpenMeteoPayload creates the correct dictionary data', () => {
        const mockOpenMeteoData = {
            "minutely_15": {
                "time": [
                    "2023-03-15T12:00:00Z", // March 15, 2023 12:00:00 PM UTC
                    "2023-03-15T13:00:00Z", // 1:00:00 PM UTC
                    "2023-03-15T14:00:00Z", // 2:00:00 PM UTC
                    "2023-03-16T12:00:00Z"  // next day
                ],
                "temperature_2m": [7, 8, 9, 12],
                "precipitation": [0.5, 1.2, 0.1, 0.0]
            },
            "utc_offset_seconds": 3600
        };

        Date.now = jest.fn(() => 1678881600 * 1000); // Set current time (12:00:00 UTC)

        const result = weather.processOpenMeteoPayload(mockOpenMeteoData, 'test_source');

        const expectedDictionary = {
            'WEATHER_TEMPERATURE_CURRENT': 7,
            'WEATHER_TEMPERATURE_MIN': 7,
            'WEATHER_TEMPERATURE_MAX': 9,
            'WEATHER_CONDITION': '',
            'WEATHER_RAIN_NEXT_HOUR_MM_X10': 5, // 0.5 * 10
            'WEATHER_RAIN_POP_PERCENT': 0, // pop is 0 now
            'WEATHER_TEMP_FORECAST_ENCODED': '7,8,9,12',
            'WEATHER_RAIN_FORECAST_MM_X10_ENCODED': '5,12,1,0',
            'WEATHER_FORECAST_START_TS': 1678881600 // 12:00:00 PM UTC
        };

        expect(result.toDict()).toEqual(expectedDictionary);
    });

    test('processOpenMeteoPayload uses apparent temperature when WeatherUseApparentTemp is true', () => {
        const mockOpenMeteoData = {
            "minutely_15": {
                "time": [
                    "2023-03-15T12:00:00Z"
                ],
                "temperature_2m": [7],
                "apparent_temperature": [10],
                "precipitation": [0.5]
            },
            "utc_offset_seconds": 3600
        };

        Date.now = jest.fn(() => 1678881600 * 1000); // 12:00:00 UTC

        // When WeatherUseApparentTemp is false
        mockConfig.getClaySettings.mockReturnValueOnce({
            SliderWeatherForecastPreviewHoursCount: 6,
            WeatherUseApparentTemp: false
        });
        const resultNormal = weather.processOpenMeteoPayload(mockOpenMeteoData, 'test_source');
        expect(resultNormal.WEATHER_TEMPERATURE_CURRENT).toBe(7);

        // When WeatherUseApparentTemp is true
        mockConfig.getClaySettings.mockReturnValueOnce({
            SliderWeatherForecastPreviewHoursCount: 6,
            WeatherUseApparentTemp: true
        });
        const resultApparent = weather.processOpenMeteoPayload(mockOpenMeteoData, 'test_source');
        expect(resultApparent.WEATHER_TEMPERATURE_CURRENT).toBe(10);
    });

    test('processOpenMeteoPayload falls back to temperature_2m if apparent_temperature is missing', () => {
        const mockOpenMeteoData = {
            "minutely_15": {
                "time": [
                    "2023-03-15T12:00:00Z"
                ],
                "temperature_2m": [7],
                "precipitation": [0.5]
            },
            "utc_offset_seconds": 3600
        };

        Date.now = jest.fn(() => 1678881600 * 1000); // 12:00:00 UTC

        mockConfig.getClaySettings.mockReturnValueOnce({
            SliderWeatherForecastPreviewHoursCount: 6,
            WeatherUseApparentTemp: true
        });
        const result = weather.processOpenMeteoPayload(mockOpenMeteoData, 'test_source');
        expect(result.WEATHER_TEMPERATURE_CURRENT).toBe(7);
    });


    test('sendWeatherToWatch', () => {
        const exampleData = new WeatherData(
            7, 7, 9, 'Clouds', 0.5, 20, [7, 8, 9, 12], [5, 12, 1, 0], 1678886400
        );

        weather.sendWeatherToWatch(exampleData, "success", "error");

        expect(mockAppMessaging.sendDictToWatch).toHaveBeenCalledTimes(1);
        expect(mockAppMessaging.sendDictToWatch).toHaveBeenCalledWith(exampleData.toDict(), "success", "error");
    });

    test('sendWeatherToWatch throws error if not passed a WeatherData instance', () => {
        expect(() => weather.sendWeatherToWatch({some: 'data'}, 'success', 'error')).toThrow('sendWeatherToWatch expects a WeatherData object');
    });

    test('cacheWeatherData stores data and timestamp in localStorage', () => {
        const exampleData = new WeatherData(
            7, 7, 9, 'Clouds', 0.5, 20, [7, 8, 9, 12], [5, 12, 1, 0], 1678886400
        );

        weather.cacheWeatherData(exampleData);

        const cachedData = JSON.parse(mockLocalStorage.getItem('weather-last-data'));
        expect(cachedData).toEqual(exampleData.toDict());
        expect(mockLocalStorage.getItem('weather-last-fetch-ts')).not.toBeNull();
    });

    test('cacheWeatherData throws error if not passed a WeatherData instance', () => {
        expect(() => weather.cacheWeatherData({some: 'data'})).toThrow('cacheWeatherData expects a WeatherData object');
    });

    // Test getWeather with simulation enabled
    test('getWeather uses simulated data when simulation is enabled', () => {
        mockConfig.isWeatherSimulationEnabled.mockReturnValue(true);
        weather.getWeather();

        expect(mockAppMessaging.sendDictToWatch).toHaveBeenCalledTimes(1);
        expect(mockPebble.lastSentMessage).not.toBeNull();
    });

    // Test getWeather with cached data and not needing update
    test('getWeather uses cached data if not needing update', () => {
        const cachedDictionary = {
            'WEATHER_TEMPERATURE_CURRENT': 10,
            'WEATHER_TEMPERATURE_MIN': 5,
            'WEATHER_TEMPERATURE_MAX': 15,
            'WEATHER_CONDITION': 'Sunny',
            'WEATHER_RAIN_NEXT_HOUR_MM_X10': 0,
            'WEATHER_RAIN_POP_PERCENT': 0,
            'WEATHER_TEMP_FORECAST_ENCODED': '10,11,12',
            'WEATHER_RAIN_FORECAST_MM_X10_ENCODED': '0,0,0'
        };
        mockLocalStorage.setItem('weather-last-data', JSON.stringify(cachedDictionary));
        mockLocalStorage.setItem('weather-last-fetch-ts', String(Date.now())); // Set current time, so it won't update

        weather.getWeather();

        expect(mockAppMessaging.sendDictToWatch).toHaveBeenCalledTimes(1);
        expect(mockPebble.lastSentMessage).toEqual(cachedDictionary);
        expect(mockGeolocation.getCurrentPosition).not.toHaveBeenCalled(); // Should not call geolocation
    });

    // Test getWeather with API call (mocked OpenMeteo)
    test('getWeather makes API call when update is needed', async () => {
        // Simulate old timestamp to trigger API call
        mockLocalStorage.setItem('weather-last-fetch-ts', String(Date.now() - (31 * 60 * 1000)));

        // Mock Date.now() to control pickClosestEntryToNow for the API response
        Date.now = jest.fn(() => 1678886400 * 1000); // March 15, 2023 12:00:00 PM UTC

        weather.getWeather();

        // wait for promises to resolve
        await new Promise(resolve => setImmediate(resolve));

        // The expected dictionary will be based on the mocked OpenMeteo response
        const expectedDictionary = {
            'WEATHER_TEMPERATURE_CURRENT': 8,
            'WEATHER_TEMPERATURE_MIN': 7,
            'WEATHER_TEMPERATURE_MAX': 8,
            'WEATHER_CONDITION': '', // OpenMeteo mapping explicitly sets this to empty
            'WEATHER_RAIN_NEXT_HOUR_MM_X10': 12,
            'WEATHER_RAIN_POP_PERCENT': 0, // OpenMeteo mapping explicitly sets this to 0
            'WEATHER_TEMP_FORECAST_ENCODED': '7,8',
            'WEATHER_RAIN_FORECAST_MM_X10_ENCODED': '5,12',
            'WEATHER_FORECAST_START_TS': 1678881600
        };

        expect(mockGeolocation.getCurrentPosition).toHaveBeenCalledTimes(1);
        expect(mockOpenMeteo.fetchWeatherData).toHaveBeenCalledTimes(1);
        expect(mockAppMessaging.sendDictToWatch).toHaveBeenCalledTimes(1);
        expect(mockPebble.lastSentMessage).toEqual(expectedDictionary);
    });

    // Test getLastFetchTimestamp
    test('getLastFetchTimestamp returns null if no timestamp is stored', () => {
        expect(weather.getLastFetchTimestamp()).toBeNull();
    });

    test('getLastFetchTimestamp returns null for invalid stored value', () => {
        mockLocalStorage.setItem('weather-last-fetch-ts', 'not-a-number');
        expect(weather.getLastFetchTimestamp()).toBeNull();
    });

    test('getLastFetchTimestamp returns the stored timestamp as a number', () => {
        const timestamp = Date.now();
        mockLocalStorage.setItem('weather-last-fetch-ts', String(timestamp));
        expect(weather.getLastFetchTimestamp()).toBe(timestamp);
    });

    // Test isWeatherCacheExpired
    test('isWeatherCacheExpired returns true if no timestamp is stored', () => {
        expect(weather.isWeatherCacheExpired()).toBe(true);
    });

    test('isWeatherCacheExpired returns false if fetched recently within same block', () => {
        // Set current time to 10:05:00
        const now = new Date(2023, 2, 15, 10, 5, 0).getTime();
        Date.now = jest.fn(() => now);

        // Fetch happened at 10:01:00 (after 10:00:30 boundary)
        const fetchTime = new Date(2023, 2, 15, 10, 1, 0).getTime();
        mockLocalStorage.setItem('weather-last-fetch-ts', String(fetchTime));

        expect(weather.isWeatherCacheExpired()).toBe(false);
    });

    test('isWeatherCacheExpired returns true if boundary crossed', () => {
        // Set current time to 10:16:00
        const now = new Date(2023, 2, 15, 10, 16, 0).getTime();
        Date.now = jest.fn(() => now);

        // Fetch happened at 10:14:00 (before 10:15:30 boundary)
        const fetchTime = new Date(2023, 2, 15, 10, 14, 0).getTime();
        mockLocalStorage.setItem('weather-last-fetch-ts', String(fetchTime));

        expect(weather.isWeatherCacheExpired()).toBe(true);
    });

    test('isWeatherCacheExpired handles 30s buffer', () => {
        // Set current time to 10:15:29 (just before boundary)
        const now = new Date(2023, 2, 15, 10, 15, 29).getTime();
        Date.now = jest.fn(() => now);

        // Fetch happened at 10:01:00
        const fetchTime = new Date(2023, 2, 15, 10, 1, 0).getTime();
        mockLocalStorage.setItem('weather-last-fetch-ts', String(fetchTime));

        // Boundary is 10:00:30, so it's not expired
        expect(weather.isWeatherCacheExpired()).toBe(false);

        // Advance to 10:15:31 (just after boundary)
        Date.now = jest.fn(() => new Date(2023, 2, 15, 10, 15, 31).getTime());
        // Now it's expired because last fetch (10:01) is before 10:15:30
        expect(weather.isWeatherCacheExpired()).toBe(true);
    });

    // Test isAnyWeatherWidgetActive
    test('isAnyWeatherWidgetActive returns true if a weather widget is active', () => {
        mockConfig.getClaySettings.mockReturnValue({
            Row0Widget: "0", // Weather
            LayoutRowCount: 1,
        });
        expect(weather.isAnyWeatherWidgetActive()).toBe(true);
    });

    test('isAnyWeatherWidgetActive returns true if a weather forecast widget is active', () => {
        mockConfig.getClaySettings.mockReturnValue({
            Row0Widget: "6", // WeatherForecast
            LayoutRowCount: 1,
        });
        expect(weather.isAnyWeatherWidgetActive()).toBe(true);
    });

    test('isAnyWeatherWidgetActive returns false if no weather-related widget is active', () => {
        mockConfig.getClaySettings.mockReturnValue({
            Row0Widget: "1", // Date
            Row1Widget: "2", // Time
            LayoutRowCount: 2,
        });
        expect(weather.isAnyWeatherWidgetActive()).toBe(false);
    });

    test('isAnyWeatherWidgetActive handles all rows', () => {
        mockConfig.getClaySettings.mockReturnValue({
            Row0Widget: "1",
            Row1Widget: "2",
            Row2Widget: "3",
            Row3Widget: "4",
            Row4Widget: "5",
            Row5Widget: "6", // WeatherForecast
            Row6Widget: "1",
            LayoutRowCount: 7,
        });
        expect(weather.isAnyWeatherWidgetActive()).toBe(true);
    });

    test('isAnyWeatherWidgetActive handles undefined rows', () => {
        mockConfig.getClaySettings.mockReturnValue({
            Row0Widget: "1",
            Row1Widget: "2",
            LayoutRowCount: 2,
        });
        expect(weather.isAnyWeatherWidgetActive()).toBe(false);
    });

    test('getWeather returns early if no weather widget is active', () => {
        mockConfig.getClaySettings.mockReturnValue({
            Row0Widget: "1", // Date (no weather)
            LayoutRowCount: 1,
        });
        const spyLog = jest.spyOn(require('../../../src/js-modern/logger').logger, 'info');
        weather.getWeather();
        expect(spyLog).toHaveBeenCalledWith('No weather widgets active, skipping weather data fetch.');
        spyLog.mockRestore();
    });

    test('getWeather error callback for getCurrentPosition', () => {
        mockConfig.getClaySettings.mockReturnValue({
            Row0Widget: "0", // Weather active
            LayoutRowCount: 1,
        });
        mockGeolocation.getCurrentPosition.mockImplementationOnce((success, error, options) => {
            error({ message: 'Location error' });
        });
        const spyWarnLog = jest.spyOn(require('../../../src/js-modern/logger').logger, 'warn');
        weather.getWeather();
        expect(spyWarnLog).toHaveBeenCalledWith('Error requesting location: Location error. Using fallback coordinates (Berlin).');
        spyWarnLog.mockRestore();
    });

    test('getWeather catch block for fetchWeatherData rejection', async () => {
        mockConfig.getClaySettings.mockReturnValue({
            Row0Widget: "0", // Weather active
            LayoutRowCount: 1,
        });
        mockGeolocation.getCurrentPosition.mockImplementationOnce((success, error, options) => {
            success({ coords: { latitude: 12.34, longitude: 56.78 } });
        });
        mockOpenMeteo.fetchWeatherData.mockRejectedValueOnce(new Error('Network failure'));
        const spyErrorLog = jest.spyOn(require('../../../src/js-modern/logger').logger, 'error');
        
        weather.getWeather();
        
        // Wait for promise to resolve
        await new Promise(process.nextTick);
        
        expect(spyErrorLog).toHaveBeenCalledWith('Error during API weather request or processing: Error: Network failure');
        spyErrorLog.mockRestore();
    });

    test('getWeather when navigator.geolocation is not defined', () => {
        mockConfig.getClaySettings.mockReturnValue({
            Row0Widget: "0", // Weather active
            LayoutRowCount: 1,
        });
        const originalGeolocation = global.navigator.geolocation;
        delete global.navigator.geolocation;
        
        const spyWarnLog = jest.spyOn(require('../../../src/js-modern/logger').logger, 'warn');
        weather.getWeather();
        expect(spyWarnLog).toHaveBeenCalledWith('navigator.geolocation is not available. Using fallback coordinates (Berlin).');
        spyWarnLog.mockRestore();
        
        // Restore geolocation
        global.navigator.geolocation = originalGeolocation;
    });
});
