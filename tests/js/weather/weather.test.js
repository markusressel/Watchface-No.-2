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
    send_dict_to_watch: jest.fn((dictionary, successMessage, errorMessage) => {
        mockPebble.sendAppMessage(dictionary, () => console.log(successMessage), () => console.log(errorMessage));
    }),
    encode_decimal_as_int: jest.fn((value, decimalPlaces) => {
        if (typeof value !== 'number') {
            return 0;
        }
        return Math.round(value * (Math.pow(10, decimalPlaces)));
    }),
    encode_number_array: jest.fn(values => Array.isArray(values) ? values.join(',') : '')
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
    fetch_weather_data: jest.fn().mockResolvedValue({
        "minutely_15": {
            "time": ["2023-03-15T12:00:00Z", "2023-03-15T13:00:00Z"],
            "temperature_2m": [7, 8],
            "rain": [0.5, 1.2]
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

    // Test kelvin_to_celsius
    test('kelvin_to_celsius converts Kelvin to Celsius correctly', () => {
        expect(weather.kelvin_to_celsius(273.15)).toBe(0);
        expect(weather.kelvin_to_celsius(280.15)).toBe(7);
        expect(weather.kelvin_to_celsius(263.15)).toBe(-10);
        expect(weather.kelvin_to_celsius(null)).toBe(0);
        expect(weather.kelvin_to_celsius(undefined)).toBe(0);
        expect(weather.kelvin_to_celsius('abc')).toBe(0);
    });

    // Test one_decimal_to_int
    test('one_decimal_to_int converts one decimal to integer correctly', () => {
        expect(weather.one_decimal_to_int(0.5)).toBe(5);
        expect(weather.one_decimal_to_int(1.2)).toBe(12);
        expect(weather.one_decimal_to_int(0)).toBe(0);
        expect(weather.one_decimal_to_int(null)).toBe(0);
        expect(weather.one_decimal_to_int(undefined)).toBe(0);
        expect(weather.one_decimal_to_int('abc')).toBe(0);
    });

    // Test process_timeline_payload
    test('process_timeline_payload creates the correct dictionary data', () => {
        const mockTimelineData = {
            "data": [
                {
                    "dt": 1678886400, // March 15, 2023 12:00:00 PM UTC
                    "temp": 280.15, // 7 C
                    "weather": [{"main": "Clouds"}],
                    "rain": 0.5, // Changed to number format matching OpenMeteo
                    "pop": 0.2
                },
                {
                    "dt": 1678890000, // March 15, 2023 1:00:00 PM UTC
                    "temp": 281.15, // 8 C
                    "weather": [{"main": "Rain"}],
                    "rain": 1.2,
                    "pop": 0.7
                },
                {
                    "dt": 1678893600, // March 15, 2023 2:00:00 PM UTC
                    "temp": 282.15, // 9 C
                    "weather": [{"main": "Clear"}],
                    "rain": 0.1,
                    "pop": 0.1
                },
                {
                    "dt": 1678972800, // March 16, 2023 12:00:00 PM UTC (next day)
                    "temp": 285.15, // 12 C
                    "weather": [{"main": "Clear"}],
                    "rain": 0.0,
                    "pop": 0.0
                }
            ],
            "timezone_offset": 3600 // +1 hour
        };

        Date.now = jest.fn(() => 1678886400 * 1000); // Set current time to 12:00:00 PM UTC

        const result = weather.process_timeline_payload(mockTimelineData, 'test_source');

        const expectedDictionary = {
            'WEATHER_TEMPERATURE_CURRENT': 7,
            'WEATHER_TEMPERATURE_MIN': 7, // Min for the day (15th March)
            'WEATHER_TEMPERATURE_MAX': 9, // Max for the day (15th March)
            'WEATHER_CONDITION': 'Clouds',
            'WEATHER_RAIN_NEXT_HOUR_MM_X10': 5, // 0.5 * 10
            'WEATHER_RAIN_POP_PERCENT': 20, // 0.2 * 100
            'WEATHER_TEMP_FORECAST_ENCODED': '7,8,9,12',
            'WEATHER_RAIN_FORECAST_MM_X10_ENCODED': '5,12,1,0'
        };

        expect(result.toDict()).toEqual(expectedDictionary);
    });

    test('send_weather_to_watch', () => {
        const exampleData = new WeatherData(
            7, 7, 9, 'Clouds', 0.5, 20, [7, 8, 9, 12], [5, 12, 1, 0]
        );

        weather.send_weather_to_watch(exampleData, "success", "error");

        expect(mockAppMessaging.send_dict_to_watch).toHaveBeenCalledTimes(1);
        expect(mockAppMessaging.send_dict_to_watch).toHaveBeenCalledWith(exampleData.toDict(), "success", "error");
    });

    test('send_weather_to_watch throws error if not passed a WeatherData instance', () => {
        expect(() => weather.send_weather_to_watch({some: 'data'}, 'success', 'error')).toThrow('send_weather_to_watch expects a WeatherData object');
    });

    test('cache_weather_data stores data and timestamp in localStorage', () => {
        const exampleData = new WeatherData(
            7, 7, 9, 'Clouds', 0.5, 20, [7, 8, 9, 12], [5, 12, 1, 0]
        );

        weather.cache_weather_data(exampleData);

        const cachedData = JSON.parse(mockLocalStorage.getItem('weather-last-data'));
        expect(cachedData).toEqual(exampleData.toDict());
        expect(mockLocalStorage.getItem('weather-last-fetch-ts')).not.toBeNull();
    });

    test('cache_weather_data throws error if not passed a WeatherData instance', () => {
        expect(() => weather.cache_weather_data({some: 'data'})).toThrow('cache_weather_data expects a WeatherData object');
    });

    // Test getWeather with simulation enabled
    test('getWeather uses simulated data when simulation is enabled', () => {
        mockConfig.isWeatherSimulationEnabled.mockReturnValue(true);
        weather.getWeather();

        expect(mockAppMessaging.send_dict_to_watch).toHaveBeenCalledTimes(1);
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

        expect(mockAppMessaging.send_dict_to_watch).toHaveBeenCalledTimes(1);
        expect(mockPebble.lastSentMessage).toEqual(cachedDictionary);
        expect(mockGeolocation.getCurrentPosition).not.toHaveBeenCalled(); // Should not call geolocation
    });

    // Test getWeather with API call (mocked OpenMeteo)
    test('getWeather makes API call when update is needed', async () => {
        // Simulate old timestamp to trigger API call
        mockLocalStorage.setItem('weather-last-fetch-ts', String(Date.now() - (31 * 60 * 1000)));

        // Mock Date.now() to control pick_closest_entry_to_now for the API response
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
            'WEATHER_RAIN_FORECAST_MM_X10_ENCODED': '5,12'
        };

        expect(mockGeolocation.getCurrentPosition).toHaveBeenCalledTimes(1);
        expect(mockOpenMeteo.fetch_weather_data).toHaveBeenCalledTimes(1);
        expect(mockAppMessaging.send_dict_to_watch).toHaveBeenCalledTimes(1);
        expect(mockPebble.lastSentMessage).toEqual(expectedDictionary);
    });

    // Test get_last_fetch_timestamp
    test('get_last_fetch_timestamp returns null if no timestamp is stored', () => {
        expect(weather.get_last_fetch_timestamp()).toBeNull();
    });

    test('get_last_fetch_timestamp returns null for invalid stored value', () => {
        mockLocalStorage.setItem('weather-last-fetch-ts', 'not-a-number');
        expect(weather.get_last_fetch_timestamp()).toBeNull();
    });

    test('get_last_fetch_timestamp returns the stored timestamp as a number', () => {
        const timestamp = Date.now();
        mockLocalStorage.setItem('weather-last-fetch-ts', String(timestamp));
        expect(weather.get_last_fetch_timestamp()).toBe(timestamp);
    });

    // Test time_since_last_fetch_exceeds
    test('time_since_last_fetch_exceeds returns true if no timestamp is stored', () => {
        expect(weather.time_since_last_fetch_exceeds(10000)).toBe(true);
    });

    test('time_since_last_fetch_exceeds returns true if time has exceeded the duration', () => {
        const now = Date.now();
        const fetchTime = now - 20000; // 20 seconds ago
        mockLocalStorage.setItem('weather-last-fetch-ts', String(fetchTime));
        Date.now = jest.fn(() => now);
        expect(weather.time_since_last_fetch_exceeds(10000)).toBe(true); // 10 seconds duration
    });

    test('time_since_last_fetch_exceeds returns false if time has not exceeded the duration', () => {
        const now = Date.now();
        const fetchTime = now - 5000; // 5 seconds ago
        mockLocalStorage.setItem('weather-last-fetch-ts', String(fetchTime));
        Date.now = jest.fn(() => now);
        expect(weather.time_since_last_fetch_exceeds(10000)).toBe(false); // 10 seconds duration
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
});