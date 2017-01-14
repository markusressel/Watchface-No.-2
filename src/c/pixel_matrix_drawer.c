#include "pixel_matrix_drawer.h"

// default (fallback) matrix for unknown characters
static bool s_default_character[5][5] = {
  {true, true, true, true, true},
  {true, true, true, true, true},
  {true, true, true, true, true},
  {true, true, true, true, true},
  {true, true, true, true, true}
};

static bool s_character_dot[5][5] = {
  {false, false, false, false, false},
  {false, false, false, false, false},
  {false, false, false, false, false},
  {false, false, false, false, false},
  {false, false, true, false, false}
};

static bool s_number_0[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, true},
  {true, false, false, false, true},
  {true, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_1[5][5] = {
  {false, false, false, true, true},
  {false, false, false, false, true},
  {false, false, false, false, true},
  {false, false, false, false, true},
  {false, false, false, false, true}
};

static bool s_number_2[5][5] = {
  {true, true, true, true, true},
  {false, false, false, false, true},
  {true, true, true, true, true},
  {true, false, false, false, false},
  {true, true, true, true, true}
};

static bool s_number_3[5][5] = {
  {true, true, true, true, true},
  {false, false, false, false, true},
  {false, true, true, true, true},
  {false, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_4[5][5] = {
  {true, false, false, false, true},
  {true, false, false, false, true},
  {true, true, true, true, true},
  {false, false, false, false, true},
  {false, false, false, false, true}
};

static bool s_number_5[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, false},
  {true, true, true, true, true},
  {false, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_6[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, false},
  {true, true, true, true, true},
  {true, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_7[5][5] = {
  {true, true, true, true, true},
  {false,false,false, true, false},
  {false, false, true, false, false},
  {false, true, false, false, false},
  {true, false, false, false, false}
};

static bool s_number_8[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, true},
  {true, true, true, true, true},
  {true, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_9[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, true},
  {true, true, true, true, true},
  {false, false, false, false, true},
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
    case '.':
      return *s_character_dot;
    case '0':
      return *s_number_0;
    case '1':
      return *s_number_1;
    case '2':
      return *s_number_2;
    case '3':
      return *s_number_3;
    case '4':
      return *s_number_4;
    case '5':
      return *s_number_5;
    case '6':
      return *s_number_6;
    case '7':
      return *s_number_7;
    case '8':
      return *s_number_8;
    case '9':
      return *s_number_9;
    default:
      return *s_default_character;
    break;
  }
  
  return *s_default_character;
}

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