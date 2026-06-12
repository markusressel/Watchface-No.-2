export default function (minified) {
    var clayConfig = this;

    clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function () {
        var clearWeatherButton = clayConfig.getItemById('clear_weather_cache_button');
        var hiddenClearCacheToggle = clayConfig.getItemById('hidden_clear_cache');

        if (!clearWeatherButton || !hiddenClearCacheToggle) {
            console.error("Could not find components for weather cache clearing");
            return;
        }

        // Initially hide the toggle, we'll show it or just use it silently
        hiddenClearCacheToggle.hide();

        clearWeatherButton.on('click', function () {
            try {
                // We set the hidden toggle to true. 
                // When the user clicks "Save", index.js will see this and clear the cache.
                hiddenClearCacheToggle.set(true);

                // Provide feedback to the user
                alert("Cache will be cleared when you click 'Save Settings'.");

                // Optional: change button text to indicate it's queued
                // clearWeatherButton.set("Cache will clear on Save");
            } catch (e) {
                console.error("Error queueing weather cache clear: " + e);
                alert("Failed to queue cache clear: " + e.message);
            }
        });
    });
};
