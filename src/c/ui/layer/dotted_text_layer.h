#pragma once
#include <pebble.h>

// A text layer that renders characters individually using the pebble graphics api
// in a dotted style
typedef Layer DottedTextLayer;

typedef enum DottedTextOffsetUnit {
    DOTTED_TEXT_OFFSET_UNIT_PIXELS = 0,
    DOTTED_TEXT_OFFSET_UNIT_BLOCKS = 1,
} DottedTextOffsetUnit;

typedef enum HorizontalAlignment {
    HORIZONTAL_ALIGN_LEFT = 0,
    HORIZONTAL_ALIGN_CENTER = 1,
    HORIZONTAL_ALIGN_RIGHT = 2,
} HorizontalAlignment;

typedef enum VerticalAlignment {
    VERTICAL_ALIGN_TOP = 0,
    VERTICAL_ALIGN_CENTER = 1,
    VERTICAL_ALIGN_BOTTOM = 2,
} VerticalAlignment;

typedef enum DottedTextRenderingMode {
    DOTTED_TEXT_RENDERING_MODE_PIXEL_PERFECT = 0,
    DOTTED_TEXT_RENDERING_MODE_SUBPIXEL = 1,
} DottedTextRenderingMode;

typedef struct DottedTextLayerData {
    char *text;
    float scale_factor;
    GColor text_color;
    int16_t character_offset_value;
    int8_t custom_dot_width;
    int8_t custom_dot_height;
    int8_t custom_gap_horizontal;
    int8_t custom_gap_vertical;
    int8_t custom_digit_width;
    uint8_t horizontal_alignment: HORIZONTAL_ALIGN_RIGHT;
    uint8_t vertical_alignment: VERTICAL_ALIGN_BOTTOM;
    uint8_t rendering_mode: DOTTED_TEXT_RENDERING_MODE_SUBPIXEL;
    uint8_t character_offset_unit: DOTTED_TEXT_OFFSET_UNIT_BLOCKS;
    bool character_offset_overridden: true;
    bool auto_scale: true;
    bool use_custom_metrics: true;
} __attribute__((__packed__)) DottedTextLayerData;


// Use this method to create a DottedTextLayer
//
// @param bounds  The boundaries for the created layer
DottedTextLayer *dotted_text_layer_create(GRect bounds);


// Use this method to get the base layer of the given DottedTextLayer
//
// @param dotted_text_layer  The DottedTextLayer to get the base layer from
// Layer dotted_text_layer_get_layer(DottedTextLayer* dotted_text_layer);


// Use this method to set the text for the layer
// 
// Keep in mind that this layer only supports a small subset of all characters
// and they are drawn manually using graphics api, not like a font.
//
// @param dotted_text_layer  the layer you want to set the text to
// @param text               the text to set
void dotted_text_layer_set_text(DottedTextLayer *dotted_text_layer, char *text);


// Use this method to set the text color for the layer
// 
// @param dotted_text_layer  the layer you want to set the color to
// @param color              the color to set
void dotted_text_layer_set_color(DottedTextLayer *dotted_text_layer, GColor color);

void dotted_text_layer_set_horizontal_alignment(
    DottedTextLayer *dotted_text_layer,
    HorizontalAlignment alignment
);
void dotted_text_layer_set_vertical_alignment(
    DottedTextLayer *dotted_text_layer,
    VerticalAlignment alignment
);

void dotted_text_layer_set_rendering_mode(
    DottedTextLayer *dotted_text_layer,
    DottedTextRenderingMode mode
);

// Use this method to set the scale factor for the layer.
// This scales dot size, dot gaps, and spacing between characters.
//
// @param dotted_text_layer  the layer you want to set the color to
// @param scale              the scale factor to set (> 0.0f)
void dotted_text_layer_set_scale_factor(DottedTextLayer *dotted_text_layer, float scale);

// Override spacing between adjacent characters.
// unit:
// - DOTTED_TEXT_OFFSET_PIXELS: value is interpreted as pixels before scaling
// - DOTTED_TEXT_OFFSET_BLOCKS: value is interpreted as block-count * dot-width
void dotted_text_layer_set_character_offset(
    DottedTextLayer *dotted_text_layer,
    int value,
    DottedTextOffsetUnit unit
);

// Enable/disable automatic scale fitting to the layer height.
// When enabled, scale is computed from available vertical space.
void dotted_text_layer_set_auto_scale(DottedTextLayer *dotted_text_layer, bool enabled);

// Override matrix metrics (base values before scaling) for this layer only.
void dotted_text_layer_set_custom_metrics(
    DottedTextLayer *dotted_text_layer,
    int dot_width,
    int dot_height,
    int gap_horizontal,
    int gap_vertical
);

// Override per-layer digit width used for matrix glyph selection.
// Pass values >= 1. The default is global clay_get_settings()->DigitWidth.
void dotted_text_layer_set_digit_width(
    DottedTextLayer *dotted_text_layer,
    int digit_width
);

// Calculate the total width of the current text content in pixels,
// considering current scale and metrics.
int dotted_text_layer_get_content_width(DottedTextLayer *dotted_text_layer);


// Use this method to destroy a DottedTextLayer
//
// @param dotted_text_layer  The layer to destroy
void dotted_text_layer_destroy(DottedTextLayer *dotted_text_layer);
