#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "video.h"

void test_load_gif()
{
    video *v = video_from_file("test/test_gif_animated.gif");
    assert(v != NULL);

    dim_t  dim = video_dimensions(v);
    double dur = video_duration(v);

    assert(dim.width == 277);
    assert(dim.height == 344);
    assert(dur == 19.1);

    intensity_t i;
    assert(video_get_intensities(v, &i));

    video_free(v);
}

void test_load_apng()
{
    video *v = video_from_file("test/test_apng.png");
    assert(v != NULL);

    dim_t  dim = video_dimensions(v);
    double dur = video_duration(v);

    assert(dim.width == 100);
    assert(dim.height == 100);
    assert(dur == 1.5);

    intensity_t i;
    assert(video_get_intensities(v, &i));

    video_free(v);
}

void test_load_webm()
{
    video *v = video_from_file("test/test_webm.webm");
    assert(v != NULL);

    dim_t  dim = video_dimensions(v);
    double dur = video_duration(v);

    assert(dim.width == 277);
    assert(dim.height == 344);
    assert(dur == 17.7);

    intensity_t i;
    assert(video_get_intensities(v, &i));

    video_free(v);
}


int main(int argc, char *argv[])
{
    // Test loading GIF, APNG
    test_load_gif();
    test_load_apng();

    // Test WebM
    test_load_webm();
}
