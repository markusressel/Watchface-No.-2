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

static bool s_number_0_small[5][5] = {
  {true, true, true, true, false},
  {true, false, false, true, false},
  {true, false, false, true, false},
  {true, false, false, true, false},
  {true, true, true, true, false}
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

static bool s_number_2_small[5][5] = {
  {true, true, true, true, false},
  {false, false, false, true, false},
  {true, true, true, true, false},
  {true, false, false, false, false},
  {true, true, true, true, false}
};

static bool s_number_3[5][5] = {
  {true, true, true, true, true},
  {false, false, false, false, true},
  {false, true, true, true, true},
  {false, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_3_small[5][5] = {
  {true, true, true, true, false},
  {false, false, false, true, false},
  {false, true, true, true, false},
  {false, false, false, true, false},
  {true, true, true, true, false}
};

static bool s_number_4[5][5] = {
  {true, false, false, false, true},
  {true, false, false, false, true},
  {true, true, true, true, true},
  {false, false, false, false, true},
  {false, false, false, false, true}
};

static bool s_number_4_small[5][5] = {
  {true, false, false, true, false},
  {true, false, false, true, false},
  {true, true, true, true, false},
  {false, false, false, true, false},
  {false, false, false, true, false}
};

static bool s_number_5[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, false},
  {true, true, true, true, true},
  {false, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_5_small[5][5] = {
  {true, true, true, true, false},
  {true, false, false, false, false},
  {true, true, true, true, false},
  {false, false, false, true, false},
  {true, true, true, true, false}
};

static bool s_number_6[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, false},
  {true, true, true, true, true},
  {true, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_6_small[5][5] = {
  {true, true, true, true, false},
  {true, false, false, false, false},
  {true, true, true, true, false},
  {true, false, false, true, false},
  {true, true, true, true, false}
};

static bool s_number_7[5][5] = {
  {true, true, true, true, true},
  {false,false,false, false, true},
  {false, false, false, false, true},
  {false, false, false, false, true},
  {false, false, false, false, true}
};

static bool s_number_7_small[5][5] = {
  {true, true, true, true, false},
  {false,false,false, true, false},
  {false, false, false, true, false},
  {false, false, false, true, false},
  {false, false, false, true, false}
};

static bool s_number_8[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, true},
  {true, true, true, true, true},
  {true, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_8_small[5][5] = {
  {true, true, true, true, false},
  {true, false, false, true, false},
  {true, true, true, true, false},
  {true, false, false, true, false},
  {true, true, true, true, false}
};

static bool s_number_9[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, true},
  {true, true, true, true, true},
  {false, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_9_small[5][5] = {
  {true, true, true, true, false},
  {true, false, false, true, false},
  {true, true, true, true, false},
  {false, false, false, true, false},
  {true, true, true, true, false}
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

static bool s_character_pipe[5][5] = {
  {false, true, false, false, false},
  {false, true, false, false, false},
  {false, true, false, false, false},
  {false, true, false, false, false},
  {false, true, false, false, false}
};

static bool s_character_minus[5][5] = {
  {false, false, false, false, false},
  {false, false, false, false, false},
  {true, true, true, true, true},
  {false, false, false, false, false},
  {false, false, false, false, false}
};

int pixel_matrix_drawer_draw_char(
  GContext* ctx, 
  GPoint point_zero, 
  char character, 
  int dot_width, int dot_height, 
  int gap_size_horizontal, int gap_size_vertical,
  bool align_right,
  bool four_dots_width) {
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
      if (!four_dots_width) {
        character_pixel_matrix = s_number_0;
        column_count = 5;
      } else {
        character_pixel_matrix = s_number_0_small;
        column_count = 4;
      }
      
      break;
    case '1':
      character_pixel_matrix = s_number_1;
      column_count = 2;
      break;
    case '2':
      if (!four_dots_width) {
        character_pixel_matrix = s_number_2;
        column_count = 5;
      } else {
        character_pixel_matrix = s_number_2_small;
        column_count = 4;
      }
      
      break;
    case '3':
      if (!four_dots_width) {
        character_pixel_matrix = s_number_3;
        column_count = 5;
      } else {
        character_pixel_matrix = s_number_3_small;
        column_count = 4;
      }
      
      break;
    case '4':
      if (!four_dots_width) {
        character_pixel_matrix = s_number_4;
        column_count = 5;
      } else {
        character_pixel_matrix = s_number_4_small;
        column_count = 4;
      }
      
      break;
    case '5':
      if (!four_dots_width) {
        character_pixel_matrix = s_number_5;
        column_count = 5;
      } else {
        character_pixel_matrix = s_number_5_small;
        column_count = 4;
      }
     
      break;
    case '6':
      if (!four_dots_width) {
        character_pixel_matrix = s_number_6;
        column_count = 5;
      } else {
        character_pixel_matrix = s_number_6_small;
        column_count = 4;
      }
      
      break;
    case '7':
      if (!four_dots_width) {
        character_pixel_matrix = s_number_7;
        column_count = 5;
      } else {
        character_pixel_matrix = s_number_7_small;
        column_count = 5;
      }
      
      break;
    case '8':
      if (!four_dots_width) {
        character_pixel_matrix = s_number_8;
        column_count = 5;
      } else {
        character_pixel_matrix = s_number_8_small;
        column_count = 4;
      }
      
      break;
    case '9':
      if (!four_dots_width) {
        character_pixel_matrix = s_number_9;
        column_count = 5;
      } else {
        character_pixel_matrix = s_number_9_small;
        column_count = 4;
      }
      
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
    case '|':
      character_pixel_matrix = s_character_pipe;
      column_count = 3;
      break;
    case '-':
      character_pixel_matrix = s_character_minus;
      column_count = 2;
      break;
    default:
      character_pixel_matrix = s_default_character;
      if (!four_dots_width) {
        column_count = 5;
      } else {
        column_count = 4;
      }
      
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
      
      int x;
      int y;
      
      if (align_right) {
        x = point_zero.x + column * dot_width + column * gap_size_horizontal - (column_count * dot_width + (column_count - 1) * gap_size_horizontal);
        y = point_zero.y + row * dot_height + row * gap_size_vertical;
      } else {
        x = point_zero.x + column * dot_width + column * gap_size_horizontal;
        y = point_zero.y + row * dot_height + row * gap_size_vertical;
      }
      
      // draw pixel
      GRect dot_bounds = GRect(
        x,
        y,
        dot_width,
        dot_height);
      
      graphics_fill_rect(ctx, dot_bounds, 0, GCornerNone);
    }
  }
  
  return column_count;
}