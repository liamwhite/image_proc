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

file_type fingerprint_buffer(void *buf, size_t len);
file_type fingerprint_file(const char *path);

#endif // _FINGERPRINT_H
