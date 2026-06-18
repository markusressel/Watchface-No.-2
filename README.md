# Watchface No. 2

A dot-styled watchface for the Pebble platform

| Time 2                                                               |
|----------------------------------------------------------------------|
| <img src="/screenshots/framed_time_2_custom_scaled.png" width="250"> |

| Core 2 Duo                                                                                                                 |
|----------------------------------------------------------------------------------------------------------------------------|
| <img src="/screenshots/framed_diorite_dark.png" width="204"> <img src="/screenshots/framed_diorite_light.png" width="204"> |

# Components

The components of the watchface are laid out in rows:

| Row | Component    | Default                             |
|-----|--------------|-------------------------------------|
| 1   | Configurable | Weather                             |
| 2   | Configurable | Date                                |
| 3   | Time         | Time w/o seconds                    |
| 4   | Configurable | Step count for the day              |
| 5   | Configurable | Battery bar with charging animation |

To change which component is in which row, use the configuration page in the Pebble app.

These are the components to choose from:

* Weather
    * Maximum Temp for the day | Minimum temp for the day
* Date
    * day of the month . month number
* Step count for the day
* Heart rate
* Battery bar
    * with charging animation

# Theme

Colors can be changed for each component individually through the configuration page in the Pebble app.
After saving your modified settings, the watchface will reload automatically.

[Color Table](COLORS.md) for reference.

# Settings

<img src="/screenshots/Settings1.png" width="300"><img src="/screenshots/Settings2.png" width="306">

# Development

## Testing

You can run both C and JS tests using the provided `Justfile`:

```shell
# Run all tests
just test

# Run only C tests
just test-c

# Run only JS tests
just test-js
```

## Code Coverage (C)

To check the code coverage of the C implementation, run:

```shell
just coverage
```

This will run all C tests with coverage instrumentation and provide a summary in the terminal.

**Recommendation:** Install `gcovr` (e.g., via `pip install gcovr`) to get a more detailed summary and an HTML report (`coverage.html`). If `gcovr` is not installed, it will fall
back to the basic `gcov` tool.

# Behind the scenes

The "dotted font" in this watchface actually isn't a font. Each character - `0-9`, some special characters like `.` and `/` and selected letters for displaying weekdays - is
matched with 5x5 bool matrix that specifies which pixel should be drawn. Between these pixels gaps are interpolated to create the pattern effect.

If you want to take a deeper look at how I've done this check out the [dotted_text_layer.c](src/c/ui/layer/dotted_text_layer.c)
and [dotted_text_layer.h](src/c/ui/layer/dotted_text_layer.h). This is
a custom implementation of a standard Pebble Layer and internally uses the [pixel_matrix_drawer.h](src/c/ui/graphics/pixel_matrix_drawer.h).

# Attributions

## Inspiration

This watchface is heavily in inspired by the [TXTTMM](https://apps.repebble.com/684087b4212f070009e2488a) watchface. If you like this watchface please consider supporting
the [TXTTMM](https://apps.repebble.com/684087b4212f070009e2488a) watchface makers for their great work by buing their watchface (just a dollar).

## Device Frame Generator

See: https://pebble-render.aru.dev/

# License

    MIT License
    
    Copyright (c) 2017 Markus Ressel
    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

