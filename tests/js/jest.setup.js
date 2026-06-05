// jest.setup.js
const {Console} = require('console');

// Create a real Node console that prints directly to stdout/stderr
const nativeConsole = new Console({
    stdout: process.stdout,
    stderr: process.stderr
});

// Helper function: Converts Hex to 24-bit ANSI escape codes
function hexToAnsi(hex) {
    hex = hex.replace(/^#/, '');
    const r = parseInt(hex.substring(0, 2), 16);
    const g = parseInt(hex.substring(2, 4), 16);
    const b = parseInt(hex.substring(4, 6), 16);
    return `\x1b[38;2;${r};${g};${b}m`;
}

// Color configurations
const colors = {
    pink: hexToAnsi('#FF69B4'),
    green: hexToAnsi('#4ADE80'),
    red: hexToAnsi('#F87171'),
    yellow: hexToAnsi('#FBBF24'),
    reset: '\x1b[0m'
};

// Formats the date: YY-MM-DD HH:mm:ss.ms
function getFormattedTimestamp() {
    const now = new Date();
    const year = String(now.getFullYear()).slice(-2);
    const month = String(now.getMonth() + 1).padStart(2, '0');
    const day = String(now.getDate()).padStart(2, '0');
    const hours = String(now.getHours()).padStart(2, '0');
    const minutes = String(now.getMinutes()).padStart(2, '0');
    const seconds = String(now.getSeconds()).padStart(2, '0');
    const ms = String(now.getMilliseconds()).padStart(2, '0').slice(0, 2);

    return `${year}-${month}-${day} ${hours}:${minutes}:${seconds}.${ms}`;
}

// Intercept before each test
beforeEach(() => {
    // 1. Define how each log level should look
    const logConfigs = [
        {method: 'log', emoji: '🟢', color: colors.green, label: 'info'},
        {method: 'info', emoji: '🟢', color: colors.pink, label: 'info'},
        {method: 'warn', emoji: '🟠', color: colors.yellow, label: 'warn'},
        {method: 'error', emoji: '🔴', color: colors.red, label: 'error'}
    ];

    // 2. Dynamically build the overriding functions
    const customLoggers = {};
    logConfigs.forEach(({method, emoji, color, label}) => {
        customLoggers[method] = (...args) => {
            // Inline the timestamp fetch directly into the template string
            nativeConsole[method](`${emoji} ${getFormattedTimestamp()} ${color}${label}${colors.reset} -`, ...args);
        };
    });

    // 3. Apply them to the global console
    global.console = {
        ...global.console,
        ...customLoggers
    };
});

// Restore after each test so Jest can clean up
afterEach(() => {
    jest.restoreAllMocks();
});