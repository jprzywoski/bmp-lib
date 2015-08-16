#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


typedef struct {
    unsigned short int type;               // 0  2 the header field used to identify the BMP & DIB file is 0x42 0x4D in hexadecimal, same as BM in ASCII.
    unsigned int bitmap_size;              // 2  4 the size of the BMP file in bytes
    unsigned short int reserved1;          // 6  2 reserved; actual value depends on the application that creates the image
    unsigned short int reserved2;          // 8  2 reserved; actual value depends on the application that creates the image
    unsigned int bitmap_offset;            // 10 4 the offset, i.e. starting address, of the byte where the bitmap image data (pixel array) can be found.
} bmp_file_header_t;


typedef struct {
    unsigned int header_size;              // 14 4 the size of this header (40 bytes)
    unsigned int width;                    // 18 4 the bitmap width in pixels (signed integer)
    unsigned int height;                   // 22 4 the bitmap height in pixels (signed integer)
    unsigned short int planes;             // 26 2 the number of color planes must be 1
    unsigned short int bits_per_pixel;     // 28 2 the number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16, 24 and 32.
    unsigned int compression;              // 30 4 the compression method being used. See the next table for a list of possible values
    unsigned int image_size;               // 34 4 the image size. This is the size of the raw bitmap data; a dummy 0 can be given for BI_RGB bitmaps.
    int x_resolution;                      // 38 4 the horizontal resolution of the image. (pixel per meter, signed integer)
    int y_resolution;                      // 42 4 the vertical resolution of the image. (pixel per meter, signed integer)
    unsigned int colors;                   // 46 4 the number of colors in the color palette, or 0 to default to
    unsigned int important_colors;         // 50 4 the number of important colors used, or 0 when every color is important; generally ignored
} bmp_bitmap_info_header_t;


typedef struct {
    bmp_file_header_t header;
    bmp_bitmap_info_header_t info;
    unsigned char **data;
} bmp_t;


unsigned int get_row_size(bmp_t *bmp)
{
    unsigned int row_size;
    row_size = ((bmp->info.bits_per_pixel * bmp->info.width + 31) / 32) * 4;
    return row_size;
}

unsigned int get_pixel_array_size(bmp_t *bmp)
{;
    return get_row_size(bmp) * bmp->info.height;
}

bmp_t *bmp_load(const char *path)
{
    unsigned int row_size;
    unsigned int pixel_array_size;
    FILE* f;
    unsigned int i;
    bmp_t *bmp = malloc(sizeof(bmp_t));

    f = fopen(path, "rb");
    if (f == NULL) {
        perror("fopen");
        return NULL;
    }

    // read header data
    fread(&bmp->header.type, sizeof(unsigned short int), 1, f);
    // check if the file is indeed a bitmap
    if (bmp->header.type != 19778) {
        printf("Invalid file format: %s\n", path);
        return NULL;
    }
    fread(&bmp->header.bitmap_size, sizeof(unsigned int), 1, f);
    fread(&bmp->header.reserved1, sizeof(unsigned short int), 1, f);
    fread(&bmp->header.reserved2, sizeof(unsigned short int), 1, f);
    fread(&bmp->header.bitmap_offset, sizeof(unsigned int), 1, f);

    // read info
    fread(&bmp->info.header_size, sizeof(unsigned int), 1, f);
    fread(&bmp->info.width, sizeof(unsigned int), 1, f);
    fread(&bmp->info.height, sizeof(unsigned int), 1, f);
    fread(&bmp->info.planes, sizeof(unsigned short int), 1, f);
    fread(&bmp->info.bits_per_pixel, sizeof(unsigned short int), 1, f);
    if (bmp->info.bits_per_pixel != 24) {
        printf("Invalid file format: %s\n", path);
        return NULL;
    }
    fread(&bmp->info.compression, sizeof(unsigned int), 1, f);
    fread(&bmp->info.image_size, sizeof(unsigned int), 1, f);
    fread(&bmp->info.x_resolution, sizeof(unsigned int), 1, f);
    fread(&bmp->info.y_resolution, sizeof(unsigned int), 1, f);
    fread(&bmp->info.colors, sizeof(unsigned int), 1, f);
    fread(&bmp->info.important_colors, sizeof(unsigned int), 1, f);

    // allocate pixel data array
    row_size = get_row_size(bmp);
    pixel_array_size = get_pixel_array_size(bmp);
    bmp->data = malloc(bmp->info.height * sizeof(unsigned char *));
    bmp->data[0] = malloc(pixel_array_size * sizeof(unsigned char));
    if (bmp->data == NULL) {
        perror("malloc");
        return NULL;
    }
    // write addresses of row_sized chunks
    for (i = 0; i < bmp->info.height; i++) {
        bmp->data[i] = (*bmp->data + row_size * i);
    }

    // read pixel data; pixel format: [b g r] ...
    fread(bmp->data[0], sizeof(char), pixel_array_size, f);

    if (fclose(f) == EOF) {
        perror("fclose");
        return NULL;
    }
    return bmp;
}

