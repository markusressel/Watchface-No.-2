// Define 'seconds' as a getter on the Number prototype
Object.defineProperty(Number.prototype, 'seconds', {
    get: function () {
        return this * 1000;
    }
});

// You can easily chain others while you're at it!
Object.defineProperty(Number.prototype, 'minutes', {
    get: function () {
        return this * 60 * 1000;
    }
});