import * as config from '../../src/js-modern/config/config';

jest.mock('@rebble/clay', () => {
    const mockClay = jest.fn();
    mockClay.prepareSettingsForAppMessage = jest.fn((settings) => {
        return { mocked: true, ...settings };
    });
    return mockClay;
});

describe('index.js settings sync', () => {
    let pebbleListeners;
    let Clay;

    beforeEach(() => {
        pebbleListeners = {};

        global.Pebble = {
            addEventListener: jest.fn((event, callback) => {
                pebbleListeners[event] = callback;
            }),
            sendAppMessage: jest.fn(),
        };

        global.localStorage = {
            getItem: jest.fn((key) => {
                if (key === 'clay-settings') {
                    return JSON.stringify({
                        Theme: 'LIGHT',
                        ShowSeconds: true,
                        BackgroundColor: '0xFF0000',
                        LayoutRowCount: 5
                    });
                }
                return null;
            }),
            setItem: jest.fn(),
            removeItem: jest.fn(),
            clear: jest.fn()
        };

        // Clear module registry to ensure index.js runs and registers listeners
        jest.resetModules();

        Clay = require('@rebble/clay');
        Clay.prepareSettingsForAppMessage.mockClear();

        // Require the file so it registers listeners
        require('../../src/js-modern/index');
    });

    it('should format booleans to integers when sending initial settings to watch', () => {
        // Trigger AppReady to set isPebbleReady = true
        expect(pebbleListeners['appmessage']).toBeDefined();
        pebbleListeners['appmessage']({
            payload: {
                AppReady: 1
            }
        });

        // Trigger RequestSettings
        pebbleListeners['appmessage']({
            payload: {
                RequestSettings: 1
            }
        });

        // Assert Clay.prepareSettingsForAppMessage was called
        expect(Clay.prepareSettingsForAppMessage).toHaveBeenCalled();
        const calledArg = Clay.prepareSettingsForAppMessage.mock.calls[0][0];
        expect(calledArg.Theme).toBe('LIGHT');
        expect(calledArg.ShowSeconds).toBe(true);
        expect(calledArg.BackgroundColor).toBe('0xFF0000');
        expect(calledArg.LayoutRowCount).toBe(5);

        // Assert sendAppMessage was called with formatted dict
        expect(global.Pebble.sendAppMessage).toHaveBeenCalled();
        const sentDict = global.Pebble.sendAppMessage.mock.calls[0][0];
        
        expect(sentDict).toBeDefined();
        expect(sentDict.mocked).toBe(true);
        expect(sentDict.Theme).toBe('LIGHT');
    });
});
