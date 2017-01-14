#include "pixel_matrix_drawer.h"

// default (fallback) matrix for unknown characters
static bool s_default_character[5][5] = {
  {true, true, true, true, true},
  {true, true, true, true, true},
  {true, true, true, true, true},
  {true, true, true, true, true},
  {true, true, true, true, true}
};

static int get_array_element_count(bool a[]) {
  if (!a) {
    return 0;
  }
  
  return sizeof(a) / sizeof(a[0]);
}

void* pixel_matrix_drawer_get_matrix(char c) {
  switch (c) {
    case '0':
    
      //break;
    case '1':
    
      break;
    case '2':
    
      //break;
    case '3':
    
      //break;
    case '4':
    
      //break;
    case '5':
    
      //break;
    case '6':
    
      //break;
    case '7':
    
      //break;
    case '8':
    
     //break;
    case '9':
    
      //break;
    default:
      
    
    return *s_default_character;
  }
  
  return *s_default_character;
}

/*
static bool* create_pixel_matrix(bool r0c0, bool r0c1, bool r0c2, bool r0c3, bool r0c4, 
                         bool r1c0, bool r1c1, bool r1c2, bool r1c3, bool r1c4,
                         bool r2c0, bool r2c1, bool r2c2, bool r2c3, bool r2c4,
                         bool r3c0, bool r3c1, bool r3c2, bool r3c3, bool r3c4,
                         bool r4c0, bool r4c1, bool r4c2, bool r4c3, bool r4c4) {
  bool[5][5] matrix = {
    {r0c0, r0c1, r0c2, r0c3, r0c4},
    {r1c0, r1c1, r1c2, r1c3, r1c4},
    {r2c0, r2c1, r2c2, r2c3, r2c4},
    {r3c0, r3c1, r3c2, r3c3, r3c4},
    {r4c0, r4c1, r4c2, r4c3, r4c4}
  };
  return matrix;
}
*/

void pixel_matrix_drawer_draw_matrix(GContext* ctx, GPoint point_zero, bool matrix[][5], int scale_factor) {
  if (!matrix) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "matrix is NULL!");
    return;
  }
  
  // Should be implicit through method parameter definition
  /*
  if (get_array_element_count(matrix) != 5)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "invalid row count: %d", get_array_element_count(matrix));
    return;
  }
  
  for(int row = 0; row < ; row++) {
    if (get_array_element_count(matrix[row]) != 5) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "invalid column count in row %d: %d", get_array_element_count(matrix[row]));
    }
  }
  */
  
  for(int row = 0; row < 5; row++) {
    for(int column = 0; column < 5; column++) {
      // get matrix value at current loop position
      bool draw = matrix[row][column];
      
      // skip pixel if not defined in matrix
      if (!matrix[row][column]) {
        continue;
      }
      
      // draw pixel
      GRect dot_bounds = GRect(
        point_zero.x + column * scale_factor * 2,
        point_zero.y + row * scale_factor * 2,
        scale_factor,
        scale_factor);
      
      graphics_fill_rect(ctx, dot_bounds, 0, GCornerNone);
    }
  }
}