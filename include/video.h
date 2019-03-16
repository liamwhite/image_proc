#ifndef _VIDEO_H
#define _VIDEO_H

#include "common.h"

typedef struct video video;

// Returns a new video pointer if this buffer was
// successfully loaded, or NULL if it failed to load.
//
// Note that this function does not copy buf, it assigns
// it internally. Do not free or modify this memory after
// passing it to video_from_buffer; it will be freed
// automatically.
video *video_from_buffer(void *buf, size_t len);

// Returns a new video pointer if this file was
// successfully loaded, or NULL if it failed to load.
video *video_from_file(const char *filename);

// Invalidates and frees this video.
void video_free(video *v);

// Gets the dimensions of this video.
dim_t video_dimensions(video *v);

// Gets the duration, in seconds, of this video.
double video_duration(video *v);

// Gets corner intensities for the median time of this video.
// This method may fail if the video is unreadable.
int video_get_intensities(video *v, intensity_t *i);

// Scale this video proportionally to either a height of max_h,
// or a width of max_w, whichever is lesser.
video *video_scale(video *v, size_t max_w, size_t max_h);

// Write this video to memory. You must free() the returned memory.
buf_t video_to_buffer(video *v);

// Write this video to a file.
int video_to_file(video *v, const char *filename);

#endif // _VIDEO_H
