export default [
    {
        "type": "heading",
        "defaultValue": "Watchface No. 2"
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Row Layout"
            },
            {
                "type": "markdown",
                "defaultValue": "Choose <b>how many</b> rows to render <b>and</b> what each row should display.<br>Rows 6 and 7 only take effect if Row Count is set to 6 or 7."
            },
            {
                "type": "slider",
                "messageKey": "LayoutRowCount",
                "defaultValue": 5,
                "label": "Row Count",
                "description": "Total visible rows",
                "min": 5,
                "max": 7,
                "step": 1
            },
            {
                "type": "subheader",
                "defaultValue": "Row Widget Selection"
            },
            {
                "type": "select",
                "messageKey": "Row0Widget",
                "defaultValue": 0,
                "label": "Row 1",
                "options": [
                    {"label": "Weather", "value": 0},
                    {"label": "Date", "value": 1},
                    {"label": "Time", "value": 2},
                    {"label": "Stepcount", "value": 3},
                    {"label": "Battery", "value": 4},
                    {"label": "Heartrate", "value": 5},
                    {"label": "Weather Forecast", "value": 6},
                ]
            },
            {
                "type": "select",
                "messageKey": "Row1Widget",
                "defaultValue": 1,
                "label": "Row 2",
                "options": [
                    {"label": "Weather", "value": 0},
                    {"label": "Date", "value": 1},
                    {"label": "Time", "value": 2},
                    {"label": "Stepcount", "value": 3},
                    {"label": "Battery", "value": 4},
                    {"label": "Heartrate", "value": 5},
                    {"label": "Weather Forecast", "value": 6},
                ]
            },
            {
                "type": "select",
                "messageKey": "Row2Widget",
                "defaultValue": 2,
                "label": "Row 3",
                "options": [
                    {"label": "Weather", "value": 0},
                    {"label": "Date", "value": 1},
                    {"label": "Time", "value": 2},
                    {"label": "Stepcount", "value": 3},
                    {"label": "Battery", "value": 4},
                    {"label": "Heartrate", "value": 5},
                    {"label": "Weather Forecast", "value": 6},
                ]
            },
            {
                "type": "select",
                "messageKey": "Row3Widget",
                "defaultValue": 3,
                "label": "Row 4",
                "options": [
                    {"label": "Weather", "value": 0},
                    {"label": "Date", "value": 1},
                    {"label": "Time", "value": 2},
                    {"label": "Stepcount", "value": 3},
                    {"label": "Battery", "value": 4},
                    {"label": "Heartrate", "value": 5},
                    {"label": "Weather Forecast", "value": 6},
                ]
            },
            {
                "type": "select",
                "messageKey": "Row4Widget",
                "defaultValue": 4,
                "label": "Row 5",
                "options": [
                    {"label": "Weather", "value": 0},
                    {"label": "Date", "value": 1},
                    {"label": "Time", "value": 2},
                    {"label": "Stepcount", "value": 3},
                    {"label": "Battery", "value": 4},
                    {"label": "Heartrate", "value": 5},
                    {"label": "Weather Forecast", "value": 6},
                ]
            },
            {
                "type": "select",
                "messageKey": "Row5Widget",
                "defaultValue": 3,
                "label": "Row 6",
                "options": [
                    {"label": "Weather", "value": 0},
                    {"label": "Date", "value": 1},
                    {"label": "Time", "value": 2},
                    {"label": "Stepcount", "value": 3},
                    {"label": "Battery", "value": 4},
                    {"label": "Heartrate", "value": 5},
                    {"label": "Weather Forecast", "value": 6},
                ]
            },
            {
                "type": "select",
                "messageKey": "Row6Widget",
                "defaultValue": 3,
                "label": "Row 7",
                "options": [
                    {"label": "Weather", "value": 0},
                    {"label": "Date", "value": 1},
                    {"label": "Time", "value": 2},
                    {"label": "Stepcount", "value": 3},
                    {"label": "Battery", "value": 4},
                    {"label": "Heartrate", "value": 5},
                    {"label": "Weather Forecast", "value": 6},
                ]
            }
        ]
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Widget-Specific Settings"
            },
            {
                "type": "text",
                "defaultValue": "Configure widget-specific behavior."
            },
            {
                "type": "subheader",
                "defaultValue": "Time"
            },
            {
                "type": "toggle",
                "messageKey": "ShowSeconds",
                "label": "Enable Seconds",
                "description": "(Discouraged: will cause battery drain)",
                "defaultValue": false
            },
            {
                "type": "subheader",
                "defaultValue": "Date"
            },
            {
                "type": "toggle",
                "messageKey": "ShowYear",
                "label": "Show Year",
                "defaultValue": false
            },
            {
                "type": "toggle",
                "messageKey": "ShowWeekdayAbbreviation",
                "label": "Enable Weekday",
                "defaultValue": false
            },
            {
                "type": "toggle",
                "messageKey": "WeekdayAbbreviationUppercase",
                "label": "Weekday in CAPS (MO, TU, WE, etc.)",
                "defaultValue": false
            },
            {
                "type": "subheader",
                "defaultValue": "Weather"
            },
            {
                "type": "text",
                "defaultValue": "This watchface uses https://open-meteo.com/"
            },
            {
                "type": "select",
                "messageKey": "WeatherSlot1",
                "defaultValue": 2,
                "label": "Left Slot",
                "options": [
                    {"label": "None", "value": 0},
                    {"label": "Current Temp", "value": 1},
                    {"label": "Max Temp", "value": 2},
                    {"label": "Min Temp", "value": 3}
                ]
            },
            {
                "type": "select",
                "messageKey": "WeatherSlot2",
                "defaultValue": 1,
                "label": "Middle Slot",
                "options": [
                    {"label": "None", "value": 0},
                    {"label": "Current Temp", "value": 1},
                    {"label": "Max Temp", "value": 2},
                    {"label": "Min Temp", "value": 3}
                ]
            },
            {
                "type": "select",
                "messageKey": "WeatherSlot3",
                "defaultValue": 3,
                "label": "Right Slot",
                "options": [
                    {"label": "None", "value": 0},
                    {"label": "Current Temp", "value": 1},
                    {"label": "Max Temp", "value": 2},
                    {"label": "Min Temp", "value": 3}
                ]
            },
            {
                "type": "subheader",
                "defaultValue": "Weather Forecast"
            },
            {
                "type": "slider",
                "messageKey": "SliderWeatherForecastPreviewHoursCount",
                "defaultValue": 6,
                "label": "Weather Forecast Preview Hours",
                "description": "Number of hours to preview in weather forecast",
                "min": 1,
                "max": 24,
                "step": 1
            },
            {
                "type": "slider",
                "messageKey": "WeatherUpdateIntervalMinutes",
                "defaultValue": 15,
                "label": "Weather Update Interval",
                "description": "How often the weather should update (minutes)",
                "min": 15,
                "max": 180,
                "step": 15
            },
            {
                "type": "button",
                "id": "clear_weather_cache_button",
                "defaultValue": "Clear Weather Cache",
                "primary": false
            },
            {
                "type": "toggle",
                "messageKey": "ClearWeatherCache",
                "label": "Clear Weather Cache on Save",
                "defaultValue": false,
                "id": "hidden_clear_cache"
            }
        ]
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Dotted Font Settings"
            },
            {
                "type": "markdown",
                "defaultValue": "These settings define the <b>base shape</b> of the dotted characters. They apply regardless of scaling mode."
            },
            {
                "type": "slider",
                "messageKey": "SliderDigitWidth",
                "defaultValue": 4,
                "label": "Digit Width",
                "description": "Number of vertical dot columns used per digit.",
                "min": 3,
                "max": 5,
                "step": 1
            },
            {
                "type": "slider",
                "messageKey": "SliderDotWidth",
                "defaultValue": 3,
                "label": "Dot Width",
                "description": "Internal width of a single dot.",
                "min": 1,
                "max": 5,
                "step": 1
            },
            {
                "type": "slider",
                "messageKey": "SliderDotHeight",
                "defaultValue": 3,
                "label": "Dot Height",
                "description": "Internal height of a single dot.",
                "min": 1,
                "max": 5,
                "step": 1
            },
            {
                "type": "slider",
                "messageKey": "SliderDotHorizontalGap",
                "defaultValue": 3,
                "label": "Horizontal Gap",
                "description": "Internal space between dots horizontally.",
                "min": 1,
                "max": 5,
                "step": 1
            },
            {
                "type": "slider",
                "messageKey": "SliderDotVerticalGap",
                "defaultValue": 3,
                "label": "Vertical Gap",
                "description": "Internal space between dots vertically.",
                "min": 1,
                "max": 5,
                "step": 1
            },
            {
                "type": "subheader",
                "defaultValue": "Scaling & Layout"
            },
            {
                "type": "toggle",
                "messageKey": "ToggleDotAutoScale",
                "label": "Enable Auto Scale",
                "defaultValue": true
            },
            {
                "type": "markdown",
                "defaultValue": "When <b>Auto Scale</b> is <b>Enabled</b>:<ul><li>Widgets automatically stretch to fill the available row height.</li><li>Spacing between rows is balanced automatically.</li><li>Use the <b>Time Row Size Ratio</b> below to control the clock's relative size.</li></ul>"
            },
            {
                "type": "slider",
                "messageKey": "SliderTimeRowRatioPercent",
                "defaultValue": 120,
                "label": "Time Row Size Ratio (%)",
                "description": "Relative size of the Time row (e.g., 120% means the clock is 20% larger than other rows).",
                "min": 100,
                "max": 300,
                "step": 10
            },
            {
                "type": "markdown",
                "defaultValue": "When <b>Auto Scale</b> is <b>Disabled</b>:<ul><li>Widgets use the <b>Manual Scale</b> factor below.</li><li>Dot sizes remain uniform across all widgets (<b>Pixel Perfect</b>).</li></ul>"
            },
            {
                "type": "slider",
                "messageKey": "SliderDotScaleFactorPercent",
                "defaultValue": 100,
                "label": "Manual Scale (%)",
                "description": "Global multiplier for the base font metrics.",
                "min": 50,
                "max": 300,
                "step": 5
            }
        ]
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Theme"
            },
            {
                "type": "radiogroup",
                "messageKey": "Theme",
                "label": "Theme",
                "defaultValue": "LIGHT",
                "options": [
                    {
                        "label": "Light - Black on White Background",
                        "value": "LIGHT"
                    },
                    {
                        "label": "Dark - White on Black Background",
                        "value": "DARK"
                    },
                    {
                        "label": "Custom - see below",
                        "value": "CUSTOM"
                    }
                ]
            }
        ]
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Colors"
            },
            {
                "type": "markdown",
                "defaultValue": "These values <b>will only have an effect</b> if you have selected \"Custom\" Theme in the section above."
            },
            {
                "type": "subheader",
                "defaultValue": "General"
            },
            {
                "type": "color",
                "messageKey": "BackgroundColor",
                "defaultValue": "0x000000",
                "label": "Background"
            },
            {
                "type": "subheader",
                "defaultValue": "Time & Date"
            },
            {
                "type": "color",
                "messageKey": "TimeTextColor",
                "defaultValue": "0xFFFFFF",
                "label": "Time"
            },
            {
                "type": "color",
                "messageKey": "DateTextColor",
                "defaultValue": "0xFFFFFF",
                "label": "Date"
            },
            {
                "type": "subheader",
                "defaultValue": "Battery"
            },
            {
                "type": "color",
                "messageKey": "BatteryFrameColor",
                "defaultValue": "0xFFFFFF",
                "label": "Frame"
            },
            {
                "type": "color",
                "messageKey": "BatteryFillColor",
                "defaultValue": "0xFFFFFF",
                "label": "Bar"
            },
            {
                "type": "subheader",
                "defaultValue": "Weather"
            },
            {
                "type": "color",
                "messageKey": "WeatherTextColor",
                "defaultValue": "0xFFFFFF",
                "label": "Weather Forecast"
            },
            {
                "type": "color",
                "messageKey": "WeatherMaxTempColor",
                "defaultValue": "0xFF0000",
                "label": "Max Temperature"
            },
            {
                "type": "color",
                "messageKey": "WeatherCurrentTempColor",
                "defaultValue": "0xFFFFFF",
                "label": "Current Temperature"
            },
            {
                "type": "color",
                "messageKey": "WeatherMinTempColor",
                "defaultValue": "0x00AAFF",
                "label": "Min Temperature"
            },
            {
                "type": "subheader",
                "defaultValue": "Stepcount"
            },
            {
                "type": "color",
                "messageKey": "StepcountTextColor",
                "defaultValue": "0xFFFFFF",
                "label": "Text"
            },
            {
                "type": "subheader",
                "defaultValue": "Heartrate"
            },
            {
                "type": "color",
                "messageKey": "HeartrateTextColor",
                "defaultValue": "0xFFFFFF",
                "label": "Text"
            }
        ]
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Developer Options"
            },
            {
                "type": "text",
                "defaultValue": "Enable simulation mode to use bundled timeline.json weather data instead of live API requests."
            },
            {
                "type": "toggle",
                "messageKey": "WeatherUseSimulation",
                "label": "Use simulated weather data",
                "defaultValue": false
            }
        ]
    },
    {
        "type": "submit",
        "defaultValue": "Save Settings"
    }
];
