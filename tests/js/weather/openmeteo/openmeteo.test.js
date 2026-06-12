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

    test('fetch_weather_data makes a GET request to the correct URL', async () => {
        const latitude = 12.34;
        const longitude = 56.78;
        const forecast_hours = 6;
        const expectedUrl = `https://api.open-meteo.com/v1/forecast?latitude=${latitude}&longitude=${longitude}&hourly=temperature_2m,rain,precipitation,precipitation_probability,apparent_temperature&minutely_15=temperature_2m,apparent_temperature,precipitation,rain&forecast_days=1&forecast_minutely_15=24&timezone=auto`;

        await openmeteo.fetch_weather_data(latitude, longitude, forecast_hours);

        expect(mockXMLHttpRequest).toHaveBeenCalledTimes(1);
        expect(mockOpen).toHaveBeenCalledWith('GET', expectedUrl);
        expect(mockSend).toHaveBeenCalledTimes(1);
    });

    test('fetch_weather_data returns parsed JSON on success', async () => {
        const latitude = 12.34;
        const longitude = 56.78;

        const result = await openmeteo.fetch_weather_data(latitude, longitude);

        expect(result).toEqual(mockData);
    });

    test('fetch_weather_data rejects with an error on network error', async () => {
        mockSend.mockImplementationOnce(function () {
            if (this.onerror) {
                this.onerror();
            }
        });

        const latitude = 12.34;
        const longitude = 56.78;

        await expect(openmeteo.fetch_weather_data(latitude, longitude)).rejects.toThrow('Network error');
    });
});
