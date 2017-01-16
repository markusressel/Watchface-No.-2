module.exports = [
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
        "defaultValue": 5,
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
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];