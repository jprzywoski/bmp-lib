====
libwinbmp.h
====

Structures
====
`bmp_file_header_t`_
    Bitmap file info.
`bmp_bitmap_info_header_t`_
    Bitmap data info.
`bmp_t`_
    Bitmap structure.

Utility Functions
====
`bmp_t *bmp_load(const char *path)`_
    Loads bitmap file from the path into bmp_t structure.
`int bmp_write(bmp_t *bmp, const char *path)`_
    Writes in-memory bitmap to a file.
`void bmp_destroy(bmp_t *bmp)`_
    Deallocates memory taken up by the bitmap.
`unsigned int get_row_size(bmp_t *bmp)`_
    Calculates row size including 4-byte alignment padding.
`unsigned int get_pixel_array_size(bmp_t *bmp)`_
    Calculates pixel array size including 4-byte alignment padding.

Image Functions
====
Just a bunch of simple functions.

Histogram
----
`bmp_t *bmp_brightness(bmp_t *bmp, int step)`_
    Adjusts the brightness of the image.
`bmp_t *bmp_invert(bmp_t *bmp)`_
    Inverts the color values.
`bmp_t *bmp_grayscale(bmp_t *bmp)`_
    Converts the image into grayscale.
`bmp_t *bmp_remove_channel(bmp_t *bmp, const char channel)`_
    Removes selected rgb channel.
`bmp_t *bmp_swap_channel(bmp_t *bmp, const char channel, const char other)`_
    Swaps two channels.
    

Image Arithmetic
----
`bmp_t *bmp_add(bmp_t *bmp, const bmp_t *other)`_
    Adds two bitmaps.
`bmp_t *bmp_subtract(bmp_t *bmp, const bmp_t *other)`_
    Subtracts two bitmaps.
`bmp_t *bmp_difference(bmp_t *bmp, const bmp_t *other)`_
    Subtracts two bitmaps (absolute pixel distance is returned).
`bmp_t *bmp_multiply(bmp_t *bmp, const bmp_t *other)`_
    Multiplies two bitmaps.
`bmp_t *bmp_average(bmp_t *bmp, const bmp_t *other)`_
    Returns minimum of two pixels.
`bmp_t *bmp_min(bmp_t *bmp, const bmp_t *other)`_
    Returns maximum of two pixels.

Convolution Filters
----
`bmp_t *bmp_blur(bmp_t *bmp)`_
    Blurs the bitmap.
`bmp_t *bmp_edges(bmp_t *bmp)`_
    Detects the edges.
`bmp_t *bmp_sharpen(bmp_t *bmp)`_
    Sharpens the image.
`bmp_t *bmp_emboss(bmp_t *bmp)`_
    Creates emboss effect.
`bmp_t *bmp_mean(bmp_t *bmp)`_
    Mean blur filter.
    
Drawing
----
`unsigned char *bmp_get_pixel(bmp_t *bmp, const unsigned int x, const unsigned int y)`_
	Returns the blue-green-red pixel values at the specified point.
`void bmp_set_pixel(bmp_t *bmp, const unsigned int x, const unsigned int y, const unsigned int rgb)`_
	Sets the pixel at the specified point.
`bmp_t *bmp_line(bmp_t *bmp, const int x0, const int y0, const int x1, const int y1, const int rgb)`_
    Draws a line.
    