int bmp_write(bmp_t *bmp, const char *path)
{
    FILE *f;

    f = fopen(path, "wb");
    if (f == NULL) {
        perror("fopen");
        return 1;
    }

    // header dump
    if (0 > (int)fwrite(&bmp->header.type, sizeof(unsigned short int), 1, f)) {
        perror("fwrite");
        return 1;
    }
    fwrite(&bmp->header.bitmap_size, sizeof(unsigned int), 1, f);
    fwrite(&bmp->header.reserved1, sizeof(unsigned short int), 1, f);
    fwrite(&bmp->header.reserved2, sizeof(unsigned short int), 1, f);
    fwrite(&bmp->header.bitmap_offset, sizeof(unsigned int), 1, f);

    // info dump
    if (0 > (int)fwrite(&bmp->info.header_size, sizeof(unsigned int), 1, f)) {
        perror("fwrite");
        return 1;
    }
    fwrite(&bmp->info.width, sizeof(unsigned int), 1, f);
    fwrite(&bmp->info.height, sizeof(unsigned int), 1, f);
    fwrite(&bmp->info.planes, sizeof(unsigned short int), 1, f);
    fwrite(&bmp->info.bits_per_pixel, sizeof(unsigned short int), 1, f);
    fwrite(&bmp->info.compression, sizeof(unsigned int), 1, f);
    fwrite(&bmp->info.image_size, sizeof(unsigned int), 1, f);
    fwrite(&bmp->info.x_resolution, sizeof(unsigned int), 1, f);
    fwrite(&bmp->info.y_resolution, sizeof(unsigned int), 1, f);
    fwrite(&bmp->info.colors, sizeof(unsigned int), 1, f);
    fwrite(&bmp->info.important_colors, sizeof(unsigned int), 1, f);

    // pixels dump
    if (0 > (int)fwrite(bmp->data[0], sizeof(char), get_pixel_array_size(bmp), f)) {
        perror("fwrite");
        return 1;
    }

    if (fclose(f) == EOF) {
        perror("fclose");
        return 1;
    }

    return 0;
}

int bmp_destroy(bmp_t *bmp)
{
    free(bmp->data[0]);
    free(bmp->data);
    free(bmp);
    return 0;
}

bmp_t *bmp_brightness(bmp_t *bmp, int step)
{
    unsigned int row_size = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;
    int d;

    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < row_size; x++) {
            d = (int)bmp->data[y][x] + step;
            if (d > 255) {
                d = 255;
            } else if (d < 0) {
                d = 0;
            }
            bmp->data[y][x] = d;
        }
    }
    return bmp;
}

bmp_t *bmp_invert(bmp_t *bmp)
{
    unsigned int row_size = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;

    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < row_size; x++) {
            bmp->data[y][x] = (unsigned char)(255 - bmp->data[y][x]);
        }
    }
    return bmp;
}

