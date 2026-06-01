#include "pixel_matrix_drawer.h"

// Visual macros to "draw" the pixels in the code
#define X true
#define _ false

// Struct to bundle a character's matrix and its width together
typedef struct {
    int width;
    bool pixels[5][5];
} Glyph;

// Default fallback character (Full 5x5 block)
static const Glyph s_default = {
    .width = 5, .pixels = {
        {X, X, X, X, X},
        {X, X, X, X, X},
        {X, X, X, X, X},
        {X, X, X, X, X},
        {X, X, X, X, X}
    }
};
// 1. All standard characters in a single lookup table
static const Glyph s_chars[128] = {
    ['.'] = {
        .width = 1, .pixels = {
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, _, _, _, _},
            {X, _, _, _, _}
        }
    },

    [','] = {
        .width = 1, .pixels = {
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, _, _, _, _},
            {X, _, _, _, _},
            {X, _, _, _, _}
        }
    },

    ['|'] = {
        .width = 3, .pixels = {
            {_, X, _, _, _},
            {_, X, _, _, _},
            {_, X, _, _, _},
            {_, X, _, _, _},
            {_, X, _, _, _}
        }
    },

    ['-'] = {
        .width = 5, .pixels = {
            {_, _, _, _, _},
            {_, _, _, _, _},
            {X, X, X, X, X},
            {_, _, _, _, _},
            {_, _, _, _, _}
        }
    },

    [' '] = {
        .width = 1, .pixels = {
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, _, _, _, _}
        }
    },

    ['C'] = {
        .width = 5, .pixels = {
            {X, X, X, X, X},
            {X, _, _, _, _},
            {X, _, _, _, _},
            {X, _, _, _, _},
            {X, X, X, X, X}
        }
    },

    ['c'] = {
        .width = 5, .pixels = {
            {_, _, _, _, _},
            {_, _, _, _, _},
            {X, X, X, X, X},
            {X, _, _, _, _},
            {X, X, X, X, X}
        }
    },

    ['M'] = {
        .width = 3, .pixels = {
            {X, _, X, _, _},
            {X, X, X, _, _},
            {X, _, X, _, _},
            {X, _, X, _, _},
            {X, _, X, _, _}
        }
    },

    ['O'] = {
        .width = 3, .pixels = {
            {X, X, X, _, _},
            {X, _, X, _, _},
            {X, _, X, _, _},
            {X, _, X, _, _},
            {X, X, X, _, _}
        }
    },

    ['o'] = {
        .width = 3, .pixels = {
            {_, _, _, _, _},
            {_, _, _, _, _},
            {X, X, X, _, _},
            {X, _, X, _, _},
            {X, X, X, _, _}
        }
    },

    ['I'] = {
        .width = 1, .pixels = {
            {X, _, _, _, _},
            {X, _, _, _, _},
            {X, _, _, _, _},
            {X, _, _, _, _},
            {X, _, _, _, _}
        }
    },

    ['i'] = {
        .width = 1, .pixels = {
            {_, _, _, _, _},
            {X, _, _, _, _},
            {_, _, _, _, _},
            {X, _, _, _, _},
            {X, _, _, _, _}
        }
    },

    ['D'] = {
        .width = 3, .pixels = {
            {X, X, _, _, _},
            {X, _, X, _, _},
            {X, _, X, _, _},
            {X, _, X, _, _},
            {X, X, _, _, _}
        }
    },

    ['T'] = {
        .width = 3, .pixels = {
            {X, X, X, _, _},
            {_, X, _, _, _},
            {_, X, _, _, _},
            {_, X, _, _, _},
            {_, X, _, _, _}
        }
    },

    ['H'] = {
        .width = 3, .pixels = {
            {X, _, X, _, _},
            {X, _, X, _, _},
            {X, X, X, _, _},
            {X, _, X, _, _},
            {X, _, X, _, _}
        }
    },

    ['h'] = {
        .width = 3, .pixels = {
            {X, _, _, _, _},
            {X, _, _, _, _},
            {X, X, X, _, _},
            {X, _, X, _, _},
            {X, _, X, _, _}
        }
    },

    ['F'] = {
        .width = 3, .pixels = {
            {X, X, X, _, _},
            {X, _, _, _, _},
            {X, X, X, _, _},
            {X, _, _, _, _},
            {X, _, _, _, _}
        }
    },

    ['R'] = {
        .width = 3, .pixels = {
            {X, X, X, _, _},
            {X, _, X, _, _},
            {X, X, X, _, _},
            {X, X, _, _, _},
            {X, _, X, _, _}
        }
    },

    ['r'] = {
        .width = 2, .pixels = {
            {_, _, X, _, _},
            {_, _, _, _, _},
            {X, X, _, _, _},
            {X, _, _, _, _},
            {X, _, _, _, _}
        }
    },

    ['S'] = {
        .width = 3, .pixels = {
            {X, X, X, _, _},
            {X, _, _, _, _},
            {X, X, X, _, _},
            {_, _, X, _, _},
            {X, X, X, _, _}
        }
    },

    ['A'] = {
        .width = 3, .pixels = {
            {X, X, X, _, _},
            {X, _, X, _, _},
            {X, X, X, _, _},
            {X, _, X, _, _},
            {X, _, X, _, _}
        }
    },

    ['a'] = {
        .width = 3, .pixels = {
            {_, _, _, _, _},
            {_, _, _, _, _},
            {_, X, X, _, _},
            {X, _, X, _, _},
            {X, X, X, _, _}
        }
    },

    ['U'] = {
        .width = 3, .pixels = {
            {X, _, X, _, _},
            {X, _, X, _, _},
            {X, _, X, _, _},
            {X, _, X, _, _},
            {X, X, X, _, _}
        }
    },

    ['u'] = {
        .width = 3, .pixels = {
            {_, _, _, _, _},
            {_, _, _, _, _},
            {X, _, X, _, _},
            {X, _, X, _, _},
            {X, X, X, _, _}
        }
    },

    ['W'] = {
        .width = 3, .pixels = {
            {X, _, X, _, _},
            {X, _, X, _, _},
            {X, _, X, _, _},
            {X, X, X, _, _},
            {X, _, X, _, _}
        }
    },

    ['E'] = {
        .width = 3, .pixels = {
            {X, X, X, _, _},
            {X, _, _, _, _},
            {X, X, X, _, _},
            {X, _, _, _, _},
            {X, X, X, _, _}
        }
    },

    ['e'] = {
        .width = 3, .pixels = {
            {_, _, _, _, _},
            {X, X, X, _, _},
            {X, X, X, _, _},
            {X, _, _, _, _},
            {X, X, X, _, _}
        }
    },

    ['/'] = {
        .width = 3, .pixels = {
            {_, _, X, _, _},
            {_, X, _, _, _},
            {_, X, _, _, _},
            {X, _, _, _, _},
            {X, _, _, _, _}
        }
    }
};

