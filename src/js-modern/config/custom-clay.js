export default function (minified) {
    var clayConfig = this;
    var _ = minified._;
    var $ = minified.$;
    var HTML = minified.HTML;

    var markdownComponent = {
        name: 'markdown',
        template: '<div class="clay-item clay-item-markdown"></div>',
        style: '.clay-item-markdown { white-space: pre-wrap; padding: 0 0.75rem; } ' +
            '.clay-item-markdown b, .clay-item-markdown strong { font-weight: bold !important; } ' +
            '.clay-item-markdown ul { list-style-type: disc !important; margin: 0.5rem 0 !important; padding-left: 1.5rem !important; } ' +
            '.clay-item-markdown li { display: list-item !important; margin-bottom: 0.2rem !important; }',
        initialize: function (min, clayConfig) {
            this.$element.ht(this.config.defaultValue || '');
        },
        manipulator: {
            get: function () {
                return this.config.defaultValue;
            },
            set: function (value) {
                this.$element.ht(value);
            }
        }
    };

    var subheaderComponent = {
        name: 'subheader',
        template: '<div class="clay-item clay-item-subheader"></div>',
        style: '.clay-item-subheader { text-align: center; padding: 0.5rem 0.75rem; } ' +
            '.clay-item-subheader b { font-weight: bold !important; }',
        initialize: function (min, clayConfig) {
            this.$element.ht('<b>' + (this.config.defaultValue || '') + '</b>');
        },
        manipulator: {
            get: function () {
                return this.config.defaultValue;
            },
            set: function (value) {
                this.$element.ht('<b>' + value + '</b>');
            }
        }
    };

    clayConfig.registerComponent(markdownComponent);
    clayConfig.registerComponent(subheaderComponent);

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
