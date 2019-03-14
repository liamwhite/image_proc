#ifndef _IMAGE_COMMON_H
#define _IMAGE_COMMON_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t width;
    uint32_t height;
} dim_t;

typedef struct {
    float nw;
    float ne;
    float sw;
    float se;
    float avg;
} intensity_t;

typedef struct {
    void *buf;
    size_t len;
} buf_t;

#endif // _IMAGE_COMMON_H
