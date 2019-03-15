#include <assert.h>
#include <stddef.h>

#include "fingerprint.h"

// Small checkerboard pattern
static const char inline_png[] =
    "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
    "\x00\x00\x00\x10\x00\x00\x00\x10\x08\x04\x00\x00\x00\xB5\xFA\x37"
    "\xEA\x00\x00\x00\x19\x49\x44\x41\x54\x78\xDA\x63\xF8\xEF\x80\x0A"
    "\x19\xD0\xE1\x08\x51\x80\x2E\x80\xA1\x61\x64\x28\x00\x00\xA9\x5B"
    "\xBF\x81\xE6\x5B\xF9\x07\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42"
    "\x60\x82";

void test_fp_buf()
{
    assert(fingerprint_buffer(inline_png, sizeof(inline_png)) == IMAGE_PNG);
}

void test_fp_jpg()
{
    assert(fingerprint_file("test/test_jpeg.jpg") == IMAGE_JPG);
}

void test_fp_png()
{
    assert(fingerprint_file("test/test_png.png") == IMAGE_PNG);
}

void test_fp_gif_static()
{
    assert(fingerprint_file("test/test_gif_static.gif") == IMAGE_GIF);
}

void test_fp_gif_animated()
{
    assert(fingerprint_file("test/test_gif_animated.gif") == IMAGE_GIF);
}

void test_fp_unknown()
{
    assert(fingerprint_file(__FILE__) == UNKNOWN);
}

int main(int argc, char *argv[])
{
    // Test fingerprinting from buffer
    test_fp_buf();

    // Test fingerprinting various files
    test_fp_jpg();
    test_fp_png();
    test_fp_gif_static();
    test_fp_gif_animated();
    test_fp_unknown();

    return 0;
}
