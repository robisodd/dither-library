/* ----------------------------------------------------------------------------------
 References:
   dither_table from:
     http://www.efg2.com/Lab/Library/ImageProcessing/DHALF.TXT
     
   Luminance equation "Y = (R+R+R+B+G+G+G+G)>>3" from
    http://stackoverflow.com/questions/596216



 Open Source Copyright:
   This software is protected under The MIT License (MIT)
   By using any piece of this software, you are agreeing to be bound by this license.
   Read the details here: https://opensource.org/licenses/MIT
   Copyright (c) 2016 Rob Spiess



 Version History:
          TODO: Release as an NPM Pebble package
                Consider updating to work with antialiasing
          
   v6-20160623: Created replace_color_in_rect_with_dithered()
                Removed all single-pixel functions
                fill_rect_dithered() now points to replace_color_in_rect_with_dithered()
                Created replace_color_with_dithered()
                Added Comments up the wazoo
                Add MIT Open Source License and Version History
                
   sd-20160622: Adapted and released as a Special-Draw extension
                Special-draw: jneubrand.github.io/special-draw
                              github.com/jneubrand/special-draw
                Released as:  github.com/robisodd/special-draw-dither
   
   v5-20160622: Separated dither functions into separate .c/.h library
                Created individual pixel dither functions:
                  get_intensity_dithered() for 2bit gray and B&W
                  get_value_dithered() for 24bit color and 1bit B&W
                  get_color_dithered(), same as value but returns GColor
                Released as: github.com/robisodd/dither-library
                
   v4-20160622: Separated layers: Dither layer and text layer
                Removed fill_screen_dithered() -- rect works fine
                Reverted back to framebuffer drawing (was too slow)
                Initial public release: github.com/robisodd/dither
                
   v3-20160621: Created fill_rect_dithered()
                Created fill_screen_dithered()
                layer update now calls fill_rect_dithered()
                Uses graphics_context_set_stroke_color() and graphics_draw_pixel()
                One set of code works on both color and B&W
                
   v2-20160620: Added support for Aplite (B&W)
                Added support for Chalk and future Pebble Watch releases
                Settled on 8x8 dither matrix
                Remove 4x4 matrix and 16x16 matrix
                
   v1-20160619: Initial Internal Release
                Got 1bit dithering working on Basalt in a small amount of code
                Figured out how to adapt 1bit dither code to 2bits automatically
                Implemented 24bit RGB -> 6bit dithered color for Basalt
                Implemented 8bit -> 2bit dithered grayscale for Basalt
---------------------------------------------------------------------------------- */

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


// Searches rect for pixels of replacement_color and replaces them with a dithered 24bit RGB color
// Note:  rect is in Absolute Screen Coordinates, not adjusted for any child layer
// Note2: If replacement_color is GColorClear (= 0), replaces every pixel in rect with the dithered 24bit RGB color
void replace_color_in_rect_with_dithered(GContext *ctx, GRect rect, GColor replacement_color, uint8_t r, uint8_t g, uint8_t b) {
  GBitmap* framebuffer = graphics_capture_frame_buffer(ctx);  // Capture the framebuffer
  if(framebuffer) {                                           // if successfully captured the framebuffer
    
    #if defined(PBL_BW)                                       // Calculate intensity now instead of inside loop
      uint8_t intensity = (r+r+r+b+g+g+g+g) >> 3;             // intensity = Average RGB into a single 0-255 "brightness" value
     intensity = (intensity + 5) / 4 - 1;                     // convert to a 0-64 value to compare against dither_table
    #endif
    
    // Section 1: Bounds Checking and Iteration
    int16_t h = gbitmap_get_bounds(framebuffer).size.h;                     // Get screen height
    if(rect.origin.y<0) {rect.size.h += rect.origin.y; rect.origin.y = 0;}  // if top    is beyond screen bounds, then adjust
    if(rect.origin.y + rect.size.h > h) rect.size.h = h - rect.origin.y;    // if bottom is beyond screen bounds, then adjust
    for(int y = rect.origin.y; y < rect.origin.y+rect.size.h; y++) {        // Iterate over all y of visible part of rect

      GBitmapDataRowInfo info = gbitmap_get_data_row_info(framebuffer, y);  // Get visible width
      if(info.min_x < rect.origin.x) info.min_x = rect.origin.x;            // If left  is within screen bounds, then adjust
      if(info.max_x > rect.origin.x + rect.size.w)                          // If right is within screen bounds
        info.max_x = rect.origin.x + rect.size.w;                           //   then adjust right side
      for(int x = info.min_x; x <= info.max_x; x++) {                       // Iterate over all x of visible part of rect

        // Section 2: Search the pixels and replace with dither
        #if defined(PBL_BW) && defined(PBL_RECT)                            // Just in case Pebble release a "B&W Round"
          if(!(info.data[x/8] & (1<<(x%8))) == !(replacement_color.argb&1)  // If screen pixel color = replacement_color
            || !replacement_color.argb) {                                   //   or if replacement_color = 0 (see Note2 above)
            if(intensity > dither_table[x&7][y&7])                          //   Then, if dither says to color white
              info.data[x/8] |= 1 << (x%8);                                 //     Then Color pixel White
            else                                                            //   Else
              info.data[x/8] &= ~(1 << (x%8));                              //     Color pixel Black
          }
        #elif defined(PBL_COLOR)                                            // Verified works on Basalt and Chalk
          if(info.data[x]==replacement_color.argb                           // If pixel color matches (doesn't ignore alpha)
             || !replacement_color.argb) {                                  //   or if replacement_color = 0 (see Note2 above)
            uint8_t d = dither_table[x&7][y&7];                             // Speed things up slightly
            info.data[x] = 0b11000000 +                                     // Replace screen pixel with dither: Alpha part
              ((64*(r%85)/85 > d ? r/85+1 : r/85)<<4) +                     //   Red part
              ((64*(g%85)/85 > d ? g/85+1 : g/85)<<2) +                     //   Green part
              ((64*(b%85)/85 > d ? b/85+1 : b/85)   );                      //   Blue part
          }
        #endif
      }
    }
    graphics_release_frame_buffer(ctx, framebuffer);  // Release the Kraken!  ... err, framebuffer.
  }
}


void fill_rect_dithered(GContext *ctx, GRect rect, uint8_t r, uint8_t g, uint8_t b) {
  replace_color_in_rect_with_dithered(ctx, rect, GColorClear, r, g, b);
}


void replace_color_with_dithered(GContext *ctx, GColor replacement_color, uint8_t r, uint8_t g, uint8_t b) {
  replace_color_in_rect_with_dithered(ctx, GRect(0, 0, 1000, 1000), replacement_color, r, g, b);
}
