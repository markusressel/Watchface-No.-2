# Watchface No. 2
A dot styled watchface for the Pebble platform

<img src="/framed_light.png" width="">

# Theme
Colors can be changed for each component individually through the build in Settings screen, accessible in the Pebble app.
After saving your modified settings, the watchface needs to be restarted to reload the changed settings.
To do this you can open the Pebble main menu and close it right after. The watchface will then have to reinitialize and load all settings from scratch.

Here are the two main themes and a variation for the custom variant:

<img src="/framed_light.png" width=""><img src="/framed_dark.png" width=""><img src="/framed_color.png" width="">

# Behind the scenes
The "dotted font" in this watchface actually isn't a font. Each character (0-9.|/o) is matched with 5x5 bool matrix that specifies which pixel should be drawn. Between these pixels gaps are interpolated to create the pattern effect.

If you want to take a deeper look at how I've done this check out the ```dotted_text_layer.c``` and ```dotted_text_layer.h```. This is a custom implementation of a standard Pebble Layer and internally uses the ```pixel_matrix_drawer.h``` whichs implementation can (obiously) be found in ```pixel_matrix_drawer.c```.

# Attributions

This watchface is heavily in inspired by the [TXTTMM](https://apps.getpebble.com/en_US/application/57f8d77298b2fc725d00006a) watchface. If you like this watchface please consider supporting the [TXTTMM](https://apps.getpebble.com/en_US/application/57f8d77298b2fc725d00006a) watchface makers for their great work by buing their watchface (just a dollar).
