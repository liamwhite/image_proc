#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <magick/api.h>

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

#define DISPOSAL_UNSPECIFIED      0       /* No disposal specified. */
#define DISPOSE_DO_NOT            1       /* Leave image in place */
#define DISPOSE_BACKGROUND        2       /* Set area to background color */
#define DISPOSE_PREVIOUS          3       /* Restore to previous content */

typedef struct {
    uint32_t start_x;
    uint32_t start_y;
    uint32_t end_x;
    uint32_t end_y;

    int valid;
} aabb;

typedef struct {
    aabb box;
    long row;
} opt_info;

static void box_union(aabb *restrict box, long row, long col)
{
    if (box->valid) {
        box->start_x = MIN(box->start_x, col);
        box->start_y = MIN(box->start_y, row);
        box->end_x = MAX(box->end_x, col + 1);
        box->end_y = MAX(box->end_y, row + 1);
    } else {
        box->start_x = col;
        box->start_y = row;
        box->end_x = col + 1;
        box->end_y = row + 1;
        box->valid = 1;
    }
}

static MagickPassFail pixel_iterator(
    void *dat,
    const void *dontcare1,
    const Image *previous_image,
    const PixelPacket *prev_pixels,
    const IndexPacket *dontcare2,
    const Image *this_image,
    const PixelPacket *this_pixels,
    const IndexPacket *dontcare3,
    const long npixels,
    ExceptionInfo *dontcare4
)
{
    opt_info *p_info = (opt_info *) dat;
    aabb box = p_info->box;

    for (long i = 0; i < npixels; ++i) {
        PixelPacket prev = prev_pixels[i];
        PixelPacket this = this_pixels[i];

        int r_diff = (prev.red - this.red) * 0.2126;
        int g_diff = (prev.green - this.green) * 0.7152;
        int b_diff = (prev.blue - this.blue) * 0.0772;
        int a_diff = prev.opacity - this.opacity;

        uint64_t diffsq = r_diff*r_diff +
                          g_diff*g_diff +
                          b_diff*b_diff +
                          a_diff*a_diff;

        // Lossy: ignore changes in the input
        // that are likely to be imperceptible
        if (diffsq > 40000)
            box_union(&box, p_info->row, i);
    }

    p_info->row++;
    p_info->box = box;

    return MagickPass;
}

static aabb get_difference_box(Image *prev, Image *this, long width, long height)
{
    opt_info info = { 0 };

    ExceptionInfo ex;
    GetExceptionInfo(&ex);

    PixelIterateDualRead(
        &pixel_iterator,
        NULL,
        "Optimize pass",
        &info,
        NULL,
        width,
        height,
        prev,
        0,
        0,
        this,
        0,
        0,
        &ex
    );

    DestroyExceptionInfo(&ex);

    return info.box;
}

Image *gif_optimize(Image *coalesced)
{
    ExceptionInfo ex;
    GetExceptionInfo(&ex);

    Image *out = CloneImage(coalesced, 0, 0, 1, &ex);
    if (!out)
        goto error;

    Image *prev = coalesced;
    Image *this = coalesced->next;

    while (this != NULL) {
        aabb diff = get_difference_box(prev, this, prev->columns, prev->rows);

        if (diff.valid) {
            // There were differing pixels, crop to the differing
            // region
            RectangleInfo box = {
                .x      = diff.start_x,
                .y      = diff.start_y,
                .width  = diff.end_x - diff.start_x,
                .height = diff.end_y - diff.start_y
            };

            Image *cropped = CropImage(this, &box, &ex);
            if (!cropped)
                goto error;

            cropped->delay = this->delay;
            cropped->dispose = DISPOSE_DO_NOT;
            cropped->tile_info = box;

            AppendImageToList(&out, cropped);
        } else {
            // All the pixels were the same between these frames
            // Constitute a 1px transparent frame
            int pixel = 0;

            Image *blank = ConstituteImage(1, 1, "RGBA", CharPixel, &pixel, &ex);
            if (!blank)
                goto error;

            blank->delay = this->delay;
            blank->dispose = DISPOSE_DO_NOT;
            blank->tile_info = (RectangleInfo) { .x = 0, .y = 0, .width = 1, .height = 1 };

            AppendImageToList(&out, blank);
        }

        prev = prev->next;
        this = this->next;
    }

    DestroyExceptionInfo(&ex);
    return out;

error:
    if (out)
        DestroyImageList(out);

    DestroyExceptionInfo(&ex);
    return NULL;
}
