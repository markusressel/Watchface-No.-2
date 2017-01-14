#pragma once
#include <pebble.h>

// Draws a character onto the canvas at the specified position
//
// @param ctx                  Pebble graphics context
// @param point_zero           start coordinate, where matrix[0][0] will be drawn
// @param character            The character to draw
// @param dot_width            The width of a single dot
// @param dot_height           The height of a single dot
// @param gap_size_horizontal  The size of horizontally empty space between two dots
// @param gap_size_vertical    The size of vertically empty space between two dots
// @param align_right          Draws the characters from right to left instead of left to right
//
// @return  width of the drawn matrix (for width in pixel this has to be multiplied by scale_factor)
int pixel_matrix_drawer_draw_char(
  GContext* ctx, 
  GPoint point_zero, 
  char character, 
  int dot_wigth, int dot_height, 
  int gap_size_horizontal, int gap_size_vertical,
  bool align_right);