bmp_t *bmp_grayscale(bmp_t *bmp)
{
    unsigned int row_size = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;
    float r;
    float g;
    float b;
    float gray;

    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < row_size; x+=3) {
            b = 0.07 * (float)bmp->data[y][x];
            g = 0.72 * (float)bmp->data[y][x+1];
            r = 0.21 * (float)bmp->data[y][x+2];
            gray = (r + g + b) / 3.0;
            bmp->data[y][x] = bmp->data[y][x+1] = bmp->data[y][x+2] = (unsigned char)gray;
        }
    }
    return bmp;
}

bmp_t *bmp_remove_channel(bmp_t *bmp, const char channel)
{
    unsigned int row_size = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;

    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < row_size; x+=3) {
            switch (channel) {
                case 'b':
                    bmp->data[y][x] = 0;
                    break;
                case 'g':
                    bmp->data[y][x+1] = 0;
                    break;
                case 'r':
                    bmp->data[y][x+2] = 0;
                    break;
            }
        }
    }
    return bmp;
}

bmp_t *bmp_swap_channel(bmp_t *bmp, const char channel, const char other)
{
    unsigned int row_size = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;

    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < row_size; x+=3) {
            switch (channel) {
                case 'b':
                    switch (other) {
                        case 'g':
                            bmp->data[y][x] = bmp->data[y][x+1];
                            break;
                        case 'r':
                            bmp->data[y][x] = bmp->data[y][x+2];
                            break;
                    }
                    break;
                case 'g':
                    switch (other) {
                        case 'b':
                            bmp->data[y][x+1] = bmp->data[y][x];
                            break;
                        case 'r':
                            bmp->data[y][x+1] = bmp->data[y][x+2];
                            break;
                    }
                    break;
                case 'r':
                    switch (other) {
                        case 'b':
                            bmp->data[y][x+2] = bmp->data[y][x];
                            break;
                        case 'g':
                            bmp->data[y][x+2] = bmp->data[y][x+1];
                            break;
                    }
                    break;
            }
        }
    }
    return bmp;
}

bmp_t *bmp_add(bmp_t *bmp, const bmp_t *other)
{
    unsigned int row_size = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;
    int d;

    assert(bmp->info.height == other->info.height);
    assert(bmp->info.width == other->info.width);

    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < row_size; x++) {
            d = (int)bmp->data[y][x] + (int)other->data[y][x];
            if (d > 255) {
                d = 255;
            }
            bmp->data[y][x] = (unsigned char)d;
        }
    }
    return bmp;
}

bmp_t *bmp_subtract(bmp_t *bmp, const bmp_t *other)
{
    unsigned int row_size = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;
    int d;

    assert(bmp->info.height == other->info.height);
    assert(bmp->info.width == other->info.width);

    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < row_size; x++) {
            d = (int)bmp->data[y][x] - (int)other->data[y][x];
            if (d < 0) {
                d = 0;
            }
            bmp->data[y][x] = (unsigned char)d;
        }
    }
    return bmp;
}

bmp_t *bmp_difference(bmp_t *bmp, const bmp_t *other)
{
    unsigned int row_size = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;
    int d;

    assert(bmp->info.height == other->info.height);
    assert(bmp->info.width == other->info.width);

    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < row_size; x++) {
            d = (int)bmp->data[y][x] - (int)other->data[y][x];
            d = (d < 0) ? -1 * d : d;
            bmp->data[y][x] = (unsigned char)d;
        }
    }
    return bmp;
}

bmp_t *bmp_multiply(bmp_t *bmp, const bmp_t *other)
{
    unsigned int row_size = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;

    assert(bmp->info.height == other->info.height);
    assert(bmp->info.width == other->info.width);

    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < row_size; x++) {
            bmp->data[y][x] = (unsigned char)(255 * ((float)bmp->data[y][x] / 255.0 * (float)other->data[y][x] / 255.0));
        }
    }
    return bmp;
}

bmp_t *bmp_average(bmp_t *bmp, const bmp_t *other)
{
    unsigned int row_size = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;

    assert(bmp->info.height == other->info.height);
    assert(bmp->info.width == other->info.width);

    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < row_size; x++) {
            bmp->data[y][x] = (unsigned char)(((int)bmp->data[y][x] + (int)other->data[y][x]) / 2);
        }
    }
    return bmp;
}

