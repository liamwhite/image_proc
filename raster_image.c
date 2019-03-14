#include "raster_image.h"

#include <string.h>
#include <magick/api.h>

struct raster_image {
    Image *image;
    ImageInfo *info;
    size_t frames;
    dim_t dimensions;
};

// Returns a new raster_image pointer if this buffer was
// successfully loaded, or NULL if it failed to load.
raster_image *raster_image_from_buffer(const void *buf, size_t len)
{
    ExceptionInfo ex;

    raster_image *ri = (raster_image *) calloc(1, sizeof(raster_image));
    if (!ri)
        goto error;

    // Set up exception handling
    GetExceptionInfo(&ex);

    ri->info = CloneImageInfo(NULL);
    if (!ri->info)
        goto error;

    ri->image = BlobToImage(ri->info, buf, len, &ex);
    if (!ri->image)
        goto error;

    ri->dimensions.width  = ri->image->columns;
    ri->dimensions.height = ri->image->rows;
    ri->frames = GetImageListLength(ri->image);

    DestroyExceptionInfo(&ex);
    return ri;

error:
    raster_image_free(ri);
    DestroyExceptionInfo(&ex);
    return NULL;
}

// Returns a new raster_image pointer if this file was
// successfully loaded, or NULL if it failed to load.
raster_image *raster_image_from_file(const char *filename)
{
    ExceptionInfo ex;

    raster_image *ri = (raster_image *) calloc(1, sizeof(raster_image));
    if (!ri)
        goto error;

    if (strlen(filename) >= MaxTextExtent)
        goto error;

    // Set up exception handling
    GetExceptionInfo(&ex);

    ri->info = CloneImageInfo(NULL);
    if (!ri->info)
        goto error;

    // Stupid GM max string length
    strncpy(ri->info->filename, filename, MaxTextExtent);

    ri->image = ReadImage(ri->info, &ex);
    if (!ri->image)
        goto error;

    ri->dimensions.width  = ri->image->columns;
    ri->dimensions.height = ri->image->rows;
    ri->frames = GetImageListLength(ri->image);

    DestroyExceptionInfo(&ex);
    return ri;

error:
    raster_image_free(ri);
    DestroyExceptionInfo(&ex);
    return NULL;
}

// Frees this raster_image.
void raster_image_free(raster_image *ri)
{
    if (!ri)
        return;

    if (ri->image) DestroyImage(ri->image);
    if (ri->info) DestroyImageInfo(ri->info);

    free(ri);
}

// Gets the dimensions of this raster_image.
dim_t raster_image_dimensions(raster_image *ri)
{
    return ri->dimensions;
}

// Gets the frame count of this raster_image.
// May be more than 1 for APNG or GIF.
size_t raster_image_frame_count(raster_image *ri)
{
    return ri->frames;
}

static intensity_t internal_intensities_get(Image *frame); // XXX

// Gets corner intensities for this raster_image.
// This takes the median frame for APNG/GIF.
intensity_t raster_image_get_intensities(raster_image *ri)
{
    Image *frame = ri->image;

    // Go to median frame
    for (size_t i = 0; i < ri->frames / 2; ++i)
        frame = frame->next;

    return internal_intensities_get(frame);
}

// Scale this raster_image proportionally to either a height of max_h,
// or a width of max_w, whichever is greater. This preserves any animation
// behavior in the image.
raster_image *raster_image_scale(raster_image *ri, size_t max_w, size_t max_h)
{
    ExceptionInfo ex;
    Image *frame = ri->image;

    raster_image *si = (raster_image *) calloc(1, sizeof(raster_image));
    if (!si)
        goto error;

    si->info  = CloneImageInfo(si->info);
    if (!si->info)
        goto error;

    si->image = NewImageList();
    if (!si->image)
        goto error;

    while (frame) {
        // XXX
        Image *scaled = ResizeImage(frame, 500, 500, LanczosFilter, 1.0, &ex);
        if (!scaled)
            goto error;

        AppendImageToList(&ri->image, scaled);

        frame = frame->next;
    }

    DestroyExceptionInfo(&ex);
    return si;

error:
    raster_image_free(si);
    DestroyExceptionInfo(&ex);
    return NULL;
}

// Write this raster_image to a file. Can return a standard errno.
int raster_image_write_file(raster_image *ri, const char *filename);

// Try to optimize this file. Returns 1 if the optimization reduced the size
// of the file and 0 otherwise.
int raster_image_optimize(raster_image *ri);
