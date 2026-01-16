var Clay = require('@rebble/clay');
var clayConfig = require('./configPage');
var clay = new Clay(clayConfig);

var weather = require('./weather');


// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial weather
    weather.getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    // Get the dictionary from the message
    var dict = e.payload;
    
    console.log('AppMessage received: ' + JSON.stringify(dict));
    
    if ("RequestData" in dict) {
      weather.getWeather();
    }
  }                     
);