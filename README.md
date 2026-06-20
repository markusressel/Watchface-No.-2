# Watchface No. 2

A dot-styled, pixel-perfect, highly configurable watchface for the Pebble smartwatches.

[![Pebble Platform](https://img.shields.io/badge/platform-Pebble-orange.svg)](https://pebble-render.aru.dev/)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

---

## Visual Showcase

Watchface No. 2 is designed to look clean, modern, and highly styled across all Pebble generations.

| Time 2 (Emery / Basalt Color) | Core 2 Duo (Diorite Dark & Light) |
|:---:|:---:|
| <img src="/screenshots/framed_time_2_custom_scaled.png" width="240" alt="Emery color preview"> | <img src="/screenshots/framed_diorite_dark.png" width="180" alt="Diorite dark preview">&nbsp;&nbsp;<img src="/screenshots/framed_diorite_light.png" width="180" alt="Diorite light preview"> |

---

## The Core Concept: Dynamic Pixel Rendering

Unlike traditional watchfaces, Watchface No. 2 **does not use a pre-rendered font**. Instead:

- Every digit (`0-9`), weekday name, separator, and character is defined by a **5x5 boolean pixel matrix**.
- Gaps, dot sizes, and vertical/horizontal spaces are **interpolated and rendered dynamically at runtime**.
- This guarantees pixel-perfect rendering at any size or ratio and allows the font metrics to be customized directly from the settings page!

Check out [dotted_text_layer.c](file:///home/markus/CLionProjects/Watchface-No.-2/src/c/ui/layer/dotted_text_layer.c) and [pixel_matrix_drawer.h](file:///home/markus/CLionProjects/Watchface-No.-2/src/c/ui/graphics/pixel_matrix_drawer.h) to see how it works under the hood.

---

## Layout Customization

You have complete control over how many rows are shown and what each row contains.

- **Row Count Selector**: Set the watchface to display **5, 6, or 7 rows**.
- **Widget Routing**: Freely map any widget to any row.

### Default Row Configuration (5-Row Mode)
| Row | Default Widget | Description |
|:---:|:---|:---|
| **1** | Weather | Left/Middle/Right slots showing configurable temperatures |
| **2** | Date | Customizable date representation with weekday |
| **3** | Time | The clock display (supports 12h/24h) |
| **4** | Stepcount | Daily steps indicator |
| **5** | Battery Bar | Charge level bar with animations |

---

## Feature-Rich Widgets

Every widget is highly configurable to suit your information needs:

### 🕒 Time Widget
* Supports standard `12h` or `24h` time formatting.
* Toggling **Show Seconds** is available (though discouraged to preserve battery).

### 📅 Date Widget
* Shows the day of the month, month, year, and weekday.
* **Separated Colors**: Configure the text color for the date, weekday name, and separator independently.
* **Zero Padding**: Toggle between `01.01.26` and `1.1.26` format.
* **Custom Layouts**: Toggle weekday abbreviations (e.g. `MO`, `TU`) and enforce UPPERCASE formatting.

### 🌤️ Weather Widget (Powered by Open-Meteo)
* Features **3 customizable slots** (Left, Middle, Right).
* Assign **Current Temperature**, **Max Temp**, or **Min Temp** to any slot.
* Separators between slots can be customized with their own color.

### 📈 Weather Forecast Graph Widget
* Displays a detailed graphical view of the forecast over a **1 to 24-hour preview window**.
* Renders a custom **temperature curve** and **precipitation bar graph**.
* Features interactive **color-coded gradient stops** for both temperature (from -10°C to 40°C) and rain levels (0mm to 10mm+) which interpolate dynamically.
* Indicates the current hour with a customizable vertical marker line.

### 👟 Step Count Widget
* Displays your daily steps tracked directly through Pebble Health.
* Customizable color mapping to blend in with your theme.

### 💓 Heart Rate Widget
* Displays real-time or last-recorded heart rate bpm data from Pebble Health.

### 🔋 Battery Bar Widget
* Features a clean border frame and charge level indicator.
* Smooth **charging animations** when plugged in.
* Configurable **Low Battery Threshold** (0% to 30%) with a dedicated low-power color state.
* **Custom Width**: A high-precision slider allows you to restrict the battery bar to any width from **1% to 100%** of the screen.

---

## Design, Themes & Fonts

### Auto-Scaling & Pixel-Perfect Modes
* **Auto-Scale (Enabled)**: Widgets automatically expand to fill available height and distribute spacing. Adjust the **Time Row Size Ratio (%)** to make the time display up to 3x larger than other widgets.
* **Auto-Scale (Disabled)**: Widgets maintain uniform dot sizes globally for a pixel-perfect, consistent grid aesthetic. Adjust the size with a **Manual Scale (%)** slider.

### Base Font Metrics Configuration
Fine-tune the character style:
* **Digit Width**: 3 to 5 dot columns.
* **Dot Dimension**: Customizable dot width and dot height (1 to 5 pixels).
* **Gap Spacing**: Configurable vertical and horizontal gaps between dots.

### Colors & Themes
* **Light Theme**: Black dots on white background.
* **Dark Theme**: White dots on black background.
* **Custom Theme**: Customize every single color element including background, text, separators, battery states, weather values, and forecast chart gradients. (Refer to [COLORS.md](file:///home/markus/CLionProjects/Watchface-No.-2/COLORS.md) for Pebble's supported color palette).

---

## Engineered for Pebble Hardware (Efficiency)

Watchface No. 2 is designed to run smoothly on original Pebble hardware with strict limits:
- **Deferred Weather Persistence**: To avoid wearing down the watch's flash storage, weather forecast data is kept in RAM and only written to flash memory during app de-initialization or when the watchface closes.
- **Smart Data Caching**: Weather queries are cached locally to reduce API requests and phone-watch Bluetooth communications, saving battery on both devices.
- **Dynamic Config Tracking**: Checks the code version to avoid redundant settings synchronization requests on watch startup.

---

## Supported Platforms

Built natively for Pebble SDK 3 / 4, targeting:
* **basalt** (Pebble Time, Pebble Time Steel)
* **diorite** (Pebble 2, Pebble 2 SE)
* **flint** (Pebble Time Round)
* **emery** (Pebble Time 2 / Emulator)

---

## Development

The project uses a Python-based wrapper and Pebble SDK via a `Justfile` workflow.

### Setup
```shell
# Install dependencies
npm install

# Setup SDK & toolchain
just setup
```

### Build & Deploy
```shell
# Standard build (generates message keys and transpiles modern JS)
just build

# Clean and build
just clean && just build

# Deploy to emulator (e.g. Emery) with logs
just deploy debug emery
```

### Testing & Coverage
```shell
# Run all C and JS tests
just test

# Language-specific tests
just test-c
just test-js

# Run C coverage instrumentation
just coverage
```

---

## Attributions

* **Inspiration**: Heavily inspired by the [TXTTMM](https://apps.repebble.com/684087b4212f070009e2488a) watchface. Please support the creators of TXTTMM for their design.
* **Device Frame Generator**: Generated using [pebble-render](https://pebble-render.aru.dev/).

---

## License

MIT License - Copyright (c) 2017 Markus Ressel. See [LICENSE](file:///home/markus/CLionProjects/Watchface-No.-2/LICENSE) for full details.


