#include <pebble.h>
#include "dither.h"

// Global variables for the demo part
static Window *main_window;
static Layer *dither_layer, *text_layer;
uint8_t color[3] = {127, 127, 127};          // Start colors in the middle
enum {red, green, blue};                     // for color[] above
enum {gray};                                 // for color[] above, but just uses "red" channel

uint8_t grayscale = false;                   // Allows 2-bit grayscale on color displays (ignored on B&W)
                                             //   Honestly, all the code below would look better if I could
                                             //   come up with a word that conveys "the opposite of grayscale".
                                             //   Then I could set this to TRUE and get rid of all the inversions
                                             //   in the forms of "!grayscale" and "~grayscale"

uint8_t cursor = 0;                          // Position for RGB caret
uint8_t cursormax = PBL_IF_COLOR_ELSE(3, 1); // 3 cursor positions for color, just the 1 for B&W


// ------------------------------------------------------------------------------------------------------------------------------------------------ //
//  Layer Update Functions
// ------------------------------------------------------------------------ //
static void dither_layer_update(Layer *layer, GContext *ctx) {
  GRect frame = layer_get_frame(layer);
  #if(PBL_COLOR)
  if(!grayscale)
    fill_rect_dithered(ctx, frame, color[red], color[green], color[blue]);    // Fill the layer
  else
  #endif
    fill_rect_dithered(ctx, frame, color[gray], color[gray], color[gray]);    // Use channel 0 (normally red) for all 3 colors
}

// ------------------------------------------------------------------------------------------------------------------------------------------------ //

static void text_layer_update(Layer *layer, GContext *ctx) {
  char text[50];  //Buffer to hold text

  #if(PBL_COLOR)
    if(!grayscale)
      snprintf(text, sizeof(text), "%cr:%d\n%cg:%d\n%cb:%d", cursor==0 ?'>':' ', color[red], cursor==1?'>':' ', color[green], cursor==2?'>':' ', color[blue]);
    else 
  #endif
      snprintf(text, sizeof(text), "%d", color[gray]);
  
  
  GRect bounds = layer_get_bounds(layer);
  GFont font = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
  GSize text_size = graphics_text_layout_get_content_size(text, font, bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft); // Get how big the text is
  
  #if(PBL_COLOR)
    // Set up text box
    if (text_size.w<111) text_size.w = 111;  // Stop text from boppin' around
    GRect text_rect = GRect((bounds.size.w - text_size.w)/2, (bounds.size.h - text_size.h)/2, text_size.w, text_size.h);
  
    // Set Replacement Color
    // Making sure text color isn't anywhere else within the rect (to prevent accidental replacements)
    GColor replace = (color[red]>127 || color[green]>127 || color[blue]>127) ? GColorOxfordBlue : GColorCeleste;
      
    // Write Text in the "Color To Be Replaced"
    graphics_context_set_text_color(ctx, replace);
    graphics_draw_text(ctx, text, font, text_rect, GTextOverflowModeTrailingEllipsis, grayscale ? GTextAlignmentCenter : GTextAlignmentLeft, NULL);

    // Replace the color the text was drawn in (limiting to the text_rect region)
    if(!grayscale)
      replace_color_in_rect_with_dithered(ctx, text_rect, replace, 128+color[red], 128+color[green], 128+color[blue]);
    else
      replace_color_in_rect_with_dithered(ctx, text_rect, replace, 128+color[gray], 128+color[gray], 128+color[gray]);
  #else
    // Set up text box
    text_size.h += 10; text_size.w += 10;    // Add some nice padding on B&W
    GRect text_rect = GRect((bounds.size.w - text_size.w)/2, (bounds.size.h - text_size.h)/2, text_size.w, text_size.h);

    // So, since there's only 2 colors on B&W, replacement color looks kinda bad.  Adding a solid background
    graphics_context_set_fill_color(ctx, GColorBlack);  // Text Background Color
    graphics_fill_rect(ctx, text_rect, 0, GCornerNone); // Draw Background Rectangle
  
    // Write Text in the "Color To Be Replaced" (GColorWhite on this B&W example)
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, text, font, text_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

    // Replace the color (GColorWhite for example) the text was drawn in. Restricted to the text_rect region.
    replace_color_in_rect_with_dithered(ctx, text_rect, GColorWhite, 64+color[gray], 64+color[gray], 64+color[gray]);
  #endif
}



// ------------------------------------------------------------------------------------------------------------------------------------------------ //
//  Button Functions
// ------------------------------------------------------------------------ //
static void     up_single_click_handler(ClickRecognizerRef recognizer, void *context) {color[cursor & ~grayscale]++;      layer_mark_dirty(dither_layer);} // Increase value
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {cursor = (cursor + 1) % cursormax; layer_mark_dirty(dither_layer);} // Advance cursor (if color)
static void   select_long_click_handler(ClickRecognizerRef recognizer, void *context) {grayscale = ~grayscale;            layer_mark_dirty(dither_layer);} // Hold to toggle grayscale (on color Pebbles)
static void   down_single_click_handler(ClickRecognizerRef recognizer, void *context) {color[cursor & ~grayscale]--;      layer_mark_dirty(dither_layer);} // Decrease value


static void click_config_provider(void *context) {
  window_single_repeating_click_subscribe(BUTTON_ID_UP,    30,  up_single_click_handler);
            window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
              window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_click_handler, NULL);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN , 30, down_single_click_handler);
}



// ------------------------------------------------------------------------ //
//  Main Functions
// ------------------------------------------------------------------------ //
static void main_window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);
  window_set_background_color(main_window, GColorBlack);
  window_set_click_config_provider(main_window, click_config_provider);

  dither_layer = layer_create(layer_get_frame(root_layer));  // fullscreen
  // dither_layer = layer_create(GRect((layer_get_bounds(root_layer).size.w - 100) / 2,20,100,100)); // Non-fullscreen
  // dither_layer = layer_create(GRect( 100,20,100,100));   // testing out-of-bounds
  layer_add_child(root_layer, dither_layer);
  layer_set_update_proc(dither_layer, dither_layer_update);

  text_layer = layer_create(layer_get_frame(root_layer));
  layer_add_child(root_layer, text_layer);
  layer_set_update_proc(text_layer, text_layer_update);
}


static void main_window_unload(Window *window) {
  layer_destroy(text_layer);
  layer_destroy(dither_layer);
}


static void init() {
  main_window = window_create();
  window_set_window_handlers(main_window, (WindowHandlers) {.load = main_window_load, .unload = main_window_unload});
  window_stack_push(main_window, true);
}


static void deinit() {
  window_destroy(main_window);
}


int main(void) {
  init();
  app_event_loop();
  deinit();
}