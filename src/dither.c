#include <pebble.h>
#include "dither.h"

static const uint8_t dither_table[8][8] = {
  { 0, 32,  8, 40,  2, 34, 10, 42}, /* 8x8 Bayer ordered dithering */
  {48, 16, 56, 24, 50, 18, 58, 26}, /* pattern. Each input pixel */
  {12, 44,  4, 36, 14, 46,  6, 38}, /* is scaled to the 0..63 range */
  {60, 28, 52, 20, 62, 30, 54, 22}, /* before looking in this table */
  { 3, 35, 11, 43,  1, 33,  9, 41}, /* to determine the action. */
  {51, 19, 59, 27, 49, 17, 57, 25},
  {15, 47,  7, 39, 13, 45,  5, 37},
  {63, 31, 55, 23, 61, 29, 53, 21}
};

// ------------------------------------------------------------------------------------------------------------------------------------------------ //
//  Dither Functions
// ------------------------------------------------------------------------ //
uint8_t get_intensity_dithered(int x, int y, uint8_t intensity) {
  #if defined(PBL_BW)
    return (intensity+5)/4 - 1 > dither_table[x&7][y&7] ? 1 : 0;
  #elif defined(PBL_COLOR)
    return 64*(intensity%85)/85 > dither_table[x&7][y&7] ? intensity/85+1 : intensity/85;
  #else
    return 0;  // I dunno, not Color, not B&W.
  #endif
}


uint8_t get_value_dithered(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  #if defined(PBL_BW) && defined(PBL_RECT)
    return ((((r+r+r+b+g+g+g+g) >> 3)+5)/4 - 1) > dither_table[x&7][y&7] ? 1 : 0;
  #elif defined(PBL_COLOR)
    return 0b11000000 +
           ((64*(r%85)/85 > dither_table[x&7][y&7] ? r/85+1 : r/85)<<4) + 
           ((64*(g%85)/85 > dither_table[x&7][y&7] ? g/85+1 : g/85)<<2) + 
            (64*(b%85)/85 > dither_table[x&7][y&7] ? b/85+1 : b/85);
  #endif
}


GColor get_color_dithered(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  uint8_t d = dither_table[x&7][y&7];
  #if defined(PBL_BW) && defined(PBL_RECT)
  return ((((r+r+r+b+g+g+g+g) >> 3)+5)/4 - 1) > d ? GColorWhite : GColorBlack;
  #elif defined(PBL_COLOR)
  return (GColor){.argb = 0b11000000 + ((64*(r%85)/85 > d ? r/85+1 : r/85)<<4) + ((64*(g%85)/85 > d ? g/85+1 : g/85)<<2) + (64*(b%85)/85 > d ? b/85+1 : b/85)};
  #endif
}


// ------------------------------------------------------------------------------------------------------------------------------------------------ //
//  Functions that use dithering
// ------------------------------------------------------------------------ //
void fill_rect_dithered(GContext *ctx, GRect rect, uint8_t r, uint8_t g, uint8_t b) {
  GBitmap* framebuffer = graphics_capture_frame_buffer(ctx);  // Capture the framebuffer
  if(framebuffer) {                                           // if successfully captured the framebuffer
    int16_t h = gbitmap_get_bounds(framebuffer).size.h;                      // Get visible height
    if(rect.origin.y<0) {rect.size.h += rect.origin.y; rect.origin.y = 0;}                   // If top    of rect is visible
    if(rect.origin.y + rect.size.h > h) rect.size.h = h - rect.origin.y;                     // If bottom of rect is visible
    for(int y = rect.origin.y; y < rect.origin.y+rect.size.h; y++) {         // Iterate over all y of visible part of rect

      GBitmapDataRowInfo info = gbitmap_get_data_row_info(framebuffer, y);   // Get visible width
      if(info.min_x < rect.origin.x) info.min_x = rect.origin.x;                             // If left   of rect is visible
      if(info.max_x > rect.origin.x + rect.size.w) info.max_x = rect.origin.x + rect.size.w; // If right  of rect is visible
      for(int x = info.min_x; x <= info.max_x; x++) {                        // Iterate over all x of visible part of rect

        // Draw the dithers!  Method 1: Using Functions
        //  #if defined(PBL_BW) && defined(PBL_RECT)
        //    info.data[x/8] &= ~(1 << (x%8));                                // Color Pixel Black
        //    info.data[x/8] |= get_value_dithered(x, y, r, g, b) << (x%8);   // Color Pixel White if dither says to
        // #elif defined(PBL_COLOR)
        //   info.data[x] = get_value_dithered(x, y, r, g, b);
        // #endif
        
        // Draw the dithers!  Method 2: Directly!
        uint8_t d = dither_table[x&7][y&7];
        #if defined(PBL_BW) && defined(PBL_RECT)
          uint8_t intensity = (r+r+r+b+g+g+g+g) >> 3;         // intensity = Average RGB into a single 0-255 "brightness" value
          info.data[x/8] &= ~(1 << (x%8));                                // Color Pixel Black
          info.data[x/8] |= (((intensity+5)/4 - 1) > d ? 1 : 0) << (x%8); // Color Pixel White if dither says to (intensity 0-64)
        #elif defined(PBL_COLOR)
          uint8_t r2 = r/85; if ((64*(r%85))/85 > d) r2++;
          uint8_t g2 = g/85; if ((64*(g%85))/85 > d) g2++;
          uint8_t b2 = b/85; if ((64*(b%85))/85 > d) b2++;
          info.data[x] = 0b11000000 + (r2<<4) + (g2<<2) + b2;
        #endif
      }
    }
    graphics_release_frame_buffer(ctx, framebuffer);  // Release the Kraken! err, framebuffer.
  }
}
