var Clay = require('@rebble/clay');
var clayConfig = require('./configPage');
var clay = new Clay(clayConfig);

var config = require('./config');
var weather = require('./weather');

var isPebbleReady = false;

// When the config page is closed, Clay saves settings to 'clay-settings' in
// localStorage (and sends watch-bound keys to the watch). We add our own
// webviewclosed listener here to react to JS-only settings like WeatherApiKey.
Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response) { return; }

  // Refresh weather: will clear display if API key is empty, or fetch if set.
    if (isPebbleReady) {
        weather.getWeather();
    }
});

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function(e) {
    console.log("PebbleKit JS ready!");

      // ==========================================
      // TEMPORARY RESET CODE
      // ==========================================
      // console.log("Wiping Clay settings...");
      // localStorage.clear();

      // We no longer send weather here immediately. We wait for the AppReady message,
      // to avoid overwhelming a not-yet-initialized watchface.
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    // Get the dictionary from the message
    var dict = e.payload;
    
    console.log('AppMessage received: ' + JSON.stringify(dict));

      if ("AppReady" in dict) {
          console.log("Watchface is ready! Sending pending data.");
          isPebbleReady = true;
          weather.getWeather();
      } else if ("RequestData" in dict) {
          if (isPebbleReady) {
              weather.getWeather();
          }
    }
  }                     
);