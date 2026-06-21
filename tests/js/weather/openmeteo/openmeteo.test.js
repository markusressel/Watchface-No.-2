const mockData = require('./mock/minutely_15.json');

// Mock XMLHttpRequest
const mockOpen = jest.fn();
const mockSend = jest.fn(function () {
    this.responseText = JSON.stringify(mockData);
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

const openmeteo = require('../../../../src/js-modern/weather/openmeteo/openmeteo');

describe('openmeteo.js', () => {
    beforeEach(() => {
        jest.clearAllMocks();
    });

    test('fetchWeatherData makes a GET request to the correct URL', async () => {
        const latitude = 12.34;
        const longitude = 56.78;
        const forecastHours = 6;
        const expectedUrl = `https://api.open-meteo.com/v1/forecast?latitude=${latitude}&longitude=${longitude}&minutely_15=temperature_2m,apparent_temperature,precipitation,rain&models=best_match&forecast_days=1&forecast_minutely_15=${4 * forecastHours}&timezone=auto`;

        await openmeteo.fetchWeatherData(latitude, longitude, forecastHours);

        expect(mockXMLHttpRequest).toHaveBeenCalledTimes(1);
        expect(mockOpen).toHaveBeenCalledWith('GET', expectedUrl);
        expect(mockSend).toHaveBeenCalledTimes(1);
    });

    test('fetchWeatherData returns parsed JSON on success', async () => {
        const latitude = 12.34;
        const longitude = 56.78;

        const result = await openmeteo.fetchWeatherData(latitude, longitude);

        expect(result).toEqual(mockData);
    });

    test('fetchWeatherData rejects with an error on network error', async () => {
        mockSend.mockImplementationOnce(function () {
            if (this.onerror) {
                this.onerror();
            }
        });

        const latitude = 12.34;
        const longitude = 56.78;

        await expect(openmeteo.fetchWeatherData(latitude, longitude)).rejects.toThrow('Network error');
    });
});
