#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "raster_image.h"

// Small checkerboard pattern
static const char inline_png[] =
    "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
    "\x00\x00\x00\x10\x00\x00\x00\x10\x08\x04\x00\x00\x00\xB5\xFA\x37"
    "\xEA\x00\x00\x00\x19\x49\x44\x41\x54\x78\xDA\x63\xF8\xEF\x80\x0A"
    "\x19\xD0\xE1\x08\x51\x80\x2E\x80\xA1\x61\x64\x28\x00\x00\xA9\x5B"
    "\xBF\x81\xE6\x5B\xF9\x07\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42"
    "\x60\x82";

void test_load_buf()
{
    raster_image *ri = raster_image_from_buffer(inline_png, sizeof(inline_png));
    assert(ri != NULL);

    dim_t  dim    = raster_image_dimensions(ri);
    size_t frames = raster_image_frame_count(ri);

    assert(dim.width == 16);
    assert(dim.height == 16);
    assert(frames == 1);

    raster_image_free(ri);
}

void test_load_file_jpg()
{
    raster_image *ri = raster_image_from_file("test/test_jpeg.jpg");
    assert(ri != NULL);

    dim_t  dim    = raster_image_dimensions(ri);
    size_t frames = raster_image_frame_count(ri);

    assert(dim.width == 1024);
    assert(dim.height == 768);
    assert(frames == 1);

    intensity_t i = raster_image_get_intensities(ri);
    assert(abs(i.nw - 28.893986) <= 1e-3);
    assert(abs(i.ne - 27.815023) <= 1e-3);
    assert(abs(i.sw - 37.484633) <= 1e-3);
    assert(abs(i.se - 36.998046) <= 1e-3);
    assert(abs(i.avg - 32.794994) <= 1e-3);

    raster_image_free(ri);
}

void test_load_file_jpg_orient()
{
    raster_image *ri = raster_image_from_file("test/test_jpeg_orient.jpg");
    assert(ri != NULL);

    dim_t  dim    = raster_image_dimensions(ri);
    size_t frames = raster_image_frame_count(ri);

    assert(dim.width == 768);
    assert(dim.height == 1024);
    assert(frames == 1);

    raster_image_free(ri);
}


void test_load_file_png()
{
    raster_image *ri = raster_image_from_file("test/test_png.png");
    assert(ri != NULL);

    dim_t  dim    = raster_image_dimensions(ri);
    size_t frames = raster_image_frame_count(ri);

    assert(dim.width == 561);
    assert(dim.height == 535);
    assert(frames == 1);

    intensity_t i = raster_image_get_intensities(ri);
    assert(abs(i.nw - 73.120985) <= 1e-3);
    assert(abs(i.ne - 72.824828) <= 1e-3);
    assert(abs(i.sw - 67.314487) <= 1e-3);
    assert(abs(i.se - 68.514023) <= 1e-3);
    assert(abs(i.avg - 70.432392) <= 1e-3);

    raster_image_free(ri);
}

void test_load_file_gif_static()
{
    raster_image *ri = raster_image_from_file("test/test_gif_static.gif");
    assert(ri != NULL);

    dim_t  dim    = raster_image_dimensions(ri);
    size_t frames = raster_image_frame_count(ri);

    assert(dim.width == 277);
    assert(dim.height == 344);
    assert(frames == 1);

    intensity_t i = raster_image_get_intensities(ri);
    assert(abs(i.nw - 47.720603) <= 1e-3);
    assert(abs(i.ne - 47.918849) <= 1e-3);
    assert(abs(i.sw - 48.038489) <= 1e-3);
    assert(abs(i.se - 48.644908) <= 1e-3);
    assert(abs(i.avg - 48.077841) <= 1e-3);

    raster_image_free(ri);
}

void test_load_file_gif_animated()
{
    raster_image *ri = raster_image_from_file("test/test_gif_animated.gif");
    assert(ri != NULL);

    dim_t  dim    = raster_image_dimensions(ri);
    size_t frames = raster_image_frame_count(ri);

    assert(dim.width == 277);
    assert(dim.height == 344);
    assert(frames == 163);

    intensity_t i = raster_image_get_intensities(ri);
    assert(abs(i.nw - 47.603815) <= 1e-3);
    assert(abs(i.ne - 48.237846) <= 1e-3);
    assert(abs(i.sw - 47.902737) <= 1e-3);
    assert(abs(i.se - 49.262892) <= 1e-3);
    assert(abs(i.avg - 48.250947) <= 1e-3);

    raster_image_free(ri);
}

int main(int argc, char *argv[])
{
    // Test loading from buffer
    test_load_buf();

    // Test loading various files
    test_load_file_jpg();
    test_load_file_jpg_orient();
    test_load_file_png();
    test_load_file_gif_static();
    test_load_file_gif_animated();

    return 0;
}
