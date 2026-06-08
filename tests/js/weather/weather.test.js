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

// Mock XMLHttpRequest
const mockXMLHttpRequest = jest.fn(() => ({
    onload: null,
    onerror: null,
    open: jest.fn(),
    send: jest.fn(function () {
        // Simulate a successful response for testing purposes
        const url = this.open.mock.calls[0][1];
        if (url.includes('openweathermap')) {
            this.responseText = JSON.stringify({
                "data": [
                    {
                        "dt": 1678886400, // March 15, 2023 12:00:00 PM UTC
                        "temp": 280.15, // 7 C
                        "weather": [{"main": "Clouds"}],
                        "rain": {"1h": 0.5},
                        "pop": 0.2
                    },
                    {
                        "dt": 1678890000, // March 15, 2023 1:00:00 PM UTC
                        "temp": 281.15, // 8 C
                        "weather": [{"main": "Rain"}],
                        "rain": {"1h": 1.2},
                        "pop": 0.7
                    }
                ],
                "timezone_offset": 3600 // +1 hour
            });
        } else {
            this.responseText = JSON.stringify({});
        }
        if (this.onload) {
            this.onload();
        }
    })
}));

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
global.XMLHttpRequest = mockXMLHttpRequest;
global.navigator = {geolocation: mockGeolocation};

// Mock the config module that weather.js requires
const mockConfig = {
    getWeatherApiKey: jest.fn(() => 'test_api_key'),
    isWeatherSimulationEnabled: jest.fn(() => false),
    setWeatherSimulationEnabled: jest.fn(),
};
jest.mock('../../../src/js-modern/config/config', () => ({
    __esModule: true,
    ...mockConfig
}));

// Mock the app_messaging module that weather.js requires
const mockAppMessaging = {
    send_dict_to_watch: jest.fn((dictionary, successMessage, errorMessage) => {
        mockPebble.sendAppMessage(dictionary, () => console.log(successMessage), () => console.log(errorMessage));
    }),
    encode_decimal_as_int: jest.fn((value, decimalPlaces) => {
        if (typeof value !== 'number') {
            return 0;
        }
        return Math.round(value * (decimalPlaces * 10));
    }),
    encode_number_array: jest.fn(values => values.join(','))
};
jest.mock('../../../src/js-modern/app_messaging', () => ({
    __esModule: true,
    ...mockAppMessaging,
}));


// Now require weather.js
const weather = require('../../../src/js-modern/weather/weather');