bmp_t *bmp_min(bmp_t *bmp, const bmp_t *other)
{
    unsigned int row_size = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;

    assert(bmp->info.height == other->info.height);
    assert(bmp->info.width == other->info.width);

    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < row_size; x++) {
            if (bmp->data[y][x] > other->data[y][x]) {
                bmp->data[y][x] = other->data[y][x];
            }
        }
    }
    return bmp;
}

bmp_t *bmp_max(bmp_t *bmp, const bmp_t *other)
{
    unsigned int row_size = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;

    assert(bmp->info.height == other->info.height);
    assert(bmp->info.width == other->info.width);

    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < row_size; x++) {
            if (bmp->data[y][x] < other->data[y][x]) {
                bmp->data[y][x] = other->data[y][x];
            }
        }
    }
    return bmp;
}

bmp_t *bmp_blur(bmp_t *bmp)
{
    unsigned int width = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;
    float filter[3][3] = {
        {0.0, 0.2, 0.0},
        {0.2, 0.2, 0.2},
        {0.0, 0.2, 0.0},
    };
    unsigned int fx;
    unsigned int fy;
    float r;
    float g;
    float b;
    int iy;
    int ix;
    unsigned int pixel_array_size;
    unsigned int row_size;
    unsigned char **temp;
    unsigned int i;

    // allocate pixel data array
    row_size = get_row_size(bmp);
    pixel_array_size = get_pixel_array_size(bmp);
    temp = malloc(bmp->info.height * sizeof(unsigned char *));
    if (temp == NULL) {
        perror("malloc");
        return NULL;
    }
    temp[0] = malloc(pixel_array_size * sizeof(unsigned char));
    if (temp[0] == NULL) {
        perror("malloc");
        return NULL;
    }
    // write addresses of row_sized chunks
    for (i = 0; i < bmp->info.height; i++) {
        temp[i] = (temp[0] + row_size * i);
    }

    // blurring the image
    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < width; x+=3) {
            b = g = r = 0.0;
            for (fy = 0; fy < 3; fy++) {
                iy = (y - 3 / 2 + fy + bmp->info.height) % bmp->info.height;
                for (fx = 0; fx < 3; fx++) {
                    ix = (width + x - 3 + 3 * fx) % width;
                    b += bmp->data[iy][ix] * filter[fy][fx];
                    g += bmp->data[iy][ix+1] * filter[fy][fx];
                    r += bmp->data[iy][ix+2] * filter[fy][fx];
                }
            }
            if (b < 0.0) b = 0.0; else if (b > 255.0) b = 255.0;
            if (g < 0.0) g = 0.0; else if (g > 255.0) g = 255.0;
            if (r < 0.0) r = 0.0; else if (r > 255.0) r = 255.0;
            temp[y][x] = (unsigned char)b;
            temp[y][x+1] = (unsigned char)g;
            temp[y][x+2] = (unsigned char)r;
        }
    }

    free(bmp->data[0]);
    free(bmp->data);
    bmp->data = temp;
    return bmp;
}

