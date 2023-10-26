#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#define NUM_FRAME 15

int count = 1;
void print_channel_data(AVFrame const *frame, const int channel_indx)
{
    if (!(frame->format == AV_SAMPLE_FMT_FLTP && frame->nb_samples > 10))
        return;
    int seg_idx_1, seg_idx_2, seg_idx_3, seg_idx_4, data_end_idx, zero_start_idx;
    for (int i = 0; i < frame->linesize[0]; i++)
    {
        if (*(frame->data[channel_indx] + i) < 0x10)
            printf("0%x", *(frame->data[channel_indx] + i));
        else
            printf("%x", *(frame->data[channel_indx] + i));
        // 4 bytes per audio sample in a channel in AV_SAMPLE_FMT_FLTP format
        if (i % sizeof(float) == (sizeof(float) - 1))
            printf(" ");
        if (i == sizeof(float) * frame->nb_samples - 1)
            printf("| ");
        if (i == (sizeof(float) * 5 - 1))
        {
            seg_idx_1 = i;
            // AV_SAMPLE_FMT_FLTP is float planar
            i = sizeof(float) * (frame->nb_samples - 3) - 1;
            seg_idx_2 = i + 1;
            data_end_idx = i + sizeof(float) * 3;
            printf("... ");
        }
        else if (i == (sizeof(float) * (frame->nb_samples + 2) - 1))
        {
            seg_idx_3 = i;
            // display the end of channel data
            i = frame->linesize[0] - sizeof(float) * 2 - 1;
            seg_idx_4 = i + 1;
            printf("... ");
        }
    }
    printf("[Indices in bytes: %d->%d...%d->%d|%d->%d...%d->%d]\n", 0, seg_idx_1, seg_idx_2, data_end_idx, data_end_idx + 1, seg_idx_3, seg_idx_4, frame->linesize[0] - 1);
}

void print_frame_info(AVFrame const *frame, const int frame_idx)
{
    printf("#%d\n", frame_idx);
    printf("  left channel data:  0x");
    print_channel_data(frame, 0);
    printf("  right channel data: 0x");
    print_channel_data(frame, 1);
}

int decode_packet(AVCodecContext *dec, const AVPacket *pkt, AVFrame *frame, void *bf_pt)
{
    // submit the packet to the decoder
    int ret = avcodec_send_packet(dec, pkt);
    if (ret < 0)
    {
        fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
        return ret;
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_frame(dec, frame);
        if (ret < 0)
        {
            // those two return values are special and mean there is no output
            //   frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
            {
                return 0;
            }
            fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
            return ret;
        }
        if (count <= NUM_FRAME)
        {
            if (count == 1)
            {
                printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
                printf("Display the informaton of the first %d frames:\n", NUM_FRAME);
                printf("    Audio frame format:                            %s\n", av_get_sample_fmt_name(frame->format));
                printf("    Size in bytes of each plane:                   %d\n", frame->linesize[0]);
                printf("    Number of audio samples (per channel):         %d\n", frame->nb_samples);
                printf("    Sample rate(the number of samples per second): %d\n", frame->sample_rate);
                printf("Audio Frame:\n");
            }
            print_frame_info(frame, count);
            count++;
        }
        av_frame_unref(frame);
    }
    return 0;
}

static int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type, char *src_filename)
{
    AVStream *st;
    const AVCodec *dec = NULL;
    // Find the first video or audio stream
    int stream_index, ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0)
    {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n", av_get_media_type_string(type), src_filename);
        return ret;
    }
    else
    {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];
        // struct: AVCodecParameters * 	codecpar
        dec = avcodec_find_decoder(st->codecpar->codec_id); // find decoder for the stream
        if (!dec)
        {
            fprintf(stderr, "Failed to find %s codec\n", av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }
        *dec_ctx = avcodec_alloc_context3(dec); // Allocate a codec context for the decoder
        if (!*dec_ctx)
        {
            fprintf(stderr, "Failed to allocate the %s codec context\n", av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }
        ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar);
        if (ret < 0)
        {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n", av_get_media_type_string(type));
            return ret;
        }
        // Init the decoders
        ret = avcodec_open2(*dec_ctx, dec, NULL);
        if (ret < 0)
        {
            fprintf(stderr, "Failed to open %s codec\n", av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }
    return 0;
}

void *decode_audio(void *arg)
{
    char *fname = arg;
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx = NULL;
    AVStream *video_stream = NULL, *audio_stream = NULL;
    AVCodec *dec_a = NULL;
    AVFrame *frame = NULL;
    AVPacket *pkt = NULL;
    int video_stream_idx = -1, audio_stream_idx = -1, ret = avformat_open_input(&fmt_ctx, fname, NULL, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open source file %s\n", fname);
        return NULL;
    }
    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not find stream information\n");
        return NULL;
    }
    ret = open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO, fname);
    if (ret < 0)
    {
        fprintf(stderr, "Fail open_codec_context\n");
        return NULL;
    }
    else
        video_stream = fmt_ctx->streams[video_stream_idx];
    ret = open_codec_context(&audio_stream_idx, &audio_dec_ctx, fmt_ctx, AVMEDIA_TYPE_AUDIO, fname);
    if (ret < 0)
    {
        fprintf(stderr, "Fail open_codec_context\n");
        return NULL;
    }
    else
        audio_stream = fmt_ctx->streams[audio_stream_idx];
    // Dump information about file onto standard error
    av_dump_format(fmt_ctx, 0, fname, 0);
    if (!audio_stream)
    {
        fprintf(stderr, "Could not find audio or video stream in the input, aborting\n");
        return NULL;
    }
    frame = av_frame_alloc();
    if (!frame)
    {
        fprintf(stderr, "Could not allocate frame\n");
        return NULL;
    }
    pkt = av_packet_alloc();
    if (!pkt)
    {
        fprintf(stderr, "Could not allocate packet\n");
        return NULL;
    }
    // Read frames from the file
    while (av_read_frame(fmt_ctx, pkt) >= 0)
    {
        if (pkt->stream_index != video_stream_idx && pkt->stream_index == audio_stream_idx)
            ret = decode_packet(audio_dec_ctx, pkt, frame, NULL);
        av_packet_unref(pkt);
        if (ret < 0)
            break;
    }
    if (audio_dec_ctx)
        decode_packet(audio_dec_ctx, NULL, frame, NULL);
    av_packet_free(&pkt);
    av_freep(frame);
    avcodec_close(video_dec_ctx);
    avcodec_close(audio_dec_ctx);
    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t thread_dec_audio_id;
    if (pthread_create(&thread_dec_audio_id, NULL, &decode_audio, *(argv + 1)) != 0)
        printf("\ncan't create thread for writing.");
    if (pthread_join(thread_dec_audio_id, NULL) != 0)
        printf("\ncan't join a thread.\n");
    return 0;
}
