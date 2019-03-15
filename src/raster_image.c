#include "raster_image.h"

#include <string.h>
#include <magick/api.h>

#define MIN(x,y) ((x) < (y) ? (x) : (y))

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

    ri->frames = GetImageListLength(ri->image);
    if (ri->frames == 1) {
        // Automatically orient the image
        Image *rotated = ri->image;
        ri->image = AutoOrientImage(rotated, rotated->orientation, &ex);
        DestroyImageList(rotated);
        if (!ri->image)
            goto error;

        StripImage(ri->image);

        ri->dimensions.width  = ri->image->columns;
        ri->dimensions.height = ri->image->rows;
    } else {
        ri->dimensions.width  = ri->image->page.width;
        ri->dimensions.height = ri->image->page.height;
    }

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

    ri->frames = GetImageListLength(ri->image);
    if (ri->frames == 1) {
        // Automatically orient the image
        Image *rotated = ri->image;
        ri->image = AutoOrientImage(rotated, rotated->orientation, &ex);
        DestroyImageList(rotated);
        if (!ri->image)
            goto error;

        StripImage(ri->image);

        ri->dimensions.width  = ri->image->columns;
        ri->dimensions.height = ri->image->rows;
    } else {
        ri->dimensions.width  = ri->image->page.width;
        ri->dimensions.height = ri->image->page.height;
    }

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

    if (ri->image)
      DestroyImageList(ri->image);

    if (ri->info)
      DestroyImageInfo(ri->info);

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

static MagickPassFail pixel_iterator(
    void *dat,
    const void *dontcare1,
    const Image *dontcare2,
    const PixelPacket *pixels,
    const IndexPacket *dontcare3,
    const long npixels,
    ExceptionInfo *dontcare4
)
{
    rect_sum_t *mem = (rect_sum_t *) dat;
    rect_sum_t curr = *mem;

    for (long i = 0; i < npixels; ++i) {
        PixelPacket p = pixels[i];

        curr.r += p.red / (sizeof(Quantum) * 128);
        curr.g += p.green / (sizeof(Quantum) * 128);
        curr.b += p.blue / (sizeof(Quantum) * 128);
    }

    *mem = curr;

    return MagickPass;
}

static rect_sum_t internal_intensities_get(Image *frame, rect_t bounds)
{
    // Empty sum is defined to be 0
    rect_sum_t curr = { 0 };

    ExceptionInfo ex;
    GetExceptionInfo(&ex);

    PixelIterateMonoRead(
        &pixel_iterator,
        NULL,
        "Intensity sum",
        &curr,
        NULL,
        bounds.start_x,
        bounds.start_y,
        bounds.end_x - bounds.start_x,
        bounds.end_y - bounds.start_y,
        frame,
        &ex
    );

    DestroyExceptionInfo(&ex);

    return curr;
}

static float sum_intensity(rect_sum_t sum, uint32_t npixels)
{
    return ((sum.b / npixels) * 0.2126 +
            (sum.g / npixels) * 0.7152 +
            (sum.r / npixels) * 0.0772) / 3;
}

static Image *get_coalesced(Image *in)
{
    ExceptionInfo ex;
    GetExceptionInfo(&ex);
    Image *out = CoalesceImages(in, &ex);
    DestroyExceptionInfo(&ex);

    return out;
}

// Gets corner intensities for this raster_image.
// This takes the median frame for APNG/GIF.
intensity_t raster_image_get_intensities(raster_image *ri)
{
    Image *frame = ri->image;
    int dispose = 0;

    if (ri->frames > 1) {
        frame = get_coalesced(frame);

        if (!frame)
            frame = ri->image;
        else
            dispose = 1;

        // Go to median frame
        for (size_t i = 0; i < ri->frames / 2; ++i)
            frame = frame->next;
    }

    uint32_t w = ri->dimensions.width;
    uint32_t h = ri->dimensions.height;

    rect_sum_t ins[] = {
        internal_intensities_get(frame, (rect_t) { 0, 0, w/2, h/2 }), // nw
        internal_intensities_get(frame, (rect_t) { w/2, 0, w, h/2 }), // ne
        internal_intensities_get(frame, (rect_t) { 0, h/2, w/2, h }), // sw
        internal_intensities_get(frame, (rect_t) { w/2, h/2, w, h }), // se
        internal_intensities_get(frame, (rect_t) { 0, 0, w, h })      // avg
    };

    if (dispose)
        DestroyImageList(frame);

    return (intensity_t) {
        .nw  = sum_intensity(ins[0], w*h/4),
        .ne  = sum_intensity(ins[1], w*h/4),
        .sw  = sum_intensity(ins[2], w*h/4),
        .se  = sum_intensity(ins[3], w*h/4),
        .avg = sum_intensity(ins[4], w*h)
    };
}

// Scale this raster_image proportionally to either a height of max_h,
// or a width of max_w, whichever is lesser. This preserves any animation
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

    // Set up exception handling
    GetExceptionInfo(&ex);

    double ratio = MIN((double) max_w / ri->dimensions.width, (double) max_h / ri->dimensions.height);

    // Doesn't matter if we don't coalesce here, but everything
    // must be scaled evenly (including page offsets)
    while (frame) {
        uint32_t new_w = frame->columns * ratio;
        uint32_t new_h = frame->rows * ratio;

        RectangleInfo new_tile = frame->tile_info;
        new_tile.width  *= ratio;
        new_tile.height *= ratio;
        new_tile.x      *= ratio;
        new_tile.y      *= ratio;

        Image *scaled = ResizeImage(frame, new_w, new_h, LanczosFilter, 1.0, &ex);
        if (!scaled)
            goto error;

        scaled->tile_info = new_tile;

        AppendImageToList(&si->image, scaled);

        frame = frame->next;
    }

    si->frames = GetImageListLength(ri->image);//ri->frames;
    si->dimensions.width  = ri->dimensions.width * ratio;
    si->dimensions.height = ri->dimensions.height * ratio;

    DestroyExceptionInfo(&ex);
    return si;

error:
    raster_image_free(si);
    DestroyExceptionInfo(&ex);
    return NULL;
}

// Write this raster_image to a file. May return an error code.
int raster_image_write_file(raster_image *ri, const char *filename)
{
    ExceptionInfo ex;

    // Set up exception handling
    GetExceptionInfo(&ex);

    int ret = ImageToFile(ri->image, filename, &ex);

    DestroyExceptionInfo(&ex);

    return ret;
}

// Try to optimize this file. Returns 1 if the optimization reduced the size
// of the file and 0 otherwise.
int raster_image_optimize(raster_image *ri)
{
    return 0;
}
