#ifndef _FINGERPRINT_H
#define _FINGERPRINT_H

typedef enum {
  IMAGE_PNG,
  IMAGE_JPG,
  IMAGE_GIF,
  IMAGE_SVG,
  VIDEO_WEBM,
  VIDEO_MP4,
  UNKNOWN
} file_type;

// Returns the associated enum value of the MIME type of
// this buffer, or UNKNOWN.
file_type fingerprint_buffer(const void *buf, size_t len);

// Returns the associated enum value of the MIME type of
// this file, or UNKNOWN.
file_type fingerprint_file(const char *path);

#endif // _FINGERPRINT_H
