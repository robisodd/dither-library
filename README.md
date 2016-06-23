# dither-library
Color dithering library for Pebble


`fill_rect_dithered(ctx, rect, r, g, b)`  
Input: A graphics context, a rect (absolute screen coordinates), and a 24bit RGB color  
Draws a rectangle filled with a dithered (patterned) color  
Note: uses absolute screen coordinates, unlike graphics_fill_rect() which draws relative to its layer  


`replace_color_in_rect_with_dithered(ctx, rect, replacement_color, r, g, b)`  
Input: A graphics context, a rect (absolute screen coordinates), a color to look for and a 24bit RGB color to replace it with  
Searches with RECT for REPLACEMENT_COLOR and replaces it with the dithered 24bit RGB  
Note: RECT uses absolute screen coordinates, unlike graphics_fill_rect() which draws relative to its layer


`replace_color_with_dithered(ctx, replacement_color, r, g, b)`  
Input: A graphcs context, a color to look for, and a 24bit RGB dithered color to replace it with  
Searches the ENTIRE SCREEN for REPLACEMENT_COLOR and replaces it with the dithered 24bit RGB  
