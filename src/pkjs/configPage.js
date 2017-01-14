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
        "defaultValue": "DARK",
        "options": [
          { 
            "label": "Dark", 
            "value": "DARK" 
          },
          { 
            "label": "Light", 
            "value": "LIGHT" 
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
        "type": "color",
        "messageKey": "ForegroundColor",
        "defaultValue": "0xFFFFFF",
        "label": "Foreground Color"
      },
      {
        "type": "color",
        "messageKey": "TextColor",
        "defaultValue": "0xFFFFFF",
        "label": "Text Color"
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
        "defaultValue": "Connection Indicator"
      },
      {
        "type": "color",
        "messageKey": "ConnectionIconColor",
        "defaultValue": "0xFFFFFF",
        "label": "Connection Icon Color"
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
        "type": "color",
        "messageKey": "BatteryTextColor",
        "defaultValue": "0xFFFFFF",
        "label": "Battery Text Color"
      },
      {
        "type": "text",
        "defaultValue": "Weather"
      },
      {
        "type": "color",
        "messageKey": "WeatherIconColor",
        "defaultValue": "0xFFFFFF",
        "label": "Weather Icon Color"
      },
      {
        "type": "color",
        "messageKey": "WeatherTextColor",
        "defaultValue": "0xFFFFFF",
        "label": "Weather Text Color"
      },
      {
        "type": "text",
        "defaultValue": "Heartrate"
      },
      {
        "type": "color",
        "messageKey": "HeartColor",
        "defaultValue": "0xFF0000",
        "label": "Heart Color"
      },
      {
        "type": "color",
        "messageKey": "HeartrateTextColor",
        "defaultValue": "0xFFFFFF",
        "label": "BPM Text Color"
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
        "messageKey": "ShowSeconds",
        "label": "Enable Seconds",
        "defaultValue": false
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];