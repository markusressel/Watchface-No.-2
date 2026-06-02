var config = require('./config');

function kelvin_to_celsius(kelvin) {
  if (typeof kelvin !== 'number') {
    return 0;
  }

  return Math.round(kelvin - 273.15);
}

function one_decimal_to_int(value) {
  if (typeof value !== 'number') {
    return 0;
  }

  return Math.round(value * 10);
}

function pick_closest_entry_to_now(entries) {
  if (!entries || entries.length === 0) {
    return null;
  }

  var nowUtc = Math.floor(Date.now() / 1000);
  var best = entries[0];
  var bestDistance = Math.abs((best.dt || 0) - nowUtc);

  for (var i = 1; i < entries.length; i++) {
    var candidate = entries[i];
    var distance = Math.abs((candidate.dt || 0) - nowUtc);
    if (distance < bestDistance) {
      best = candidate;
      bestDistance = distance;
    }
  }

  return best;
}

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
  var url = "https://api.openweathermap.org/data/4.0/onecall/timeline/15min?lat=" +
      pos.coords.latitude +
      "&lon=" + pos.coords.longitude +
      "&appid=" + config.getWeatherApiKey();

    console.log("Weather request URL is: " + url);
  
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      
      console.log("JSON response is: " + JSON.stringify(json));

      var timeline = json.data || [];
      var current = pick_closest_entry_to_now(timeline) || timeline[0] || {};
      var minKelvin = current.temp;
      var maxKelvin = current.temp;

      for (var i = 0; i < timeline.length; i++) {
        var t = timeline[i] && timeline[i].temp;
        if (typeof t !== 'number') {
          continue;
        }

        if (typeof minKelvin !== 'number' || t < minKelvin) {
          minKelvin = t;
        }
        if (typeof maxKelvin !== 'number' || t > maxKelvin) {
          maxKelvin = t;
        }
      }

      // Temperature in Kelvin requires adjustment
      var temperatureCurrent = kelvin_to_celsius(current.temp);
      console.log("Current Temperature is " + temperatureCurrent);
      var temperatureMin = kelvin_to_celsius(minKelvin);
      console.log("Min Temperature is " + temperatureMin);
      var temperatureMax = kelvin_to_celsius(maxKelvin);
      console.log("Max Temperature is " + temperatureMax);

      // Conditions
      var conditions = "";
      if (current.weather && current.weather.length > 0 && current.weather[0].main) {
        conditions = current.weather[0].main;
      }
      console.log("Conditions are " + conditions);

      // Rain forecast based on selected 15-minute entry
      var rainMm = (current.rain && current.rain["1h"]) ? current.rain["1h"] : 0;
      var popPercent = typeof current.pop === 'number' ? Math.round(current.pop * 100) : 0;
      console.log("Rain from selected entry (mm/h): " + rainMm + ", pop (%): " + popPercent);

      // Assemble dictionary using our keys
      var dictionary = {
        "WEATHER_TEMPERATURE_CURRENT": temperatureCurrent,
        "WEATHER_TEMPERATURE_MIN": temperatureMin,
        "WEATHER_TEMPERATURE_MAX": temperatureMax,
        "WEATHER_CONDITION": conditions,
        "WEATHER_RAIN_NEXT_HOUR_MM_X10": one_decimal_to_int(rainMm),
        "WEATHER_RAIN_POP_PERCENT": popPercent
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
  var apiKey = config.getWeatherApiKey();
  
  // If no API key is configured, clear any stale weather data
  if (!apiKey || apiKey.length === 0) {
    console.log("No OpenWeatherMap API key configured. Clearing weather data.");
    
    // Send empty weather data to clear the display on the watchface
    var clearDictionary = {
      "WEATHER_TEMPERATURE_CURRENT": 0,
      "WEATHER_TEMPERATURE_MIN": 0,
      "WEATHER_TEMPERATURE_MAX": 0,
      "WEATHER_CONDITION": "",
      "WEATHER_RAIN_NEXT_HOUR_MM_X10": 0,
      "WEATHER_RAIN_POP_PERCENT": 0
    };
    
    Pebble.sendAppMessage(clearDictionary,
      function(e) {
        console.log("Weather data cleared successfully!");
      },
      function(e) {
        console.log("Error clearing weather data!");
      }
    );
    
    return;
  }
  
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

module.exports.getWeather = getWeather;