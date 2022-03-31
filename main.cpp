extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
}

static AVBufferRef *hw_device_ctx = nullptr;
static enum AVPixelFormat hw_pix_fmt;

static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts) {
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }

    fprintf(stderr, "Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}

static int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type)
{
    int err = 0;

    if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type,
                                      nullptr, nullptr, 0)) < 0) {
        fprintf(stderr, "Failed to create specified HW device.\n");
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);

    return err;
}

int main() {
    // get hw device type, check if supported
    enum AVHWDeviceType type;
    type = av_hwdevice_find_type_by_name("vdpau");
    if (type == AV_HWDEVICE_TYPE_NONE) {
        fprintf(stderr, "Device type vdpau is not supported.");
        return -1;
    }

    // packet alloc
    AVPacket *packet = av_packet_alloc();
    if (!packet) {
        fprintf(stderr, "Packet alloc failed.");
        return -1;
    }

    // open input file
    AVFormatContext *input_ctx = nullptr;
    if (avformat_open_input(&input_ctx, "test.flv", nullptr, nullptr) != 0) {
        fprintf(stderr, "Open input file test.flv failed.");
        return -1;
    }

    // find video stream info
    const AVCodec *decoder = nullptr;
    if (avformat_find_stream_info(input_ctx, nullptr) < 0) {
        fprintf(stderr, "Cannot find input file stream info.");
        return -1;
    }
    int video_stream = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
    if (video_stream < 0) {
        fprintf(stderr, "Cannot find input file video stream info.");
        return -1;
    }

    // get hw device config & pixel format
    for (int i = 0; ; i++) {
        const AVCodecHWConfig *hw_config = avcodec_get_hw_config(decoder, i);
        if (!hw_config) {
            fprintf(stderr, "Decoder %s not support device type %s\n", decoder->name, av_hwdevice_get_type_name(type));
        }

        if (hw_config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && hw_config->device_type == type) {
            hw_pix_fmt = hw_config->pix_fmt;
            break;
        }
    }

    // alloc decoder context
    AVCodecContext *decoder_ctx = nullptr;
    if (!(decoder_ctx = avcodec_alloc_context3(decoder))) {
        return AVERROR(ENOMEM);
    }

    // copy video stream codec parameters
    AVStream *video;
    video = input_ctx->streams[video_stream];
    if (avcodec_parameters_to_context(decoder_ctx, video->codecpar) < 0) {
        return -1;
    }

    decoder_ctx->get_format = get_hw_format;

    // init hw decoder
    if (hw_decoder_init(decoder_ctx, type) < 0) {
        return -1;
    }

    // open codec
    if (avcodec_open2(decoder_ctx, decoder, nullptr) < 0) {
        fprintf(stderr, "Failed to open codec for video stream.");
        return -1;
    }

    // decode loop
    int ret = 0;
    while(ret >= 0) {
        if ((ret = av_read_frame(input_ctx, packet)) < 0) {
            break;
        }
    }
    return 0;
}
