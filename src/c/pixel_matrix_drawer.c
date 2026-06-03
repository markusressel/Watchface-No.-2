#include "pixel_matrix_drawer.h"

// Visual macros to "draw" the pixels in the code
#define X 1
#define _ 0
// Helper Makros to keep the "X" vs "_" code notation while
// using bit-packing for in-memory representation (instead of a bool-matrix).
// This uses about 75% less memory.
#define ROW(a, b, c, d, e) ((a << 4) | (b << 3) | (c << 2) | (d << 1) | e)
#define MATRIX(r0, r1, r2, r3, r4) ((r0 << 20) | (r1 << 15) | (r2 << 10) | (r3 << 5) | r4)

// Struct to bundle a character's matrix and its width together
typedef struct {
    int width;
    // Bit-Packing to store the full 5x5 matrix in a single integer
    uint32_t pixels;
} Glyph;

// Default fallback character (Full 5x5 block)
static const Glyph s_default = {
    .width = 5, .pixels = MATRIX(
        ROW(X, X, X, X, X),
        ROW(X, X, X, X, X),
        ROW(X, X, X, X, X),
        ROW(X, X, X, X, X),
        ROW(X, X, X, X, X)
    )
};
// 1. All standard characters in a single lookup table
static const Glyph s_chars[128] = {
    ['.'] = {
        .width = 1, .pixels = MATRIX(
            ROW(_, _, _, _, _),
            ROW(_, _, _, _, _),
            ROW(_, _, _, _, _),
            ROW(_, _, _, _, _),
            ROW(X, _, _, _, _)
        )
    },

    [','] = {
        .width = 1, .pixels = MATRIX(
            ROW(_, _, _, _, _),
            ROW(_, _, _, _, _),
            ROW(_, _, _, _, _),
            ROW(X, _, _, _, _),
            ROW(X, _, _, _, _)
        )
    },

    [':'] = {
        .width = 1, .pixels = MATRIX(
            ROW(_, _, _, _, _),
            ROW(X, _, _, _, _),
            ROW(_, _, _, _, _),
            ROW(X, _, _, _, _),
            ROW(_, _, _, _, _)
        )
    },

    ['|'] = {
        .width = 3, .pixels = MATRIX(
            ROW(_, X, _, _, _),
            ROW(_, X, _, _, _),
            ROW(_, X, _, _, _),
            ROW(_, X, _, _, _),
            ROW(_, X, _, _, _)
        )
    },

    ['-'] = {
        .width = 5, .pixels = MATRIX(
            ROW(_, _, _, _, _),
            ROW(_, _, _, _, _),
            ROW(X, X, X, X, X),
            ROW(_, _, _, _, _),
            ROW(_, _, _, _, _)
        )
    },

    [' '] = {
        .width = 1, .pixels = MATRIX(
            ROW(_, _, _, _, _),
            ROW(_, _, _, _, _),
            ROW(_, _, _, _, _),
            ROW(_, _, _, _, _),
            ROW(_, _, _, _, _)
        )
    },

    ['C'] = {
        .width = 5, .pixels = MATRIX(
            ROW(X, X, X, X, X),
            ROW(X, _, _, _, _),
            ROW(X, _, _, _, _),
            ROW(X, _, _, _, _),
            ROW(X, X, X, X, X)
        )
    },

    ['c'] = {
        .width = 5, .pixels = MATRIX(
            ROW(_, _, _, _, _),
            ROW(_, _, _, _, _),
            ROW(X, X, X, X, X),
            ROW(X, _, _, _, _),
            ROW(X, X, X, X, X)
        )
    },

    ['M'] = {
        .width = 3, .pixels = MATRIX(
            ROW(X, _, X, _, _),
            ROW(X, X, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _)
        )
    },

    ['O'] = {
        .width = 3, .pixels = MATRIX(
            ROW(X, X, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, X, X, _, _)
        )
    },

    ['o'] = {
        .width = 3, .pixels = MATRIX(
            ROW(_, _, _, _, _),
            ROW(_, _, _, _, _),
            ROW(X, X, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, X, X, _, _)
        )
    },

    ['I'] = {
        .width = 1, .pixels = MATRIX(
            ROW(X, _, _, _, _),
            ROW(X, _, _, _, _),
            ROW(X, _, _, _, _),
            ROW(X, _, _, _, _),
            ROW(X, _, _, _, _)
        )
    },

    ['i'] = {
        .width = 1, .pixels = MATRIX(
            ROW(_, _, _, _, _),
            ROW(X, _, _, _, _),
            ROW(_, _, _, _, _),
            ROW(X, _, _, _, _),
            ROW(X, _, _, _, _)
        )
    },

    ['D'] = {
        .width = 3, .pixels = MATRIX(
            ROW(X, X, _, _, _),
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, X, _, _, _)
        )
    },

    ['T'] = {
        .width = 3, .pixels = MATRIX(
            ROW(X, X, X, _, _),
            ROW(_, X, _, _, _),
            ROW(_, X, _, _, _),
            ROW(_, X, _, _, _),
            ROW(_, X, _, _, _)
        )
    },

    ['H'] = {
        .width = 3, .pixels = MATRIX(
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, X, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _)
        )
    },

    ['h'] = {
        .width = 3, .pixels = MATRIX(
            ROW(X, _, _, _, _),
            ROW(X, _, _, _, _),
            ROW(X, X, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _)
        )
    },

    ['F'] = {
        .width = 3, .pixels = MATRIX(
            ROW(X, X, X, _, _),
            ROW(X, _, _, _, _),
            ROW(X, X, X, _, _),
            ROW(X, _, _, _, _),
            ROW(X, _, _, _, _)
        )
    },

    ['R'] = {
        .width = 3, .pixels = MATRIX(
            ROW(X, X, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, X, X, _, _),
            ROW(X, X, _, _, _),
            ROW(X, _, X, _, _)
        )
    },

    ['r'] = {
        .width = 2, .pixels = MATRIX(
            ROW(_, _, X, _, _),
            ROW(_, _, _, _, _),
            ROW(X, X, _, _, _),
            ROW(X, _, _, _, _),
            ROW(X, _, _, _, _)
        )
    },

    ['S'] = {
        .width = 3, .pixels = MATRIX(
            ROW(X, X, X, _, _),
            ROW(X, _, _, _, _),
            ROW(X, X, X, _, _),
            ROW(_, _, X, _, _),
            ROW(X, X, X, _, _)
        )
    },

    ['A'] = {
        .width = 3, .pixels = MATRIX(
            ROW(X, X, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, X, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _)
        )
    },

    ['a'] = {
        .width = 3, .pixels = MATRIX(
            ROW(_, _, _, _, _),
            ROW(_, _, _, _, _),
            ROW(_, X, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, X, X, _, _)
        )
    },

    ['U'] = {
        .width = 3, .pixels = MATRIX(
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, X, X, _, _)
        )
    },

    ['u'] = {
        .width = 3, .pixels = MATRIX(
            ROW(_, _, _, _, _),
            ROW(_, _, _, _, _),
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, X, X, _, _)
        )
    },

    ['W'] = {
        .width = 3, .pixels = MATRIX(
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, _, X, _, _),
            ROW(X, X, X, _, _),
            ROW(X, _, X, _, _)
        )
    },

    ['E'] = {
        .width = 3, .pixels = MATRIX(
            ROW(X, X, X, _, _),
            ROW(X, _, _, _, _),
            ROW(X, X, X, _, _),
            ROW(X, _, _, _, _),
            ROW(X, X, X, _, _)
        )
    },

    ['e'] = {
        .width = 3, .pixels = MATRIX(
            ROW(_, _, _, _, _),
            ROW(X, X, X, _, _),
            ROW(X, X, X, _, _),
            ROW(X, _, _, _, _),
            ROW(X, X, X, _, _)
        )
    },

    ['/'] = {
        .width = 3, .pixels = MATRIX(
            ROW(_, _, X, _, _),
            ROW(_, X, _, _, _),
            ROW(_, X, _, _, _),
            ROW(X, _, _, _, _),
            ROW(X, _, _, _, _)
        )
    }
};

