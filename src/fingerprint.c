/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf -CGD -t -K name -N mime_lookup src/fingerprint.gperf  */
/* Computed positions: -k'1,7' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "src/fingerprint.gperf"

#include <magic.h>
#include <string.h>

#include <stdio.h>

#include "fingerprint.h"
#line 9 "src/fingerprint.gperf"
struct mime_value {
    const char *name;
    file_type value;
};

#define TOTAL_KEYWORDS 7
#define MIN_WORD_LENGTH 9
#define MAX_WORD_LENGTH 13
#define MIN_HASH_VALUE 9
#define MAX_HASH_VALUE 15
/* maximum key range = 7, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register size_t len)
{
  static const unsigned char asso_values[] =
    {
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16,  2, 16, 16,
      16, 16, 16,  2, 16,  0,  5, 16, 16,  5,
      16, 16,  0, 16, 16,  0, 16, 16,  0,  0,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16
    };
  return len + asso_values[(unsigned char)str[6]] + asso_values[(unsigned char)str[0]];
}

static const struct mime_value wordlist[] =
  {
#line 14 "src/fingerprint.gperf"
    {"image/png",IMAGE_PNG},
#line 18 "src/fingerprint.gperf"
    {"video/webm",VIDEO_WEBM},
#line 16 "src/fingerprint.gperf"
    {"image/gif",IMAGE_GIF},
#line 19 "src/fingerprint.gperf"
    {"audio/webm",VIDEO_WEBM},
#line 17 "src/fingerprint.gperf"
    {"image/svg+xml",IMAGE_SVG},
#line 20 "src/fingerprint.gperf"
    {"video/mp4",VIDEO_MP4},
#line 15 "src/fingerprint.gperf"
    {"image/jpeg",IMAGE_JPG}
  };

static const signed char lookup[] =
  {
    -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  1,  2,  3,  4,
     5,  6
  };

const struct mime_value *
mime_lookup (register const char *str, register size_t len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = wordlist[index].name;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &wordlist[index];
            }
        }
    }
  return 0;
}
#line 21 "src/fingerprint.gperf"


static __thread magic_t cookie = NULL;

static magic_t magic()
{
    if (!cookie) {
        cookie = magic_open(MAGIC_MIME_TYPE);
        magic_load(cookie, NULL);
    }

    return cookie;
}

file_type fingerprint_buffer(const void *buf, size_t len)
{
    const char *mime = magic_buffer(magic(), buf, len);
    if (!mime)
      return UNKNOWN;

    const struct mime_value *mv = mime_lookup(mime, strlen(mime));

    if (mv)
      return mv->value;
    else
      return UNKNOWN;
}


file_type fingerprint_file(const char *filename)
{
    const char *mime = magic_file(magic(), filename);
    if (!mime)
      return UNKNOWN;

    const struct mime_value *mv = mime_lookup(mime, strlen(mime));

    if (mv)
      return mv->value;
    else
      return UNKNOWN;
}
