function getWeatherApiKey() {
  // Clay stores all settings in 'clay-settings' as a JSON blob
  try {
    var claySettings = JSON.parse(localStorage.getItem('clay-settings')) || {};
    return claySettings.WeatherApiKey || '';
  } catch (e) {
    return '';
  }
}

module.exports.getWeatherApiKey = getWeatherApiKey;
