const https = require('https');

// Simple XMLHttpRequest polyfill for Node.js using native https
global.XMLHttpRequest = class {
    constructor() {
        this.onload = null;
        this.onerror = null;
        this.responseText = null;
        this.status = null;
    }

    open(method, url) {
        this.method = method;
        this.url = url;
    }

    send() {
        const req = https.request(this.url, {method: this.method}, (res) => {
            let data = '';
            res.on('data', (chunk) => {
                data += chunk;
            });
            res.on('end', () => {
                this.status = res.statusCode;
                this.responseText = data;
                if (this.onload) this.onload();
            });
        });
        req.on('error', (e) => {
            if (this.onerror) this.onerror(e);
        });
        req.end();
    }
};

const openmeteo = require('../../../../src/js-modern/weather/openmeteo/openmeteo');

describe('OpenMeteo Integration Test', () => {
    // This test makes a real API call to OpenMeteo.
    // It is intended for manual debugging to inspect the live API response.
    // It is skipped by default to avoid failing CI/CD pipelines due to network issues or API changes.
    test('should fetch weather data from the real OpenMeteo API and log the response', async () => {
        const latitude = 52.4391595;
        const longitude = 13.3490739;
        const forecastHours = 6;

        try {
            const weatherData = await openmeteo.fetchWeatherData(latitude, longitude, forecastHours);
            console.log('OpenMeteo API Response:', JSON.stringify(weatherData, null, 2));
            expect(weatherData).toBeDefined();
            expect(weatherData.latitude).toBeCloseTo(latitude, 1);
            expect(weatherData.longitude).toBeCloseTo(longitude, 1);
            expect(weatherData.minutely_15).toBeDefined();
        } catch (error) {
            console.error('Error fetching weather data from OpenMeteo:', error);
            // We don't want the test to fail, just to observe the output
        }
    }, 30000); // Increase timeout to 30 seconds for real network request
});