// 2. Separate lookup for numbers due to size variants
static const Glyph s_numbers[3][10] = {
    /* Size 3 */
    {
        {
            .width = 3, .pixels = MATRIX(
                ROW(X,X,X,_,_),
                ROW(X,_,X,_,_),
                ROW(X,_,X,_,_),
                ROW(X,_,X,_,_),
                ROW(X,X,X,_,_)
            ),
        },
        {
            .width = 2, .pixels = MATRIX(
                ROW(X,X,_,_,_),
                ROW(_,X,_,_,_),
                ROW(_,X,_,_,_),
                ROW(_,X,_,_,_),
                ROW(_,X,_,_,_)
            ),
        },
        {
            .width = 3, .pixels = MATRIX(
                ROW(X,X,X,_,_),
                ROW(_,_,X,_,_),
                ROW(X,X,X,_,_),
                ROW(X,_,_,_,_),
                ROW(X,X,X,_,_)
            ),
        },
        {
            .width = 3, .pixels = MATRIX(
                ROW(X,X,X,_,_),
                ROW(_,_,X,_,_),
                ROW(_,X,X,_,_),
                ROW(_,_,X,_,_),
                ROW(X,X,X,_,_)
            ),
        },
        {
            .width = 3, .pixels = MATRIX(
                ROW(X,_,X,_,_),
                ROW(X,_,X,_,_),
                ROW(X,X,X,_,_),
                ROW(_,_,X,_,_),
                ROW(_,_,X,_,_)
            ),
        },
        {
            .width = 3, .pixels = MATRIX(
                ROW(X,X,X,_,_),
                ROW(X,_,_,_,_),
                ROW(X,X,X,_,_),
                ROW(_,_,X,_,_),
                ROW(X,X,X,_,_)
            ),
        },
        {
            .width = 3, .pixels = MATRIX(
                ROW(X,X,X,_,_),
                ROW(X,_,_,_,_),
                ROW(X,X,X,_,_),
                ROW(X,_,X,_,_),
                ROW(X,X,X,_,_)
            ),
        },
        {
            .width = 3, .pixels = MATRIX(
                ROW(X,X,X,_,_),
                ROW(_,_,X,_,_),
                ROW(_,_,X,_,_),
                ROW(_,_,X,_,_),
                ROW(_,_,X,_,_)
            ),
        },
        {
            .width = 3, .pixels = MATRIX(
                ROW(X,X,X,_,_),
                ROW(X,_,X,_,_),
                ROW(X,X,X,_,_),
                ROW(X,_,X,_,_),
                ROW(X,X,X,_,_)
            ),
        },
        {
            .width = 3, .pixels = MATRIX(
                ROW(X,X,X,_,_),
                ROW(X,_,X,_,_),
                ROW(X,X,X,_,_),
                ROW(_,_,X,_,_),
                ROW(X,X,X,_,_)
            ),
        }
    },
    /* Size 4 */
    {
        {
            .width = 4, .pixels = MATRIX(
                ROW(X,X,X,X,_),
                ROW(X,_,_,X,_),
                ROW(X,_,_,X,_),
                ROW(X,_,_,X,_),
                ROW(X,X,X,X,_)
            ),
        },
        {
            .width = 2, .pixels = MATRIX(
                ROW(X,X,_,_,_),
                ROW(_,X,_,_,_),
                ROW(_,X,_,_,_),
                ROW(_,X,_,_,_),
                ROW(_,X,_,_,_)
            ),
        },
        {
            .width = 4, .pixels = MATRIX(
                ROW(X,X,X,X,_),
                ROW(_,_,_,X,_),
                ROW(X,X,X,X,_),
                ROW(X,_,_,_,_),
                ROW(X,X,X,X,_)
            ),
        },
        {
            .width = 4, .pixels = MATRIX(
                ROW(X,X,X,X,_),
                ROW(_,_,_,X,_),
                ROW(_,X,X,X,_),
                ROW(_,_,_,X,_),
                ROW(X,X,X,X,_)
            ),
        },
        {
            .width = 4, .pixels = MATRIX(
                ROW(X,_,_,X,_),
                ROW(X,_,_,X,_),
                ROW(X,X,X,X,_),
                ROW(_,_,_,X,_),
                ROW(_,_,_,X,_)
            ),
        },
        {
            .width = 4, .pixels = MATRIX(
                ROW(X,X,X,X,_),
                ROW(X,_,_,_,_),
                ROW(X,X,X,X,_),
                ROW(_,_,_,X,_),
                ROW(X,X,X,X,_)
            ),
        },
        {
            .width = 4, .pixels = MATRIX(
                ROW(X,X,X,X,_),
                ROW(X,_,_,_,_),
                ROW(X,X,X,X,_),
                ROW(X,_,_,X,_),
                ROW(X,X,X,X,_)
            ),
        },
        {
            .width = 4, .pixels = MATRIX(
                ROW(X,X,X,X,_),
                ROW(_,_,_,X,_),
                ROW(_,_,_,X,_),
                ROW(_,_,_,X,_),
                ROW(_,_,_,X,_)
            ),
        },
        {
            .width = 4, .pixels = MATRIX(
                ROW(X,X,X,X,_),
                ROW(X,_,_,X,_),
                ROW(X,X,X,X,_),
                ROW(X,_,_,X,_),
                ROW(X,X,X,X,_)
            ),
        },
        {
            .width = 4, .pixels = MATRIX(
                ROW(X,X,X,X,_),
                ROW(X,_,_,X,_),
                ROW(X,X,X,X,_),
                ROW(_,_,_,X,_),
                ROW(X,X,X,X,_)
            ),
        }
    },
    /* Size 5 */
    {
        {
            .width = 5, .pixels = MATRIX(
                ROW(X,X,X,X,X),
                ROW(X,_,_,_,X),
                ROW(X,_,_,_,X),
                ROW(X,_,_,_,X),
                ROW(X,X,X,X,X)
            ),
        },
        {
            .width = 2, .pixels = MATRIX(
                ROW(X,X,_,_,_),
                ROW(_,X,_,_,_),
                ROW(_,X,_,_,_),
                ROW(_,X,_,_,_),
                ROW(_,X,_,_,_)
            ),
        },
        {
            .width = 5, .pixels = MATRIX(
                ROW(X,X,X,X,X),
                ROW(_,_,_,_,X),
                ROW(X,X,X,X,X),
                ROW(X,_,_,_,_),
                ROW(X,X,X,X,X)
            ),
        },
        {
            .width = 5, .pixels = MATRIX(
                ROW(X,X,X,X,X),
                ROW(_,_,_,_,X),
                ROW(_,X,X,X,X),
                ROW(_,_,_,_,X),
                ROW(X,X,X,X,X)
            ),
        },
        {
            .width = 5, .pixels = MATRIX(
                ROW(X,_,_,_,X),
                ROW(X,_,_,_,X),
                ROW(X,X,X,X,X),
                ROW(_,_,_,_,X),
                ROW(_,_,_,_,X)
            ),
        },
        {
            .width = 5, .pixels = MATRIX(
                ROW(X,X,X,X,X),
                ROW(X,_,_,_,_),
                ROW(X,X,X,X,X),
                ROW(_,_,_,_,X),
                ROW(X,X,X,X,X)
            ),
        },
        {
            .width = 5, .pixels = MATRIX(
                ROW(X,X,X,X,X),
                ROW(X,_,_,_,_),
                ROW(X,X,X,X,X),
                ROW(X,_,_,_,X),
                ROW(X,X,X,X,X)
            ),
        },
        {
            .width = 5, .pixels = MATRIX(
                ROW(X,X,X,X,X),
                ROW(_, _, _, _, X),
                ROW(_, _, _, _, X),
                ROW(_, _, _, _, X),
                ROW(_, _, _, _, X)
            ),
        },
        {
            .width = 5, .pixels = MATRIX(
                ROW(X,X,X,X,X),
                ROW(X, _, _, _, X),
                ROW(X, X, X, X, X),
                ROW(X, _, _, _, X),
                ROW(X, X, X, X, X)
            ),
        },
        {
            .width = 5, .pixels = MATRIX(
                ROW(X,X,X,X,X),
                ROW(X, _, _, _, X),
                ROW(X, X, X, X, X),
                ROW(_, _, _, _, X),
                ROW(X, X, X, X, X)
            ),
        }
    }
};

