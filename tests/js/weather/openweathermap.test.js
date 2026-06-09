const mockConfig = {
    getWeatherApiKey: jest.fn(() => 'test_api_key'),
};
jest.mock('../../../src/js-modern/config/config', () => ({
    __esModule: true,
    ...mockConfig
}));

// Mock XMLHttpRequest
const mockOpen = jest.fn();
const mockSend = jest.fn(function () {
    this.responseText = JSON.stringify({
        "data": [
            {
                "dt": 1678886400, // March 15, 2023 12:00:00 PM UTC
                "temp": 280.15, // 7 C
                "weather": [{"main": "Clouds"}],
                "rain": {"1h": 0.5},
                "pop": 0.2
            }
        ],
        "timezone_offset": 3600 // +1 hour
    });
    if (this.onload) {
        this.onload();
    }
});

const mockXMLHttpRequest = jest.fn(() => ({
    onload: null,
    onerror: null,
    open: mockOpen,
    send: mockSend,
}));
global.XMLHttpRequest = mockXMLHttpRequest;

const owm = require('../../../src/js-modern/weather/openweathermap');

describe('openweathermap.js', () => {
    beforeEach(() => {
        jest.clearAllMocks();
    });

    test('fetch_weather_data makes a GET request to the correct URL', async () => {
        const latitude = 12.34;
        const longitude = 56.78;
        const expectedUrl = `https://api.openweathermap.org/data/4.0/onecall/timeline/15min?lat=${latitude}&lon=${longitude}&appid=test_api_key`;

        await owm.fetch_weather_data(latitude, longitude);

        expect(mockXMLHttpRequest).toHaveBeenCalledTimes(1);
        expect(mockOpen).toHaveBeenCalledWith('GET', expectedUrl);
        expect(mockSend).toHaveBeenCalledTimes(1);
    });

    test('fetch_weather_data returns parsed JSON on success', async () => {
        const latitude = 12.34;
        const longitude = 56.78;
        const expectedData = {
            "data": [
                {
                    "dt": 1678886400,
                    "temp": 280.15,
                    "weather": [{"main": "Clouds"}],
                    "rain": {"1h": 0.5},
                    "pop": 0.2
                }
            ],
            "timezone_offset": 3600
        };

        const result = await owm.fetch_weather_data(latitude, longitude);

        expect(result).toEqual(expectedData);
    });

    test('fetch_weather_data rejects with an error on network error', async () => {
        mockSend.mockImplementationOnce(function () {
            if (this.onerror) {
                this.onerror();
            }
        });

        const latitude = 12.34;
        const longitude = 56.78;

        await expect(owm.fetch_weather_data(latitude, longitude)).rejects.toThrow('Network error');
    });
});
