#pragma once
#include <pebble.h>

//    Input: An x and y screen coordinate and an intensity [0-255]:
// On   B&W: returns either 0 or 1 for black or white
// On Color: returns either 0, 1, 2 or 3 (Black, Dark, Lighter, Light)
// usage examples:
//   grays = 0b11000000 + 0b00010101 * get_gray_dithered(x, y, intensity);
//    reds = 0b11000000 + 0b00010000 * get_gray_dithered(x, y, intensity);
//   cyans = 0b11000000 + 0b00000101 * get_gray_dithered(x, y, intensity);
//   graphics_context_set_stroke_color(ctx, (GColor){.argb = grays});
//   graphics_draw_pixel(ctx, GPoint(x, y));
uint8_t get_intensity_dithered(int x, int y, uint8_t intensity);


//    Input: An x and y screen coordinate and a 24bit RGB color [0-255, 0-255, 0-255]
// On   B&W: returns either GColorBlack or GColorWhite
// On Color: returns a GColor adjusted and dithered for that (x, y) coordinate
// usage example:
//   Draw_Dithered_Horizontal_Line(x1, x2, y, r, g, b) {
//     for (x = x1; x < x2; x++) {
//       graphics_context_set_stroke_color(ctx, get_color_dithered(x, y, r, g, b));
//       graphics_draw_pixel(ctx, GPoint(x, y));
//     }
//   }
GColor get_color_dithered(int x, int y, uint8_t r, uint8_t g, uint8_t b);


// Just like get_color_dithered(), but returns the number instead of the GColor
//  Basically: get_color_dithered(...) = (GColor){.argb=get_value_dithered(...)}
uint8_t get_value_dithered(int x, int y, uint8_t r, uint8_t g, uint8_t b);


// Input: A graphics context, a rect (absolute screen coordinates), and a 24bit RGB color
// Draws a rectangle filled with a dithered (patterned) color
// Note: uses absolute screen coordinates, unlike graphics_fill_rect() which draws relative to its layer
void fill_rect_dithered(GContext *ctx, GRect rect, uint8_t r, uint8_t g, uint8_t b);