// 2. Separate lookup for numbers due to size variants
static const Glyph s_numbers[3][10] = {
    /* Size 3 */
    {
        {
            .width = 3,
            {
                {X,X,X,_,_},
                {X,_,X,_,_},
                {X,_,X,_,_},
                {X,_,X,_,_},
                {X,X,X,_,_}
            },
        },
        {
            .width = 2, {
                {X,X,_,_,_},
                {_,X,_,_,_},
                {_,X,_,_,_},
                {_,X,_,_,_},
                {_,X,_,_,_}
            },
        },
        {
            .width = 3,
            .pixels = {
                {X,X,X,_,_},
                {_,_,X,_,_},
                {X,X,X,_,_},
                {X,_,_,_,_},
                {X,X,X,_,_}
            },
        },
        {
            .width = 3, .pixels = {
                {X,X,X,_,_},
                {_,_,X,_,_},
                {_,X,X,_,_},
                {_,_,X,_,_},
                {X,X,X,_,_}
            },
        },
        {
            .width = 3,
            .pixels = {
                {X,_,X,_,_},
                {X,_,X,_,_},
                {X,X,X,_,_},
                {_,_,X,_,_},
                {_,_,X,_,_}
            },
        },
        {
            .width = 3, .pixels = {
                {X,X,X,_,_},
                {X,_,_,_,_},
                {X,X,X,_,_},
                {_,_,X,_,_},
                {X,X,X,_,_}
            },
        },
        {
            .width = 3,
            .pixels = {
                {X,X,X,_,_},
                {X,_,_,_,_},
                {X,X,X,_,_},
                {X,_,X,_,_},
                {X,X,X,_,_}
            },
        },
        {
            .width = 3, .pixels = {
                {X,X,X,_,_},
                {_,_,X,_,_},
                {_,_,X,_,_},
                {_,_,X,_,_},
                {_,_,X,_,_}
            },
        },
        {
            .width = 3,
            .pixels = {
                {X,X,X,_,_},
                {X,_,X,_,_},
                {X,X,X,_,_},
                {X,_,X,_,_},
                {X,X,X,_,_}
            },
        },
        {
            .width = 3, .pixels = {
                {X,X,X,_,_},
                {X,_,X,_,_},
                {X,X,X,_,_},
                {_,_,X,_,_},
                {X,X,X,_,_}
            },
        }
    },
    /* Size 4 */
    {
        {
            .width = 4,
            .pixels = {
                {X,X,X,X,_},
                {X,_,_,X,_},
                {X,_,_,X,_},
                {X,_,_,X,_},
                {X,X,X,X,_}
            },
        },
        {
            2, .pixels = {
                {X,X,_,_,_},
                {_,X,_,_,_},
                {_,X,_,_,_},
                {_,X,_,_,_},
                {_,X,_,_,_}
            },
        },
        {
            .width = 4,
            .pixels = {
                {X,X,X,X,_},
                {_,_,_,X,_},
                {X,X,X,X,_},
                {X,_,_,_,_},
                {X,X,X,X,_}
            },
        },
        {
            .width = 4, .pixels = {
                {X,X,X,X,_},
                {_,_,_,X,_},
                {_,X,X,X,_},
                {_,_,_,X,_},
                {X,X,X,X,_}
            },
        },
        {
            .width = 4,
            .pixels = {
                {X,_,_,X,_},
                {X,_,_,X,_},
                {X,X,X,X,_},
                {_,_,_,X,_},
                {_,_,_,X,_}
            },
        },
        {
            .width = 4, .pixels = {
                {X,X,X,X,_},
                {X,_,_,_,_},
                {X,X,X,X,_},
                {_,_,_,X,_},
                {X,X,X,X,_}
            },
        },
        {
            .width = 4,
            .pixels = {
                {X,X,X,X,_},
                {X,_,_,_,_},
                {X,X,X,X,_},
                {X,_,_,X,_},
                {X,X,X,X,_}
            },
        },
        {
            .width = 4, .pixels = {
                {X,X,X,X,_},
                {_,_,_,X,_},
                {_,_,_,X,_},
                {_,_,_,X,_},
                {_,_,_,X,_}
            },
        },
        {
            .width = 4,
            .pixels = {
                {X,X,X,X,_},
                {X,_,_,X,_},
                {X,X,X,X,_},
                {X,_,_,X,_},
                {X,X,X,X,_}
            },
        },
        {
            .width = 4, .pixels = {
                {X,X,X,X,_},
                {X,_,_,X,_},
                {X,X,X,X,_},
                {_,_,_,X,_},
                {X,X,X,X,_}
            },
        }
    },
    /* Size 5 */
    {
        {
            .width = 5,
            .pixels = {
                {X,X,X,X,X},
                {X,_,_,_,X},
                {X,_,_,_,X},
                {X,_,_,_,X},
                {X,X,X,X,X}
            },
        },
        {
            2, .pixels = {
                {X,X,_,_,_},
                {_,X,_,_,_},
                {_,X,_,_,_},
                {_,X,_,_,_},
                {_,X,_,_,_}
            },
        },
        {
            .width = 5,
            .pixels = {
                {X,X,X,X,X},
                {_,_,_,_,X},
                {X,X,X,X,X},
                {X,_,_,_,_},
                {X,X,X,X,X}
            },
        },
        {
            .width = 5, .pixels = {
                {X,X,X,X,X},
                {_,_,_,_,X},
                {_,X,X,X,X},
                {_,_,_,_,X},
                {X,X,X,X,X}
            },
        },
        {
            .width = 5,
            .pixels = {
                {X,_,_,_,X},
                {X,_,_,_,X},
                {X,X,X,X,X},
                {_,_,_,_,X},
                {_,_,_,_,X}
            },
        },
        {
            .width = 5, .pixels = {
                {X,X,X,X,X},
                {X,_,_,_,_},
                {X,X,X,X,X},
                {_,_,_,_,X},
                {X,X,X,X,X}
            },
        },
        {
            .width = 5,
            .pixels = {
                {X,X,X,X,X},
                {X,_,_,_,_},
                {X,X,X,X,X},
                {X,_,_,_,X},
                {X,X,X,X,X}
            },
        },
        {
            .width = 5, .pixels = {
                {X,X,X,X,X},
                {_,_,_,_,X},
                {_,_,_,_,X},
                {_,_,_,_,X},
                {_,_,_,_,X}
            },
        },
        {
            .width = 5,
            .pixels = {
                {X,X,X,X,X},
                {X,_,_,_,X},
                {X,X,X,X,X},
                {X,_,_,_,X},
                {X,X,X,X,X}
            },
        },
        {
            .width = 5, .pixels = {
                {X,X,X,X,X},
                {X,_,_,_,X},
                {X,X,X,X,X},
                {_,_,_,_,X},
                {X,X,X,X,X}
            },
        }
    }
};

