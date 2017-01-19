# Watchface No. 2
A dot styled watchface for the Pebble platform

<img src="/framed_light.png" width=""><img src="/framed_dark_small.png" width="">

# Components
Currently the components of this watchface can not be changed in order without coding and recompiling.
These are the current components in their top to bottom order:

* Weather
  * Maximum Temp for the day | Minimum temp for the day
* Date
  * day of the month . month number
* Time
  * hours:minutes(:seconds)
* Step count for the day
* Battery bar
  * with charging animation


# Theme
Colors can be changed for each component individually through the build in Settings screen, accessible in the Pebble app.
After saving your modified settings, the watchface needs to be restarted to reload the changed settings.
To do this you can open the Pebble main menu and close it right after. The watchface will then have to reinitialize and load all settings from scratch.

Here are the two main themes and a variation for the custom variant:

<img src="/framed_light.png" width=""><img src="/framed_dark.png" width=""><img src="/framed_color.png" width="">

# Settings

<img src="/Settings1.png" width="300"><img src="/Settings2.png" width="306">

# Behind the scenes
The "dotted font" in this watchface actually isn't a font. Each character (0-9.|/o) is matched with 5x5 bool matrix that specifies which pixel should be drawn. Between these pixels gaps are interpolated to create the pattern effect.

If you want to take a deeper look at how I've done this check out the ```dotted_text_layer.c``` and ```dotted_text_layer.h```. This is a custom implementation of a standard Pebble Layer and internally uses the ```pixel_matrix_drawer.h``` whichs implementation can (obiously) be found in ```pixel_matrix_drawer.c```.

# Attributions

This watchface is heavily in inspired by the [TXTTMM](https://apps.getpebble.com/en_US/application/57f8d77298b2fc725d00006a) watchface. If you like this watchface please consider supporting the [TXTTMM](https://apps.getpebble.com/en_US/application/57f8d77298b2fc725d00006a) watchface makers for their great work by buing their watchface (just a dollar).

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

