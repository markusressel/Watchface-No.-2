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

static bool s_character_comma[5][5] = {
  {false, false, false, false, false},
  {false, false, false, false, false},
  {false, false, false, false, false},
  {true, false, false, false, false},
  {true, false, false, false, false}
};

static bool s_number_0[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, true},
  {true, false, false, false, true},
  {true, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_0_size_4[5][5] = {
  {true, true, true, true, false},
  {true, false, false, true, false},
  {true, false, false, true, false},
  {true, false, false, true, false},
  {true, true, true, true, false}
};

static bool s_number_0_size_3[5][5] = {
  {true, true, true, false, false},
  {true, false, true, false, false},
  {true, false, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false}
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

static bool s_number_2_size_4[5][5] = {
  {true, true, true, true, false},
  {false, false, false, true, false},
  {true, true, true, true, false},
  {true, false, false, false, false},
  {true, true, true, true, false}
};

static bool s_number_2_size_3[5][5] = {
  {true, true, true, false, false},
  {false, false, true, false, false},
  {true, true, true, false, false},
  {true, false, false, false, false},
  {true, true, true, false, false}
};

static bool s_number_3[5][5] = {
  {true, true, true, true, true},
  {false, false, false, false, true},
  {false, true, true, true, true},
  {false, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_3_size_4[5][5] = {
  {true, true, true, true, false},
  {false, false, false, true, false},
  {false, true, true, true, false},
  {false, false, false, true, false},
  {true, true, true, true, false}
};

static bool s_number_3_size_3[5][5] = {
  {true, true, true, false, false},
  {false, false, true, false, false},
  {false, true, true, false, false},
  {false, false, true, false, false},
  {true, true, true, false, false}
};

static bool s_number_4[5][5] = {
  {true, false, false, false, true},
  {true, false, false, false, true},
  {true, true, true, true, true},
  {false, false, false, false, true},
  {false, false, false, false, true}
};

static bool s_number_4_size_4[5][5] = {
  {true, false, false, true, false},
  {true, false, false, true, false},
  {true, true, true, true, false},
  {false, false, false, true, false},
  {false, false, false, true, false}
};

static bool s_number_4_size_3[5][5] = {
  {true, false, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false},
  {false, false, true, false, false},
  {false, false, true, false, false}
};

static bool s_number_5[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, false},
  {true, true, true, true, true},
  {false, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_5_size_4[5][5] = {
  {true, true, true, true, false},
  {true, false, false, false, false},
  {true, true, true, true, false},
  {false, false, false, true, false},
  {true, true, true, true, false}
};

static bool s_number_5_size_3[5][5] = {
  {true, true, true, false, false},
  {true, false, false, false, false},
  {true, true, true, false, false},
  {false, false, true, false, false},
  {true, true, true, false, false}
};

static bool s_number_6[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, false},
  {true, true, true, true, true},
  {true, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_6_size_4[5][5] = {
  {true, true, true, true, false},
  {true, false, false, false, false},
  {true, true, true, true, false},
  {true, false, false, true, false},
  {true, true, true, true, false}
};

static bool s_number_6_size_3[5][5] = {
  {true, true, true, false, false},
  {true, false, false, false, false},
  {true, true, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false}
};

static bool s_number_7[5][5] = {
  {true, true, true, true, true},
  {false,false,false, false, true},
  {false, false, false, false, true},
  {false, false, false, false, true},
  {false, false, false, false, true}
};

static bool s_number_7_size_4[5][5] = {
  {true, true, true, true, false},
  {false,false,false, true, false},
  {false, false, false, true, false},
  {false, false, false, true, false},
  {false, false, false, true, false}
};

static bool s_number_7_size_3[5][5] = {
  {true, true, true, false, false},
  {false,false, true, false, false},
  {false, false, true, false, false},
  {false, false, true, false, false},
  {false, false, true, false, false}
};

static bool s_number_8[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, true},
  {true, true, true, true, true},
  {true, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_8_size_4[5][5] = {
  {true, true, true, true, false},
  {true, false, false, true, false},
  {true, true, true, true, false},
  {true, false, false, true, false},
  {true, true, true, true, false}
};

static bool s_number_8_size_3[5][5] = {
  {true, true, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false}
};

static bool s_number_9[5][5] = {
  {true, true, true, true, true},
  {true, false, false, false, true},
  {true, true, true, true, true},
  {false, false, false, false, true},
  {true, true, true, true, true}
};

static bool s_number_9_size_4[5][5] = {
  {true, true, true, true, false},
  {true, false, false, true, false},
  {true, true, true, true, false},
  {false, false, false, true, false},
  {true, true, true, true, false}
};

static bool s_number_9_size_3[5][5] = {
  {true, true, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false},
  {false, false, true, false, false},
  {true, true, true, false, false}
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

static bool s_character_space[5][5] = {
  {false, false, false, false, false},
  {false, false, false, false, false},
  {false, false, false, false, false},
  {false, false, false, false, false},
  {false, false, false, false, false}
};

static bool s_character_M[5][5] = {
  {true, false, true, false, false},
  {true, true, true, false, false},
  {true, false, true, false, false},
  {true, false, true, false, false},
  {true, false, true, false, false}
};

static bool s_character_O[5][5] = {
  {true, true, true, false, false},
  {true, false, true, false, false},
  {true, false, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false}
};

static bool s_character_o[5][5] = {
  {false, false, false, false, false},
  {false, false, false, false, false},
  {true, true, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false}
};

static bool s_character_I[5][5] = {
  {true, false, false, false, false},
  {true, false, false, false, false},
  {true, false, false, false, false},
  {true, false, false, false, false},
  {true, false, false, false, false}
};

static bool s_character_i[5][5] = {
  {false, false, false, false, false},
  {true, false, false, false, false},
  {false, false, false, false, false},
  {true, false, false, false, false},
  {true, false, false, false, false}
};

static bool s_character_D[5][5] = {
  {true, true, false, false, false},
  {true, false, true, false, false},
  {true, false, true, false, false},
  {true, false, true, false, false},
  {true, true, false, false, false}
};

static bool s_character_T[5][5] = {
  {true, true, true, false, false},
  {false, true, false, false, false},
  {false, true, false, false, false},
  {false, true, false, false, false},
  {false, true, false, false, false}
};

static bool s_character_H[5][5] = {
  {true, false, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false},
  {true, false, true, false, false},
  {true, false, true, false, false}
};

static bool s_character_h[5][5] = {
  {true, false, false, false, false},
  {true, false, false, false, false},
  {true, true, true, false, false},
  {true, false, true, false, false},
  {true, false, true, false, false}
};

static bool s_character_F[5][5] = {
  {true, true, true, false, false},
  {true, false, false, false, false},
  {true, true, true, false, false},
  {true, false, false, false, false},
  {true, false, false, false, false}
};

static bool s_character_R[5][5] = {
  {true, true, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false},
  {true, true, false, false, false},
  {true, false, true, false, false}
};

static bool s_character_r[5][5] = {
  {false, false, true, false, false},
  {false, false, false, false, false},
  {true, true, false, false, false},
  {true, false, false, false, false},
  {true, false, false, false, false}
};

static bool s_character_S[5][5] = {
  {true, true, true, false, false},
  {true, false, false, false, false},
  {true, true, true, false, false},
  {false, false, true, false, false},
  {true, true, true, false, false}
};

static bool s_character_A[5][5] = {
  {true, true, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false},
  {true, false, true, false, false},
  {true, false, true, false, false}
};

static bool s_character_a[5][5] = {
  {false, false, false, false, false},
  {false, false, false, false, false},
  {false, true, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false}
};

static bool s_character_U[5][5] = {
  {true, false, true, false, false},
  {true, false, true, false, false},
  {true, false, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false}
};

static bool s_character_u[5][5] = {
  {false, false, false, false, false},
  {false, false, false, false, false},
  {true, false, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false}
};

static bool s_character_W[5][5] = {
  {true, false, true, false, false},
  {true, false, true, false, false},
  {true, false, true, false, false},
  {true, true, true, false, false},
  {true, false, true, false, false}
};

static bool s_character_E[5][5] = {
  {true, true, true, false, false},
  {true, false, false, false, false},
  {true, true, true, false, false},
  {true, false, false, false, false},
  {true, true, true, false, false}
};

static bool s_character_e[5][5] = {
  {false, false, false, false, false},
  {true, true, true, false, false},
  {true, true, true, false, false},
  {true, false, false, false, false},
  {true, true, true, false, false}
};

int pixel_matrix_drawer_draw_char(
  GContext* ctx, 
  GPoint point_zero, 
  char character, 
  int dot_width, int dot_height, 
  int gap_size_horizontal, int gap_size_vertical,
  bool align_right,
  int digit_size) {
  if (!character) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "character is NULL!");
    return 0;
  }
  
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "digit_size: %d", digit_size);
  
  // find matrix for character
  bool (*character_pixel_matrix)[5];
  int row_count = 5;
  int column_count = digit_size;
  
  switch (character) {
    case '.':
      character_pixel_matrix = s_character_dot;
      column_count = 1;
      break;
    case ',':
      character_pixel_matrix = s_character_comma;
      column_count = 1;
      break;
    case '0':
      if (digit_size == 5) {
        character_pixel_matrix = s_number_0;
      } else if (digit_size == 4) {
        character_pixel_matrix = s_number_0_size_4;
      } else {
        character_pixel_matrix = s_number_0_size_3;
      }
      
      break;
    case '1':
      character_pixel_matrix = s_number_1;
      column_count = 2;
      break;
    case '2':
      if (digit_size == 5) {
        character_pixel_matrix = s_number_2;
      } else if (digit_size == 4) {
        character_pixel_matrix = s_number_2_size_4;
      } else {
        character_pixel_matrix = s_number_2_size_3;
      }
      
      break;
    case '3':
      if (digit_size == 5) {
        character_pixel_matrix = s_number_3;
      } else if (digit_size == 4) {
        character_pixel_matrix = s_number_3_size_4;
      } else {
        character_pixel_matrix = s_number_3_size_3;
      }
      
      break;
    case '4':
      if (digit_size == 5) {
        character_pixel_matrix = s_number_4;
      } else if (digit_size == 4) {
        character_pixel_matrix = s_number_4_size_4;
      } else {
        character_pixel_matrix = s_number_4_size_3;
      }
      
      break;
    case '5':
      if (digit_size == 5) {
        character_pixel_matrix = s_number_5;
      } else if (digit_size == 4) {
        character_pixel_matrix = s_number_5_size_4;
      } else {
        character_pixel_matrix = s_number_5_size_3;
      }
     
      break;
    case '6':
      if (digit_size == 5) {
        character_pixel_matrix = s_number_6;
      } else if (digit_size == 4) {
        character_pixel_matrix = s_number_6_size_4;
      } else {
        character_pixel_matrix = s_number_6_size_3;
      }
      
      break;
    case '7':
      if (digit_size == 5) {
        character_pixel_matrix = s_number_7;
      } else if (digit_size == 4) {
        character_pixel_matrix = s_number_7_size_4;
      } else {
        character_pixel_matrix = s_number_7_size_3;
      }
      
      break;
    case '8':
      if (digit_size == 5) {
        character_pixel_matrix = s_number_8;
      } else if (digit_size == 4) {
        character_pixel_matrix = s_number_8_size_4;
      } else {
        character_pixel_matrix = s_number_8_size_3;
      }
      
      break;
    case '9':
      if (digit_size == 5) {
        character_pixel_matrix = s_number_9;
      } else if (digit_size == 4) {
        character_pixel_matrix = s_number_9_size_4;
      } else {
        character_pixel_matrix = s_number_9_size_3;
      }
      
      break;
    case 'C':
      character_pixel_matrix = s_character_c;
      break;
    case 'M':
      character_pixel_matrix = s_character_M;
      column_count = 3;
      break;
    case 'O':
      character_pixel_matrix = s_character_O;
      column_count = 3;
      break;
    case 'o':
      character_pixel_matrix = s_character_o;
      column_count = 3;
      break;
    case 'I':
      character_pixel_matrix = s_character_I;
      column_count = 3;
      break;
    case 'i':
      character_pixel_matrix = s_character_i;
      column_count = 1;
      break;
    case 'D':
      character_pixel_matrix = s_character_D;
      column_count = 3;
      break;
    case 'F':
      character_pixel_matrix = s_character_F;
      column_count = 3;
      break;
    case 'R':
      character_pixel_matrix = s_character_R;
      column_count = 3;
      break;
    case 'r':
      character_pixel_matrix = s_character_r;
      column_count = 2;
      break;
    case 'S':
      character_pixel_matrix = s_character_S;
      column_count = 3;
      break;
    case 'A':
      character_pixel_matrix = s_character_A;
      column_count = 3;
      break;
    case 'a':
      character_pixel_matrix = s_character_a;
      column_count = 3;
      break;
    case 'U':
      character_pixel_matrix = s_character_U;
      column_count = 3;
      break;
    case 'u':
      character_pixel_matrix = s_character_u;
      column_count = 3;
      break;
    case 'T':
      character_pixel_matrix = s_character_T;
      column_count = 3;
      break;
    case 'H':
      character_pixel_matrix = s_character_H;
      column_count = 3;
      break;
    case 'h':
      character_pixel_matrix = s_character_h;
      column_count = 3;
      break;
     case 'W':
      character_pixel_matrix = s_character_W;
      column_count = 3;
      break;
    case 'E':
      character_pixel_matrix = s_character_E;
      column_count = 3;
      break;
    case 'e':
      character_pixel_matrix = s_character_e;
      column_count = 3;
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
    case ' ':
      character_pixel_matrix = s_character_space;
      column_count = 1;
      break;
    default:
      character_pixel_matrix = s_default_character;
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