static Glyph glyph_for_char(const char character, const int digit_size) {
    Glyph glyph;

    if (character >= '0' && character <= '9') {
        const int size_idx = (digit_size == 5) ? 2 : ((digit_size == 4) ? 1 : 0);
        glyph = s_numbers[size_idx][character - '0'];
    } else {
        glyph = s_chars[(int) character];
    }

    if (glyph.width == 0) {
        glyph = s_default;
    }

    return glyph;
}

int pixel_matrix_drawer_char_width(const char character, const int digit_size) {
    if (!character) return 0;
    return glyph_for_char(character, digit_size).width;
}

int pixel_matrix_drawer_draw_char(
    GContext *ctx, const GPoint point_zero, const char character,
    const int dot_width, const int dot_height,
    const int gap_size_horizontal, const int gap_size_vertical,
    const bool align_right, const int digit_size) {
    if (!character) return 0;

    Glyph glyph = glyph_for_char(character, digit_size);

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < glyph.width; col++) {
            const int bit_index = 24 - (row * 5 + col);

            // Skip if pixel is false ('_')
            if (!((glyph.pixels >> bit_index) & 1)) {
                continue;
            }

            int x_offset = col * (dot_width + gap_size_horizontal);

            if (align_right) {
                x_offset -= (glyph.width * dot_width + (glyph.width - 1) * gap_size_horizontal);
            }

            const int x = point_zero.x + x_offset;
            const int y = point_zero.y + row * (dot_height + gap_size_vertical);
            graphics_fill_rect(ctx, GRect(x, y, dot_width, dot_height), 0, GCornerNone);
        }
    }

    return glyph.width;
}

// Undefine the macros so they don't leak into other files
#undef X
#undef _
