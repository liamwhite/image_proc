#ifndef _RASTER_IMAGE_H
#define _RASTER_IMAGE_H

#include "common.h"

typedef struct raster_image raster_image;

// Returns a new raster_image pointer if this buffer was
// successfully loaded, or NULL if it failed to load.
raster_image *raster_image_from_buffer(const void *buf, size_t len);

// Returns a new raster_image pointer if this file was
// successfully loaded, or NULL if it failed to load.
raster_image *raster_image_from_file(const char *filename);

// Invalidates and frees this raster_image.
void raster_image_free(raster_image *ri);

// Gets the dimensions of this raster_image.
dim_t raster_image_dimensions(raster_image *ri);

// Gets the frame count of this raster_image.
// May be more than 1 for APNG or GIF.
size_t raster_image_frame_count(raster_image *ri);

// Gets corner intensities for this raster_image.
// This takes the median frame for APNG/GIF.
intensity_t raster_image_get_intensities(raster_image *ri);

// Scale this raster_image proportionally to either a height of max_h,
// or a width of max_w, whichever is lesser. This preserves any animation
// behavior in the image.
raster_image *raster_image_scale(raster_image *ri, size_t max_w, size_t max_h);

// Write this raster_image to memory. You must free() the returned memory.
buf_t raster_image_to_buffer(raster_image *ri);

// Write this raster_image to a file.
int raster_image_to_file(raster_image *ri, const char *filename);

int raster_image_optimize(raster_image *ri);

#endif // _RASTER_IMAGE_H
