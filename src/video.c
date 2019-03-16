#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include "video.h"

struct video {
    AVCodecContext *vctx;
    AVCodecContext *actx;
    AVFormatContext *format;
    AVIOContext *avio;
    AVFrame *frame;
    AVPacket *pkt;
    AVCodec *vcodec;
    AVCodec *acodec;
    int vstream_idx;
    int astream_idx;

    uint8_t *avio_buf; /// 4096-byte temporary buffer

    uint8_t *buf;  /// Orginal pointer to data
    size_t  len;   /// Original length of data
    int64_t pos;   /// Position in stream
    int fd;        /// Open file descriptor, or -1 if N/A

    dim_t dimensions;
    double duration;
    int64_t last_pts;
};

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_pix;

static int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    video *v = (video *) opaque;

    // Don't copy more than is remaining, or
    // less than zero bytes
    buf_size = FFMIN(buf_size, v->len - v->pos);
    buf_size = FFMAX(buf_size, 0);
    memcpy(buf, &v->buf[v->pos], buf_size);

    v->pos += buf_size;

    // Return how many bytes were read
    if (buf_size == 0)
        return AVERROR(EINVAL);
    else
        return buf_size;
    
}

static int64_t seek(void *opaque, int64_t offset, int whence)
{
    video *v = (video *) opaque;

    // POSIX allows seeking past the end of a stream,
    // so we also allow that here.

    switch (whence) {
    case SEEK_SET:
        if (offset < 0)
            return AVERROR(EINVAL);

        v->pos = offset;
        return v->pos;

    case SEEK_CUR:
        if (v->pos + offset < 0)
            return AVERROR(EINVAL);

        v->pos += offset;
        return v->pos;

    case SEEK_END:
        if (v->len + offset < 0)
            return AVERROR(EINVAL);

        v->pos = v->len + offset;
        return v->pos;
    default:
        return AVERROR(EINVAL);
    }
}

static int valid_video_codec(enum AVCodecID id)
{
    // Support WebM/MP4 video codecs, and typical animation codecs
    switch (id) {
    case AV_CODEC_ID_H264:
    case AV_CODEC_ID_VP8:
    case AV_CODEC_ID_VP9:
    case AV_CODEC_ID_APNG:
    case AV_CODEC_ID_GIF:
        return 1;

    default:
        return 0;
    }
}

static int valid_audio_codec(enum AVCodecID id)
{
    // WebM (Opus, Vorbis) + MP4 (AAC)
    switch (id) {
    case AV_CODEC_ID_OPUS:
    case AV_CODEC_ID_VORBIS:
    case AV_CODEC_ID_AAC:
        return 1;

    default:
        return 0;
    }
}

static video *video_initialize(video *v, void *buf, size_t len)
{
    v->buf = (uint8_t *) buf;
    v->len = len;
    if (!v->buf)
        goto error;

    v->format     = avformat_alloc_context();
    v->avio_buf   = av_malloc(4096);
    v->avio       = avio_alloc_context(v->avio_buf, 4096, 0, v, &read_packet, NULL, &seek);
    v->format->pb = v->avio;

    if (avformat_open_input(&v->format, "", NULL, NULL) < 0)
        goto error;

    if (avformat_find_stream_info(v->format, NULL) < 0)
        goto error;

    // Bad video stream is an error
    v->vstream_idx = av_find_best_stream(v->format, AVMEDIA_TYPE_VIDEO, -1, -1, &v->vcodec, 0);
    if (v->vstream_idx < 0 || !valid_video_codec(v->vcodec->id))
        goto error;

    // Bad audio stream is not an error
    v->astream_idx = av_find_best_stream(v->format, AVMEDIA_TYPE_AUDIO, -1, -1, &v->acodec, 0);
    if (v->astream_idx < 0 || !valid_audio_codec(v->acodec->id))
        v->acodec = NULL;

    // Initialize decoding contexts
    v->vctx = avcodec_alloc_context3(v->vcodec);

    if (v->acodec)
        v->actx = avcodec_alloc_context3(v->acodec);

    if (!v->vctx)
        goto error;

    // Setup codecs with coding parameters from container
    avcodec_parameters_to_context(v->vctx, v->format->streams[v->vstream_idx]->codecpar);

    if (v->actx)
        avcodec_parameters_to_context(v->actx, v->format->streams[v->astream_idx]->codecpar);

    // Initialize decoders
    if (avcodec_open2(v->vctx, v->vcodec, NULL) < 0 || avcodec_open2(v->vctx, v->acodec, NULL) < 0)
        goto error;

    v->frame = av_frame_alloc();
    v->pkt = av_packet_alloc();

    if (!v->frame || !v->pkt)
        goto error;

    v->dimensions.width  = v->vctx->width;
    v->dimensions.height = v->vctx->height;
    v->duration = -1;

    // Video must have dimensions
    if (v->dimensions.width == 0 || v->dimensions.height == 0)
        goto error;

    //av_dump_format(v->format, 0, "", 0);

    // All good
    return v;

error:
    video_free(v);
    return NULL;
}

