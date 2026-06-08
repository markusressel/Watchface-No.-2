const Persistence = require('../../src/js-modern/persistence');

// Mock localStorage if it does not exist
if (typeof localStorage === 'undefined') {
    global.localStorage = {
        setItem: jest.fn(),
        getItem: jest.fn(),
        clear: jest.fn(),
    };
} else {
    // If it exists in jsdom, we can spy on it
    jest.spyOn(localStorage, 'setItem');
    jest.spyOn(localStorage, 'getItem');
}

describe('Persistence', () => {
    beforeEach(() => {
        jest.clearAllMocks();
        if (localStorage.clear) {
            localStorage.clear();
        }
    });

    describe('putJson', () => {
        test('should stringify and store the value', () => {
            const key = Persistence.StorageKeys.WEATHER_LAST_DATA_KEY;
            const value = {a: 1, b: 'test'};
            Persistence.putJson(key, value);
            expect(localStorage.setItem).toHaveBeenCalledWith(key, JSON.stringify(value));
        });
    });

    describe('getJson', () => {
        test('should retrieve the value from localStorage', () => {
            const key = Persistence.StorageKeys.WEATHER_LAST_DATA_KEY;
            const value = {a: 1, b: 'test'};
            // Mock the return value for this specific call
            localStorage.getItem = jest.fn().mockReturnValueOnce(JSON.stringify(value));

            const result = Persistence.getJson(key);
            expect(localStorage.getItem).toHaveBeenCalledWith(key);
            expect(result).toEqual(value);
        });

        test('should return null if value is not set', () => {
            const key = 'testKey';
            localStorage.getItem = jest.fn().mockReturnValueOnce(null);
            const result = Persistence.getJson(key);
            expect(result).toBeNull();
        });
    });

    describe('putString', () => {
        test('should store string value', () => {
            const key = 'testKey';
            const value = 123;
            Persistence.putString(key, value);
            expect(localStorage.setItem).toHaveBeenCalledWith(key, '123');
        });
    });

    describe('getInt', () => {
        test('should retrieve int value', () => {
            const key = 'testKey';
            localStorage.getItem = jest.fn().mockReturnValueOnce('123');
            const result = Persistence.getInt(key);
            expect(localStorage.getItem).toHaveBeenCalledWith(key);
            expect(result).toBe(123);
        });

        test('should return null if value is not set', () => {
            const key = 'testKey';
            localStorage.getItem = jest.fn().mockReturnValueOnce(null);
            const result = Persistence.getInt(key);
            expect(result).toBeNull();
        });

        test('should return null if value is not a number', () => {
            const key = 'testKey';
            localStorage.getItem = jest.fn().mockReturnValueOnce('abc');
            const result = Persistence.getInt(key);
            expect(result).toBeNull();
        });
    });
});
