#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

#include <libavformat/avformat.h>

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
};

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
        // Format duration is in units of AV_TIME_BASE/s
        v->duration = v->format->duration / (double) AV_TIME_BASE;
        return v->duration;
    }
 
    // No idea what the duration is, so we will have to decode the entire
    // video in order to find it
    av_seek_frame(v->format, -1, 0, AVSEEK_FLAG_BACKWARD);
    int64_t last_pts = 0;

    while (1) {
        // done reading
        if (av_read_frame(v->format, v->pkt) < 0)
            break;

        // Not good enough to just look at the packet pts, unfortunately,
        // as libav does not seem to set it correctly in every case; we
        // need to also pass the frame to the decoder to get the right dur.
        if (v->pkt->stream_index == v->vstream_idx) {
            if (avcodec_send_packet(v->vctx, v->pkt) != 0)
                continue;

            if (avcodec_receive_frame(v->vctx, v->frame) == 0) {
                last_pts = FFMAX(last_pts, v->frame->pts + v->pkt->duration);
                av_frame_unref(v->frame);
            }
        }

        av_packet_unref(v->pkt);
    }

    // pts duration is in units of s/ts
    v->duration = last_pts * vstream->time_base.num / (double) vstream->time_base.den;

    // Reset the stream
    av_seek_frame(v->format, 0, vstream->first_dts, AVSEEK_FLAG_BACKWARD);

    if (v->duration > 0)
        return v->duration;
    else
        return 0;
}

// Gets corner intensities for the median time of this video.
int video_get_intensities(video *v, intensity_t *i)
{
    return 0;
}

// Scale this video proportionally to either a height of max_h,
// or a width of max_w, whichever is lesser.
video *video_scale(video *v, size_t max_w, size_t max_h);

// Write this video to memory. You must free() the returned memory.
buf_t video_to_buffer(video *v);

// Write this video to a file.
int video_to_file(video *v, const char *filename);