int pixel_matrix_drawer_draw_char(
    GContext *ctx, const GPoint point_zero, const char character,
    const int dot_width, const int dot_height,
    const int gap_size_horizontal, const int gap_size_vertical,
    const bool align_right, const bool solid_blocks, const int digit_size) {
    if (!character) return 0;

    Glyph glyph;

    // 1. Look up the character instantly based on its ASCII value
    if (character >= '0' && character <= '9') {
        const int size_idx = (digit_size == 5) ? 2 : ((digit_size == 4) ? 1 : 0);
        glyph = s_numbers[size_idx][character - '0'];
    } else {
        // Simply check if it's within ASCII range
        glyph = s_chars[(int) character];
    }

    // 2. If the character wasn't in our arrays, use the default block
    if (glyph.width == 0) {
        glyph = s_default;
    }

    // 3. Draw the pixels using clean, simplified math
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < glyph.width; col++) {
            // Skip if pixel is false ('_')
            if (!glyph.pixels[row][col]) {
                continue;
            }

            int x_offset = col * (dot_width + gap_size_horizontal);

            if (align_right) {
                x_offset -= (glyph.width * dot_width + (glyph.width - 1) * gap_size_horizontal);
            }

            const int x = point_zero.x + x_offset;
            const int y = point_zero.y + row * (dot_height + gap_size_vertical);
            const int draw_width = solid_blocks && col < glyph.width - 1
                                       ? dot_width + gap_size_horizontal
                                       : dot_width;
            const int draw_height = solid_blocks && row < 4
                                        ? dot_height + gap_size_vertical
                                        : dot_height;

            const GRect dot_bounds = GRect(x, y, draw_width, draw_height);
            graphics_fill_rect(ctx, dot_bounds, 0, GCornerNone);
        }
    }

    return glyph.width;
}

// Undefine the macros so they don't leak into other files
#undef X
#undef _