bmp_t *bmp_edges(bmp_t *bmp)
{
    unsigned int width = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;
    float filter[3][3] = {
        {-1.0, -1.0, -1.0},
        {-1.0,  8.0, -1.0},
        {-1.0, -1.0, -1.0},
    };
    unsigned int fx;
    unsigned int fy;
    float r;
    float g;
    float b;
    int iy;
    int ix;
    unsigned int pixel_array_size;
    unsigned int row_size;
    unsigned char **temp;
    unsigned int i;

    // allocate pixel data array
    row_size = get_row_size(bmp);
    pixel_array_size = get_pixel_array_size(bmp);
    temp = malloc(bmp->info.height * sizeof(unsigned char *));
    if (temp == NULL) {
        perror("malloc");
        return NULL;
    }
    temp[0] = malloc(pixel_array_size * sizeof(unsigned char));
    if (temp[0] == NULL) {
        perror("malloc");
        return NULL;
    }
    // write addresses of row_sized chunks
    for (i = 0; i < bmp->info.height; i++) {
        temp[i] = (temp[0] + row_size * i);
    }

    // blurring the image
    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < width; x+=3) {
            b = g = r = 0.0;
            for (fy = 0; fy < 3; fy++) {
                iy = (y - 3 / 2 + fy + bmp->info.height) % bmp->info.height;
                for (fx = 0; fx < 3; fx++) {
                    ix = (width + x - 3 + 3 * fx) % width;
                    b += bmp->data[iy][ix] * filter[fy][fx];
                    g += bmp->data[iy][ix+1] * filter[fy][fx];
                    r += bmp->data[iy][ix+2] * filter[fy][fx];
                }
            }
            if (b < 0.0) b = 0.0; else if (b > 255.0) b = 255.0;
            if (g < 0.0) g = 0.0; else if (g > 255.0) g = 255.0;
            if (r < 0.0) r = 0.0; else if (r > 255.0) r = 255.0;
            temp[y][x] = (unsigned char)b;
            temp[y][x+1] = (unsigned char)g;
            temp[y][x+2] = (unsigned char)r;
        }
    }

    free(bmp->data[0]);
    free(bmp->data);
    bmp->data = temp;
    return bmp;
}

bmp_t *bmp_sharpen(bmp_t *bmp)
{
    unsigned int width = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;
    float filter[3][3] = {
        {-1.0, -1.0, -1.0},
        {-1.0,  9.0, -1.0},
        {-1.0, -1.0, -1.0},
    };
    unsigned int fx;
    unsigned int fy;
    float r;
    float g;
    float b;
    int iy;
    int ix;
    unsigned int pixel_array_size;
    unsigned int row_size;
    unsigned char **temp;
    unsigned int i;

    // allocate pixel data array
    row_size = get_row_size(bmp);
    pixel_array_size = get_pixel_array_size(bmp);
    temp = malloc(bmp->info.height * sizeof(unsigned char *));
    if (temp == NULL) {
        perror("malloc");
        return NULL;
    }
    temp[0] = malloc(pixel_array_size * sizeof(unsigned char));
    if (temp[0] == NULL) {
        perror("malloc");
        return NULL;
    }
    // write addresses of row_sized chunks
    for (i = 0; i < bmp->info.height; i++) {
        temp[i] = (temp[0] + row_size * i);
    }

    // blurring the image
    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < width; x+=3) {
            b = g = r = 0.0;
            for (fy = 0; fy < 3; fy++) {
                iy = (y - 3 / 2 + fy + bmp->info.height) % bmp->info.height;
                for (fx = 0; fx < 3; fx++) {
                    ix = (width + x - 3 + 3 * fx) % width;
                    b += bmp->data[iy][ix] * filter[fy][fx];
                    g += bmp->data[iy][ix+1] * filter[fy][fx];
                    r += bmp->data[iy][ix+2] * filter[fy][fx];
                }
            }
            if (b < 0.0) b = 0.0; else if (b > 255.0) b = 255.0;
            if (g < 0.0) g = 0.0; else if (g > 255.0) g = 255.0;
            if (r < 0.0) r = 0.0; else if (r > 255.0) r = 255.0;
            temp[y][x] = (unsigned char)b;
            temp[y][x+1] = (unsigned char)g;
            temp[y][x+2] = (unsigned char)r;
        }
    }

    free(bmp->data[0]);
    free(bmp->data);
    bmp->data = temp;
    return bmp;
}

