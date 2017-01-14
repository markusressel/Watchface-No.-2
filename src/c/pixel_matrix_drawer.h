#pragma once
#include <pebble.h>

// Draws a character onto the canvas at the specified position
//
// @param ctx           Pebble graphics context
// @param point_zero    start coordinate, where matrix[0][0] will be drawn
// @param character     The character to draw
// @param scale_factor  Factor to multiply array with, drawn pixels and positioning will be scaled
// @return  width of the drawn matrix (for width in pixel this has to be multiplied by scale_factor)
int pixel_matrix_drawer_draw_char(GContext* ctx, GPoint point_zero, char character, int scale_factor);