// Returns a new video pointer if this buffer was
// successfully loaded, or NULL if it failed to load.
video *video_from_buffer(void *buf, size_t len)
{
    video *v = (video *) calloc(1, sizeof(video));
    if (!v)
        return NULL;

    v->fd = -1;

    return video_initialize(v, buf, len);
}

// Returns a new video pointer if this file was
// successfully loaded, or NULL if it failed to load.
video *video_from_file(const char *filename)
{
    video *v = (video *) calloc(1, sizeof(video));
    if (!v)
        return NULL;

    v->fd = open(filename, O_RDONLY);
    if (v->fd < 0)
        return NULL;

    struct stat len;
    if (fstat(v->fd, &len) < 0)
        goto error;

    void *buf = mmap(NULL, len.st_size, PROT_READ, MAP_PRIVATE, v->fd, 0);
    if (buf == MAP_FAILED)
        goto error;

    return video_initialize(v, buf, len.st_size);

error:
    if (v->fd >= 0)
        close(v->fd);

    return NULL;
}

// Invalidates and frees this video.
void video_free(video *v)
{
    if (!v)
        return;

    if (v->vctx)
        avcodec_free_context(&v->vctx);
    if (v->actx)
        avcodec_free_context(&v->actx);
    if (v->format)
        avformat_close_input(&v->format);
    if (v->avio->buffer)
        av_freep(&v->avio->buffer);
    if (v->avio)
        av_freep(&v->avio);
    if (v->frame)
        av_frame_free(&v->frame);
    if (v->pkt)
        av_packet_free(&v->pkt);

    if (v->fd > 0) {
        munmap(v->buf, v->len);
        close(v->fd);

        v->buf = NULL;
    }   

    if (v->buf)
        free(v->buf);

    free(v);
}

// Gets the dimensions of this video.
dim_t video_dimensions(video *v)
{
    return v->dimensions;
}

// Gets the duration, in seconds, of this video.
double video_duration(video *v)
{
    AVStream *vstream = v->format->streams[v->vstream_idx];

    if (v->duration > -1)
        return v->duration;

    if (v->format->duration != AV_NOPTS_VALUE) {
        // Format duration is in units of s/AV_TIME_BASE
        v->duration = v->format->duration / (double) AV_TIME_BASE;
        v->last_pts = v->duration * vstream->time_base.den / vstream->time_base.num;
        return v->duration;
    }

    // No idea what the duration is, so we will have to decode the entire
    // video in order to find it
    av_seek_frame(v->format, -1, 0, AVSEEK_FLAG_BACKWARD);
    v->last_pts = 0;

    while (1) {
        // done reading
        if (av_read_frame(v->format, v->pkt) < 0)
            break;

        // Not good enough to just look at the packet pts, unfortunately,
        // as libav does not seem to set it correctly in every case; we
        // need to also pass the frame to the decoder to get the right pts.
        if (v->pkt->stream_index == v->vstream_idx) {
            if (avcodec_send_packet(v->vctx, v->pkt) != 0)
                continue;

            if (avcodec_receive_frame(v->vctx, v->frame) == 0) {
                v->last_pts = FFMAX(v->last_pts, v->frame->pts + v->pkt->duration);
                av_frame_unref(v->frame);
            }
        }

        av_packet_unref(v->pkt);
    }

    // pts duration is in units of s/ts
    v->duration = v->last_pts * vstream->time_base.num / (double) vstream->time_base.den;

    // Reset the stream
    av_seek_frame(v->format, -1, 0, AVSEEK_FLAG_BACKWARD);

    if (v->duration > 0)
        return v->duration;
    else
        return 0;
}