bmp_t *bmp_emboss(bmp_t *bmp)
{
    unsigned int width = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;
    float filter[3][3] = {
        {-1.0, -1.0,  0.0},
        {-1.0,  0.0,  1.0},
        {0.0,   1.0,  1.0},
    };
    unsigned int fx;
    unsigned int fy;
    float r;
    float g;
    float b;
    int iy;
    int ix;
    unsigned int pixel_array_size;
    unsigned int row_size;
    unsigned char **temp;
    unsigned int i;
    float bias = 128.0;

    // allocate pixel data array
    row_size = get_row_size(bmp);
    pixel_array_size = get_pixel_array_size(bmp);
    temp = malloc(bmp->info.height * sizeof(unsigned char *));
    if (temp == NULL) {
        perror("malloc");
        return NULL;
    }
    temp[0] = malloc(pixel_array_size * sizeof(unsigned char));
    if (temp[0] == NULL) {
        perror("malloc");
        return NULL;
    }
    // write addresses of row_sized chunks
    for (i = 0; i < bmp->info.height; i++) {
        temp[i] = (temp[0] + row_size * i);
    }

    // blurring the image
    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < width; x+=3) {
            b = g = r = 0.0;
            for (fy = 0; fy < 3; fy++) {
                iy = (y - 3 / 2 + fy + bmp->info.height) % bmp->info.height;
                for (fx = 0; fx < 3; fx++) {
                    ix = (width + x - 3 + 3 * fx) % width;
                    b += bmp->data[iy][ix] * filter[fy][fx];
                    g += bmp->data[iy][ix+1] * filter[fy][fx];
                    r += bmp->data[iy][ix+2] * filter[fy][fx];
                }
            }
            if (b + bias < 0.0) b = 0.0; else if (b + bias > 255.0) b = 255.0;
            if (g + bias < 0.0) g = 0.0; else if (g + bias > 255.0) g = 255.0;
            if (r + bias < 0.0) r = 0.0; else if (r + bias > 255.0) r = 255.0;
            temp[y][x] = (unsigned char)b;
            temp[y][x+1] = (unsigned char)g;
            temp[y][x+2] = (unsigned char)r;
        }
    }

    free(bmp->data[0]);
    free(bmp->data);
    bmp->data = temp;
    return bmp;
}

bmp_t *bmp_mean(bmp_t *bmp)
{
    unsigned int width = (bmp->info.width * bmp->info.bits_per_pixel) / 8;
    unsigned int x;
    unsigned int y;
    float filter[3][3] = {
        {0.1111, 0.1111, 0.1111},
        {0.1111, 0.1111, 0.1111},
        {0.1111, 0.1111, 0.1111},
    };
    unsigned int fx;
    unsigned int fy;
    float r;
    float g;
    float b;
    int iy;
    int ix;
    unsigned int pixel_array_size;
    unsigned int row_size;
    unsigned char **temp;
    unsigned int i;

    // allocate pixel data array
    row_size = get_row_size(bmp);
    pixel_array_size = get_pixel_array_size(bmp);
    temp = malloc(bmp->info.height * sizeof(unsigned char *));
    if (temp == NULL) {
        perror("malloc");
        return NULL;
    }
    temp[0] = malloc(pixel_array_size * sizeof(unsigned char));
    if (temp[0] == NULL) {
        perror("malloc");
        return NULL;
    }
    // write addresses of row_sized chunks
    for (i = 0; i < bmp->info.height; i++) {
        temp[i] = (temp[0] + row_size * i);
    }

    // blurring the image
    for (y = 0; y < bmp->info.height; y++) {
        for (x = 0; x < width; x+=3) {
            b = g = r = 0.0;
            for (fy = 0; fy < 3; fy++) {
                iy = (y - 3 / 2 + fy + bmp->info.height) % bmp->info.height;
                for (fx = 0; fx < 3; fx++) {
                    ix = (width + x - 3 + 3 * fx) % width;
                    b += bmp->data[iy][ix] * filter[fy][fx];
                    g += bmp->data[iy][ix+1] * filter[fy][fx];
                    r += bmp->data[iy][ix+2] * filter[fy][fx];
                }
            }
            if (b < 0.0) b = 0.0; else if (b > 255.0) b = 255.0;
            if (g < 0.0) g = 0.0; else if (g > 255.0) g = 255.0;
            if (r < 0.0) r = 0.0; else if (r > 255.0) r = 255.0;
            temp[y][x] = (unsigned char)b;
            temp[y][x+1] = (unsigned char)g;
            temp[y][x+2] = (unsigned char)r;
        }
    }

    free(bmp->data[0]);
    free(bmp->data);
    bmp->data = temp;
    return bmp;
}