describe('weather.js', () => {
    const originalDateNow = Date.now;

    beforeEach(() => {
        // Reset all mocks before each test
        jest.clearAllMocks();
        mockLocalStorage.clear();
        mockPebble.lastSentMessage = null;
        mockConfig.getWeatherApiKey.mockReturnValue('test_api_key');
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
                    "rain": {"1h": 0.5},
                    "pop": 0.2
                },
                {
                    "dt": 1678890000, // March 15, 2023 1:00:00 PM UTC
                    "temp": 281.15, // 8 C
                    "weather": [{"main": "Rain"}],
                    "rain": {"1h": 1.2},
                    "pop": 0.7
                },
                {
                    "dt": 1678893600, // March 15, 2023 2:00:00 PM UTC
                    "temp": 282.15, // 9 C
                    "weather": [{"main": "Clear"}],
                    "rain": {"1h": 0.1},
                    "pop": 0.1
                },
                {
                    "dt": 1678972800, // March 16, 2023 12:00:00 PM UTC (next day)
                    "temp": 285.15, // 12 C
                    "weather": [{"main": "Clear"}],
                    "rain": {"1h": 0.0},
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

        expect(result).toEqual(expectedDictionary);
    });

    test('send_weather_to_watch', () => {
        const exampleData = {
            'WEATHER_TEMPERATURE_CURRENT': 7,
            'WEATHER_TEMPERATURE_MIN': 7,
            'WEATHER_TEMPERATURE_MAX': 9,
            'WEATHER_CONDITION': 'Clouds',
            'WEATHER_RAIN_NEXT_HOUR_MM_X10': 5,
            'WEATHER_RAIN_POP_PERCENT': 20,
            'WEATHER_TEMP_FORECAST_ENCODED': '7,8,9,12',
            'WEATHER_RAIN_FORECAST_MM_X10_ENCODED': '5,12,1,0'
        };

        weather.send_weather_to_watch(exampleData);

        expect(mockAppMessaging.send_dict_to_watch).toHaveBeenCalledTimes(1);
        expect(mockAppMessaging.send_dict_to_watch).toHaveBeenCalledWith(exampleData, undefined, undefined);
    });

    test('cache_weather_data stores data and timestamp in localStorage', () => {
        const exampleData = {
            'WEATHER_TEMPERATURE_CURRENT': 7,
            'WEATHER_TEMPERATURE_MIN': 7,
            'WEATHER_TEMPERATURE_MAX': 9,
            'WEATHER_CONDITION': 'Clouds',
            'WEATHER_RAIN_NEXT_HOUR_MM_X10': 5,
            'WEATHER_RAIN_POP_PERCENT': 20,
            'WEATHER_TEMP_FORECAST_ENCODED': '7,8,9,12',
            'WEATHER_RAIN_FORECAST_MM_X10_ENCODED': '5,12,1,0'
        };

        weather.cache_weather_data(exampleData);

        const cachedData = JSON.parse(mockLocalStorage.getItem('weather-last-data'));
        expect(cachedData).toEqual(exampleData);
        expect(mockLocalStorage.getItem('weather-last-fetch-ts')).not.toBeNull();
    });

    // Test getWeather with simulation enabled
    test('getWeather uses simulated data when simulation is enabled', () => {
        mockConfig.isWeatherSimulationEnabled.mockReturnValue(true);
        weather.getWeather();

        expect(mockAppMessaging.send_dict_to_watch).toHaveBeenCalledTimes(1);
        expect(mockPebble.lastSentMessage).not.toBeNull();
    });

    // Test getWeather with no API key
    test('getWeather sends clear data when no API key is configured', () => {
        mockConfig.getWeatherApiKey.mockReturnValue('');
        weather.getWeather();

        const expectedClearDictionary = {
            "WEATHER_TEMPERATURE_CURRENT": 0,
            "WEATHER_TEMPERATURE_MIN": 0,
            "WEATHER_TEMPERATURE_MAX": 0,
            "WEATHER_CONDITION": "",
            "WEATHER_RAIN_NEXT_HOUR_MM_X10": 0,
            "WEATHER_RAIN_POP_PERCENT": 0,
            "WEATHER_TEMP_FORECAST_ENCODED": "",
            "WEATHER_RAIN_FORECAST_MM_X10_ENCODED": ""
        };

        expect(mockAppMessaging.send_dict_to_watch).toHaveBeenCalledTimes(1);
        expect(mockPebble.lastSentMessage).toEqual(expectedClearDictionary);
        expect(JSON.parse(mockLocalStorage.getItem('weather-last-data'))).toEqual(expectedClearDictionary);
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

    // Test getWeather with API call (mocked XHR)
    test('getWeather makes API call when update is needed', async () => {
        // Simulate old timestamp to trigger API call
        mockLocalStorage.setItem('weather-last-fetch-ts', String(Date.now() - (31 * 60 * 1000)));

        // Mock Date.now() to control pick_closest_entry_to_now for the API response
        Date.now = jest.fn(() => 1678886400 * 1000); // Set current time to match the first entry in mock XHR data

        weather.getWeather();

        // wait for promises to resolve
        await new Promise(resolve => setImmediate(resolve));

        // The expected dictionary will be based on the mocked XMLHttpRequest response
        const expectedDictionary = {
            'WEATHER_TEMPERATURE_CURRENT': 7,
            'WEATHER_TEMPERATURE_MIN': 7,
            'WEATHER_TEMPERATURE_MAX': 8,
            'WEATHER_CONDITION': 'Clouds',
            'WEATHER_RAIN_NEXT_HOUR_MM_X10': 5,
            'WEATHER_RAIN_POP_PERCENT': 20,
            'WEATHER_TEMP_FORECAST_ENCODED': '7,8',
            'WEATHER_RAIN_FORECAST_MM_X10_ENCODED': '5,12'
        };

        expect(mockGeolocation.getCurrentPosition).toHaveBeenCalledTimes(1);
        expect(mockXMLHttpRequest).toHaveBeenCalledTimes(1);
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
});