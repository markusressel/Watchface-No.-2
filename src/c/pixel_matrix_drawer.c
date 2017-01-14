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
  {true, false, false, false, false}
};

static bool s_number_0[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, true},
  {true, false, false, false, true},
  {true, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_1[5][5] = {
  {true, true, false, false, false},
  {false, true, false, false, false},
  {false, true, false, false, false},
  {false, true, false, false, false},
  {false, true, false, false, false}
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
  {false,false,false, false, true},
  {false, false, false, false, true},
  {false, false, false, false, true},
  {false, false, false, false, true}
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

static bool s_character_degree[5][5] = {
  {true, true, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false},
  {false, false, false, false, false},
  {false, false, false, false, false}
};

static bool s_character_c[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, false},
  {true, false, false, false, false},
  {true, false, false, false, false},
  {true, true, true, true, true}
};

static bool s_character_slash[5][5] = {
  {false, false, true, false, false},
  {false, true, false, false, false},
  {false, true, false, false, false},
  {true, false, false, false, false},
  {true, false, false, false, false}
};

static bool s_character_minus[5][5] = {
  {false, false, false, false, false},
  {false, false, false, false, false},
  {true, true, true, false, false},
  {false, false, false, false, false},
  {false, false, false, false, false}
};

int pixel_matrix_drawer_draw_char(GContext* ctx, GPoint point_zero, char character, int dot_width, int dot_height) {
  if (!character) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "character is NULL!");
    return 0;
  }
  
  // find matrix for character
  bool (*character_pixel_matrix)[5];
  int row_count = 5;
  int column_count;
  
  switch (character) {
    case '.':
      character_pixel_matrix = s_character_dot;
      column_count = 1;
      break;
    case '0':
      character_pixel_matrix = s_number_0;
      column_count = 5;
      break;
    case '1':
      character_pixel_matrix = s_number_1;
      column_count = 2;
      break;
    case '2':
      character_pixel_matrix = s_number_2;
      column_count = 5;
      break;
    case '3':
      character_pixel_matrix = s_number_3;
      column_count = 5;
      break;
    case '4':
      character_pixel_matrix = s_number_4;
      column_count = 5;
      break;
    case '5':
      character_pixel_matrix = s_number_5;
      column_count = 5;
      break;
    case '6':
      character_pixel_matrix = s_number_6;
      column_count = 5;
      break;
    case '7':
      character_pixel_matrix = s_number_7;
      column_count = 5;
      break;
    case '8':
      character_pixel_matrix = s_number_8;
      column_count = 5;
      break;
    case '9':
      character_pixel_matrix = s_number_9;
      column_count = 5;
      break;
    case 'o':
      character_pixel_matrix = s_character_degree;
      column_count = 3;
      break;
    case 'C':
      character_pixel_matrix = s_character_c;
      column_count = 5;
      break;
    case '/':
      character_pixel_matrix = s_character_slash;
      column_count = 3;
      break;
    case '-':
      character_pixel_matrix = s_character_minus;
      column_count = 3;
      break;
    default:
      character_pixel_matrix = s_default_character;
      column_count = 5;
      break;
  }
  
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "matrix assigned");
  
  for(int row = 0; row < row_count; row++) {
    for(int column = 0; column < column_count; column++) {
      // get matrix value at current loop position
      // bool draw = character_pixel_matrix[(row + column) * sizeof(bool)];
      
      // skip pixel if not defined in matrix
      if (!character_pixel_matrix[row][column]) {
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "skipping pixel");
        
        continue;
      }
      
      // APP_LOG(APP_LOG_LEVEL_DEBUG, "drawing pixel");
      
      // draw pixel
      GRect dot_bounds = GRect(
        point_zero.x + column * dot_width * 2,
        point_zero.y + row * dot_height * 2,
        dot_width,
        dot_height);
      
      graphics_fill_rect(ctx, dot_bounds, 0, GCornerNone);
    }
  }
  
  return column_count;
}