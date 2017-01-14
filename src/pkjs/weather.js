var config = require('./config');

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  // Construct URL
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude + '&appid=' + config.getWeatherApiKey();
  
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      
      console.log("JSON response is: " + JSON.stringify(json));

      // Temperature in Kelvin requires adjustment
      var temperatureCurrent = Math.round(json.main.temp - 273.15);
      console.log("Current Temperature is " + temperatureCurrent);
      var temperatureMin = Math.round(json.main.temp_min - 273.15);
      console.log("Max Temperature is " + temperatureMin);
      var temperatureMax = Math.round(json.main.temp_max - 273.15);
      console.log("Min Temperature is " + temperatureMax);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log("Conditions are " + conditions);
      
      // Assemble dictionary using our keys
      var dictionary = {
        "WEATHER_TEMPERATURE_CURRENT": temperatureCurrent,
        "WEATHER_TEMPERATURE_MIN": temperatureMin,
        "WEATHER_TEMPERATURE_MAX": temperatureMax,
        "WEATHER_CONDITION": conditions
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }      
  );
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

module.exports.getWeather = getWeather;