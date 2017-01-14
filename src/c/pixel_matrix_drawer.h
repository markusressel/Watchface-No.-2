#pragma once
#include <pebble.h>

// Returns a matrix for a character
// The matrix will filled completely, when no specific version could be found
//
// @param c  character to find matrix for
// @return   5x5 bool matrix
void* pixel_matrix_drawer_get_matrix(char c);

// Draws a 5x5 Matrix onto the canvas at the specified position
//
// @param ctx           Pebble graphics context
// @param point_zero    start coordinate, where matrix[0][0] will be drawn
// @param matrix        A 5x5 matrix specifying which pixels to draw
// @param scale_factor  Factor to multiply array with, drawn pixels and positioning will be scaled
void pixel_matrix_drawer_draw_matrix(GContext* ctx, GPoint point_zero, bool matrix[][5], int scale_factor);