static float sum_intensity(rect_sum_t sum, uint32_t npixels)
{
    return ((sum.r / npixels) * 0.2126 +
            (sum.g / npixels) * 0.7152 +
            (sum.b / npixels) * 0.0772) / 3;
}

static rect_sum_t rect_sum(uint8_t *restrict opaque, uint32_t width, rect_t bounds)
{
    rgb_pix *restrict image = (rgb_pix *) opaque;

    // The sum of an empty region is defined to be zero.
    rect_sum_t ret = { 0 };

    for (uint32_t i = bounds.start_y; i < bounds.end_y; ++i) {
      for (uint32_t j = bounds.start_x; j < bounds.end_x; ++j) {
          rgb_pix p = image[i * width + j];

          ret.r += p.r;
          ret.g += p.g;
          ret.b += p.b;
        }
    }

    return ret;
}

static void calculate_frame_intensities(video *v, intensity_t *i)
{
    // Do colorspace conversion with swscale.
    //
    // This frame is unlikely to be in RGB24, so we take the hit and
    // convert it unconditionally. This is just memcpy if the format is
    // already RGB24.
    //
    struct SwsContext *ctx = NULL;
    uint8_t *rgb = NULL;
    AVFrame *f = v->frame;
    uint32_t w = f->width;
    uint32_t h = f->height;

    ctx = sws_getContext(w, h, v->vctx->pix_fmt, w, h, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

    int32_t rgbstride = f->width * 3;
    rgb = av_malloc(rgbstride * f->height);

    if (!ctx || !rgb)
        goto error;

    sws_scale(ctx, (const uint8_t **) f->data, f->linesize, 0, h, &rgb, &rgbstride);

    rect_sum_t ins[] = {
        rect_sum(rgb, w, (rect_t) { 0, 0, w/2, h/2 }), // nw
        rect_sum(rgb, w, (rect_t) { w/2, 0, w, h/2 }), // ne
        rect_sum(rgb, w, (rect_t) { 0, h/2, w/2, h }), // sw
        rect_sum(rgb, w, (rect_t) { w/2, h/2, w, h }), // se
        rect_sum(rgb, w, (rect_t) { 0, 0, w, h })      // avg
    };

    *i = (intensity_t) {
        .nw  = sum_intensity(ins[0], w*h/4),
        .ne  = sum_intensity(ins[1], w*h/4),
        .sw  = sum_intensity(ins[2], w*h/4),
        .se  = sum_intensity(ins[3], w*h/4),
        .avg = sum_intensity(ins[4], w*h)
    };

error:
    if (ctx)
        sws_freeContext(ctx);

    if (rgb)
        av_free(rgb);
}

// Gets corner intensities for the median time of this video.
int video_get_intensities(video *v, intensity_t *i)
{
    video_duration(v);

    // no length?
    if (v->duration <= 0)
        return 0;

    int64_t mid_time = v->duration * AV_TIME_BASE / 2;
    int64_t mid_pts  = v->last_pts / 2;

    // the animation may have at minimum one keyframe, so go there
    av_seek_frame(v->format, -1, mid_time, AVSEEK_FLAG_BACKWARD);

    int found = 0;

    // now iterate until we find the frame we're looking for
    while (!found) {
        // done reading
        if (av_read_frame(v->format, v->pkt) < 0)
            break;

        if (v->pkt->stream_index == v->vstream_idx) {
            if (avcodec_send_packet(v->vctx, v->pkt) != 0)
                continue;

            if (avcodec_receive_frame(v->vctx, v->frame) == 0) {
                if (v->frame->pts + v->pkt->duration >= mid_pts) {
                    calculate_frame_intensities(v, i);
                    found = 1;
                }

                av_frame_unref(v->frame);
            }
        }

        av_packet_unref(v->pkt);
    }

    return found;
}

// Scale this video proportionally to either a height of max_h,
// or a width of max_w, whichever is lesser.
video *video_scale(video *v, size_t max_w, size_t max_h);

// Write this video to memory. You must free() the returned memory.
buf_t video_to_buffer(video *v);

// Write this video to a file.
int video_to_file(video *v, const char *filename);
