# dither-library
Color dithering library for Pebble

##Screenshots  
Aplite:  
![](Aplite.png)  

Basalt:  
![](Basalt-Color.png) ![](BasaltGrayscale.png)  

Chalk:  
![](ChalkColor.png) ![](ChalkGrayscale.png)  

####`fill_rect_dithered(ctx, rect, r, g, b)`  
Given a graphics context, a rect (absolute screen coordinates), and a 24bit RGB color, draws a rectangle filled with a dithered (patterned) 6-bit color.  Note that rect uses absolute screen coordinates, unlike `graphics_fill_rect()` which draws relative to its layer.


####`replace_color_in_rect_with_dithered(ctx, rect, replacement_color, r, g, b)`  
Given a graphics context, a rect, a color to look for and a 24bit RGB color to replace it with, searches within the `rect` for `replacement_color` and replaces it with a 6bit color dithered from the 24bit RGB given.  Note that `rect` uses absolute screen coordinates, unlike normal Pebble API functions (e.g. `graphics_fill_rect()`) which draw relative to their layer.


####`replace_color_with_dithered(ctx, replacement_color, r, g, b)`  
Given a graphcs context, a color to look for, and a 24bit RGB dithered color to replace it with, searches the ENTIRE SCREEN for `replacement_color` and replaces it with a 6bit color dithered from the 24bit RGB given.  Note that it currently doesn't ignore the `alpha` channel, though screen pixels are almost always `alpha = 3`.
