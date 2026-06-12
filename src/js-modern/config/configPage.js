export default [
    {
        "type": "heading",
        "defaultValue": "Watchface No. 2"
    },
    {
        "type": "text",
        "defaultValue": "Configure colors and other settings."
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
                        "label": "Light",
                        "value": "LIGHT"
                    },
                    {
                        "label": "Dark",
                        "value": "DARK"
                    },
                    {
                        "label": "Custom",
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
                "type": "text",
                "defaultValue": "These values will only have effect if you have selected \"Custom\" Theme in the section above."
            },
            {
                "type": "text",
                "defaultValue": "General"
            },
            {
                "type": "color",
                "messageKey": "BackgroundColor",
                "defaultValue": "0x000000",
                "label": "Background Color"
            },
            {
                "type": "text",
                "defaultValue": "Time & Date"
            },
            {
                "type": "color",
                "messageKey": "TimeTextColor",
                "defaultValue": "0xFFFFFF",
                "label": "Time Color"
            },
            {
                "type": "color",
                "messageKey": "DateTextColor",
                "defaultValue": "0xFFFFFF",
                "label": "Date Color"
            },
            {
                "type": "text",
                "defaultValue": "Battery"
            },
            {
                "type": "color",
                "messageKey": "BatteryFrameColor",
                "defaultValue": "0xFFFFFF",
                "label": "Battery Frame Color"
            },
            {
                "type": "color",
                "messageKey": "BatteryFillColor",
                "defaultValue": "0xFFFFFF",
                "label": "Battery Fill Color"
            },
            {
                "type": "text",
                "defaultValue": "Weather"
            },
            {
                "type": "color",
                "messageKey": "WeatherTextColor",
                "defaultValue": "0xFFFFFF",
                "label": "Weather Text Color"
            },
            {
                "type": "text",
                "defaultValue": "Stepcount"
            },
            {
                "type": "color",
                "messageKey": "StepcountTextColor",
                "defaultValue": "0xFFFFFF",
                "label": "Stepcount Text Color"
            },
            {
                "type": "text",
                "defaultValue": "Heartrate"
            },
            {
                "type": "color",
                "messageKey": "HeartrateTextColor",
                "defaultValue": "0xFFFFFF",
                "label": "Heartrate Text Color"
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
                "type": "slider",
                "messageKey": "SliderDigitWidth",
                "defaultValue": 4,
                "label": "Digit Width",
                "description": "Amount of dots used to display digits",
                "min": 3,
                "max": 5,
                "step": 1
            },
            {
                "type": "slider",
                "messageKey": "SliderDotWidth",
                "defaultValue": 3,
                "label": "Dot Width",
                "description": "Width in Pixel for a single dot",
                "min": 1,
                "max": 5,
                "step": 1
            },
            {
                "type": "slider",
                "messageKey": "SliderDotHeight",
                "defaultValue": 3,
                "label": "Dot Height",
                "description": "Height in Pixel for a single dot",
                "min": 1,
                "max": 5,
                "step": 1
            },
            {
                "type": "slider",
                "messageKey": "SliderDotHorizontalGap",
                "defaultValue": 3,
                "label": "Dot Horizontal Gap",
                "description": "Horizontal gap in pixels between two dots",
                "min": 1,
                "max": 5,
                "step": 1
            },
            {
                "type": "slider",
                "messageKey": "SliderDotVerticalGap",
                "defaultValue": 3,
                "label": "Dot Vertical Gap",
                "description": "Vertical gap in pixels between two dots",
                "min": 1,
                "max": 5,
                "step": 1
            },
            {
                "type": "toggle",
                "messageKey": "ToggleDotAutoScale",
                "label": "Auto Scale",
                "defaultValue": true
            },
            {
                "type": "slider",
                "messageKey": "SliderDotScaleFactorPercent",
                "defaultValue": 100,
                "label": "Manual Scale (%)",
                "description": "Used when Auto Scale is disabled",
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
                "defaultValue": "More Settings"
            },
            {
                "type": "toggle",
                "messageKey": "ShowYear",
                "label": "Show Year",
                "defaultValue": false
            },
            {
                "type": "toggle",
                "messageKey": "ShowSeconds",
                "label": "Enable Seconds",
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
                "label": "Weekday in uppercase",
                "defaultValue": false
            }
        ]
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Row Layout"
            },
            {
                "type": "text",
                "defaultValue": "Choose how many rows to render and what each row should display."
            },
            {
                "type": "text",
                "defaultValue": "Rows 6 and 7 only take effect if Row Count is set to 6 or 7."
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
                "defaultValue": "Weather Settings"
            },
            {
                "type": "text",
                "defaultValue": "This watchface uses https://open-meteo.com/"
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
