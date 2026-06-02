var CLAY_SETTINGS_KEY = 'clay-settings';

function getClaySettings() {
  try {
    return JSON.parse(localStorage.getItem(CLAY_SETTINGS_KEY)) || {};
  } catch (e) {
    return {};
  }
}

function setClaySettings(settings) {
  try {
    localStorage.setItem(CLAY_SETTINGS_KEY, JSON.stringify(settings || {}));
  } catch (e) {
    console.log('Could not save clay settings: ' + e);
  }
}

function getWeatherApiKey() {
  return getClaySettings().WeatherApiKey || '';
}

function isWeatherSimulationEnabled() {
  return !!getClaySettings().WeatherUseSimulation;
}

function setWeatherSimulationEnabled(enabled) {
  var settings = getClaySettings();
  settings.WeatherUseSimulation = !!enabled;
  setClaySettings(settings);
}

function toggleWeatherSimulation() {
  var nextState = !isWeatherSimulationEnabled();
  setWeatherSimulationEnabled(nextState);
  return nextState;
}

module.exports.getWeatherApiKey = getWeatherApiKey;
module.exports.isWeatherSimulationEnabled = isWeatherSimulationEnabled;
module.exports.setWeatherSimulationEnabled = setWeatherSimulationEnabled;
module.exports.toggleWeatherSimulation = toggleWeatherSimulation;
