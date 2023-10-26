#include <cairo.h>
#include <gtk/gtk.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <CoreAudio/CoreAudio.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

#define BUFFERSIZE 2
#define BUFFERSIZE_A 64
#define AUDIO_CHANNEL 2
#define FPS 24

GtkWidget *darea;
struct timespec ts_prev, ts_cur;

struct Arg
{
    struct Buffer *buffer_pt;
    struct Buffer_A *buffer_pt_a;
    char *filename;
    AVFrame *cur_frame;
};

struct Buffer
{
    pthread_mutex_t lock;
    AVFrame *data[BUFFERSIZE];
    unsigned int nread;        // number of frames already displayed
    unsigned int nwrite;       // number of frames already decoded
    pthread_cond_t cond_read;  // wait channel for displayer
    pthread_cond_t cond_write; // wait channel for decoder
    bool readable;             // true if the GTK window is ready; false, otherwise
    bool writable;             // true if decoder does NOT reach the end of the file; false, otherwise
};

struct Buffer_A
{
    pthread_mutex_t lock;
    float data[BUFFERSIZE_A];
    unsigned int nread;        // number of frames already displayed
    unsigned int nwrite;       // number of frames already decoded
    pthread_cond_t cond_read;  // wait channel for displayer
    pthread_cond_t cond_write; // wait channel for decoder
    pthread_cond_t cond_stop;  // wait channel for stopping playback
    bool readable;             // true if the GTK window is ready; false, otherwise
    bool writable;             // true if decoder does NOT reach the end of the file; false, otherwise
};

void buffer_init(struct Buffer *bf)
{
    if (pthread_mutex_init(&bf->lock, NULL))
        fprintf(stderr, "Failed to init the lock in Buffer bf");
    if (pthread_cond_init(&bf->cond_read, NULL) || pthread_cond_init(&bf->cond_write, NULL))
        fprintf(stderr, "Failed to init the cond in Buffer bf");
    bf->nread = 0;
    bf->nwrite = 0;
    bf->readable = false;
    bf->writable = true;
}

void buffer_init_a(struct Buffer_A *bf_pt)
{
    int ret;
    ret = pthread_mutex_init(&bf_pt->lock, NULL);
    if (ret)
        fprintf(stderr, "Failed to init the lock in Buffer_A bf");

    bf_pt->nread = 0;
    bf_pt->nwrite = 0;
    ret = pthread_cond_init(&bf_pt->cond_read, NULL);
    if (ret)
        fprintf(stderr, "Failed to init the cond in Buffer_A bf");
    ret = pthread_cond_init(&bf_pt->cond_write, NULL);
    if (ret)
        fprintf(stderr, "Failed to init the cond in Buffer_A bf");
    ret = pthread_cond_init(&bf_pt->cond_stop, NULL);
    if (ret)
        fprintf(stderr, "Failed to init the cond in Buffer_A bf");

    bf_pt->readable = true;
    bf_pt->writable = true;

    printf("Func - buffer_init_a succeed\n");
    return;
}

static int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type, char *src_filename)
{
    int ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    int stream_index = ret;
    const AVCodec *dec = avcodec_find_decoder(fmt_ctx->streams[stream_index]->codecpar->codec_id); // find decoder for the stream
    if (ret < 0)
    {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n", av_get_media_type_string(type), src_filename);
        return ret;
    }
    else
    {
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
        if ((ret = avcodec_parameters_to_context(*dec_ctx, fmt_ctx->streams[stream_index]->codecpar)) < 0)
        {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n", av_get_media_type_string(type));
            return ret;
        }
        // Init the decoders
        if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0)
        {
            fprintf(stderr, "Failed to open %s codec\n", av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
        return 0;
    }
}

int decode_packet_a(AVCodecContext *dec, const AVPacket *pkt, AVFrame *frame, struct Buffer_A *pt)
{
    // submit the packet to the decoder
    int ret = avcodec_send_packet(dec, pkt);
    if (ret < 0)
    {
        fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
        return ret;
    }
    // acquire a lock
    pthread_mutex_lock(&pt->lock);
    ret = avcodec_receive_frame(dec, frame);
    if (ret < 0)
    {
        // those two return values are special and mean there is no output
        //   frame available, but there were no errors during decoding
        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
        {
            pthread_mutex_unlock(&pt->lock);
            return 0;
        }
        fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
        pthread_mutex_unlock(&pt->lock);
        return ret;
    }
    // write the frame data to Buffer
    for (int i = 0; i < frame->nb_samples; i++)
        for (int j = 0; j < AUDIO_CHANNEL; j++)
        {
            while (pt->nwrite == pt->nread + BUFFERSIZE_A)
            {
                pthread_cond_signal(&pt->cond_read);
                pthread_cond_wait(&pt->cond_write, &pt->lock);
            }
            // Read Audio Frame Data
            pt->data[pt->nwrite % BUFFERSIZE_A] = *((float *)frame->data[j] + i);
            pt->nwrite++;
        }
    av_frame_unref(frame);
    // release lock
    pthread_mutex_unlock(&pt->lock);
    return 0;
}

void *decode_audio(void *usrdata)
{
    struct Arg *arg = usrdata;
    struct Buffer_A *pt = arg->buffer_pt_a;
    char *fname = arg->filename;
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx = NULL;
    AVStream *video_stream = NULL, *audio_stream = NULL;
    AVCodec *dec_a = NULL;
    AVFrame *frame = NULL;
    AVPacket *pkt = NULL;
    int video_stream_idx = -1, audio_stream_idx = -1;
    if (avformat_open_input(&fmt_ctx, fname, NULL, NULL) < 0)
    {
        fprintf(stderr, "Could not open source file %s\n", fname);
        return NULL;
    }
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
    {
        fprintf(stderr, "Could not find stream information\n");
        return NULL;
    }
    if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO, fname) < 0)
    {
        fprintf(stderr, "Fail open_codec_context\n");
        return NULL;
    }
    video_stream = fmt_ctx->streams[video_stream_idx];
    if (open_codec_context(&audio_stream_idx, &audio_dec_ctx, fmt_ctx, AVMEDIA_TYPE_AUDIO, fname) < 0)
    {
        fprintf(stderr, "Fail open_codec_context\n");
        return NULL;
    }
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
            if (decode_packet_a(audio_dec_ctx, pkt, frame, pt) < 0)
                break;
        av_packet_unref(pkt);
    }
    if (audio_dec_ctx)
        decode_packet_a(audio_dec_ctx, NULL, frame, pt);
    av_packet_free(&pkt);
    av_freep(frame);
    pt->writable = false;
    pthread_cond_signal(&pt->cond_read);
    pthread_cond_signal(&pt->cond_stop);
    avcodec_close(video_dec_ctx);
    avcodec_close(audio_dec_ctx);
    return NULL;
}

static AVFrame *avFrameConvertPixelFormat(const AVFrame *src, enum AVPixelFormat dstFormat)
{
    AVFrame *dst = av_frame_alloc();
    int width = src->width, height = src->height;
    if (dst == NULL)
    {
        fprintf(stderr, "avcodec_alloc_frame failed");
        return NULL;
    }
    if (av_image_alloc(dst->data, dst->linesize, width, height, dstFormat, 1) < 0)
    {
        fprintf(stderr, "av_image_alloc failed");
        return NULL;
    }
    dst->format = dstFormat;
    dst->width = src->width;
    dst->height = src->height;
    struct SwsContext *conversion = sws_getContext(width, height, (enum AVPixelFormat)src->format, width, height, dstFormat, SWS_FAST_BILINEAR | SWS_FULL_CHR_H_INT | SWS_ACCURATE_RND, NULL, NULL, NULL);
    sws_scale(conversion, (uint8_t const *const *)src->data, src->linesize, 0, height, dst->data, dst->linesize);
    sws_freeContext(conversion);
    return dst;
}

int writer_decode_packet(AVCodecContext *dec, const AVPacket *pkt, AVFrame *frame, struct Buffer *pt)
{
    int ret = avcodec_send_packet(dec, pkt);
    if (ret < 0)
    {
        fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
        return ret;
    }
    pthread_mutex_lock(&pt->lock);
    while (pt->nwrite == pt->nread + BUFFERSIZE)
    {
        if (pt->readable)
            pthread_cond_signal(&pt->cond_read);
        else
            printf("writing done - waiting for GTK readay(pt->readable)\n");
        pthread_cond_wait(&pt->cond_write, &pt->lock);
    }
    ret = avcodec_receive_frame(dec, frame);
    if (ret < 0)
    {
        // those two return values are special and mean there is no output
        //   frame available, but there were no errors during decoding
        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
        {
            pthread_mutex_unlock(&pt->lock);
            return 0;
        }
        fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
        pthread_mutex_unlock(&pt->lock);
        return ret;
    }
    // convert yuv420p10le to rgb24 (or any other RGB formats)
    // it is the reader's responsibility to call av_frame_free(&frame) after reading one frame
    pt->data[pt->nwrite++ % BUFFERSIZE] = avFrameConvertPixelFormat(frame, AV_PIX_FMT_RGB32);
    av_frame_unref(frame);
    pthread_mutex_unlock(&pt->lock);
    return 0;
}

void *write_frame_rgb32(void *usrdata)
{
    struct Arg *arg = usrdata;
    struct Buffer *pt = arg->buffer_pt;
    char *fname = arg->filename;
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *video_dec_ctx = NULL;
    AVStream *video_stream = NULL;
    AVFrame *frame = NULL;
    AVPacket *pkt = NULL;
    int video_stream_idx = -1;
    if (avformat_open_input(&fmt_ctx, fname, NULL, NULL) < 0)
    {
        fprintf(stderr, "Could not open source file %s\n", fname);
        return NULL;
    }
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
    {
        fprintf(stderr, "Could not find stream information\n");
        return NULL;
    }
    if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO, fname) < 0)
    {
        fprintf(stderr, "Fail open_codec_context\n");
        return NULL;
    }
    else
        video_stream = fmt_ctx->streams[video_stream_idx];
    av_dump_format(fmt_ctx, 0, fname, 0);
    if (!video_stream)
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
        if (pkt->stream_index == video_stream_idx)
            if (writer_decode_packet(video_dec_ctx, pkt, frame, pt) < 0)
                break;
        av_packet_unref(pkt);
    }
    if (video_dec_ctx)
        writer_decode_packet(video_dec_ctx, NULL, frame, pt);
    av_packet_free(&pkt);
    av_freep(frame);
    pt->writable = false;
    pthread_cond_signal(&pt->cond_read);
    avcodec_close(video_dec_ctx);
    return NULL;
}

OSStatus my_fill_buffer(AudioObjectID inDevice, const AudioTimeStamp *inNow, const AudioBufferList *inInputData, const AudioTimeStamp *inInputTime, AudioBufferList *outOutputData, const AudioTimeStamp *inOutputTime, void *__nullable inClientData)
{
    struct Arg *arg = inClientData;
    struct Buffer_A *pt = arg->buffer_pt_a;
    AudioBuffer buffer = outOutputData->mBuffers[0];
    // acquire a lock
    pthread_mutex_lock(&pt->lock);
    for (int i = 0; i < buffer.mDataByteSize / (sizeof(float)) && pt->nread <= pt->nwrite; i++)
    {
        while (pt->nread == pt->nwrite)
        {
            if (!pt->writable)
            {
                pt->readable = false;
                // release lock
                pthread_mutex_unlock(&pt->lock);
                printf("Audio palying complete!\n");
                pthread_cond_signal(&pt->cond_stop);
                return noErr;
            }
            pthread_cond_signal(&pt->cond_write);
            pthread_cond_wait(&pt->cond_read, &pt->lock);
        }
        ((float *)buffer.mData)[i] = (pt->data[pt->nread % BUFFERSIZE_A]);
        pt->nread++;
    }
    pthread_cond_signal(&pt->cond_write);
    // release lock
    pthread_mutex_unlock(&pt->lock);
    return noErr;
}

void printCoreAudioErrorAndExit(OSStatus error)
{
    if (error != noErr)
    {
        printf("Error: %d", error);
        exit(-1);
    }
}

void *playing_sound(void *usrdata)
{
    struct Arg *arg = usrdata;
    struct Buffer_A *pt = arg->buffer_pt_a;
    // Create a struct to query to correct parameter
    AudioObjectPropertyAddress property = (AudioObjectPropertyAddress){
        .mSelector = kAudioHardwarePropertyDefaultOutputDevice,
        .mElement = kAudioObjectPropertyElementMain,
        .mScope = kAudioObjectPropertyScopeGlobal};
    // tell how much memory should be allocated for the result of AudioObjectGetPropertyData
    UInt32 propertySize;
    printCoreAudioErrorAndExit(AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &property, 0, NULL, &propertySize));
    // print out the index of the speaker
    printf("-> The index of the speaker is %lu\n", (unsigned long)kAudioObjectSystemObject);
    // Create a variable of the expected output type
    AudioObjectID defaultOutputDevice;
    // Try to call the function to receive the desired property
    printCoreAudioErrorAndExit(AudioObjectGetPropertyData(kAudioObjectSystemObject, &property, 0, NULL, &propertySize, &defaultOutputDevice));
    // let the audio device pull the audio data, by calling a function in regular
    // intervals, in which you can provide a chunk of audio data.
    AudioDeviceIOProcID procID;
    printCoreAudioErrorAndExit(AudioDeviceCreateIOProcID(defaultOutputDevice, my_fill_buffer, usrdata, &procID));
    // We need to start the device
    printCoreAudioErrorAndExit(AudioDeviceStart(defaultOutputDevice, procID));
    while (pt->writable || pt->readable)
        pthread_cond_wait(&pt->cond_stop, &pt->lock);
    // stop the playback.
    printCoreAudioErrorAndExit(AudioDeviceStop(defaultOutputDevice, NULL));
    // After the playback has stopped we will clean up the resources and terminate the program.
    printCoreAudioErrorAndExit(AudioDeviceDestroyIOProcID(defaultOutputDevice, procID));
    return NULL;
}

void on_draw_event(GtkWidget *widget, cairo_t *cr, struct Arg *arg)
{
    struct Buffer *pt = arg->buffer_pt;
    AVFrame *frame = arg->cur_frame;
    cairo_surface_t *surface = cairo_image_surface_create_for_data(frame->data[0], CAIRO_FORMAT_ARGB32, frame->width, frame->height, cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, frame->width));
    av_freep(frame);
    cairo_set_source_surface(cr, surface, 0, 0);
    if (pt->nread == 0)
        clock_gettime(CLOCK_MONOTONIC, &ts_prev);
    struct timespec ts_acc = ts_prev;
    ts_acc.tv_sec = ts_acc.tv_sec + pt->nread / 24;
    long nsec_tmp = ts_acc.tv_nsec + (pt->nread % 24) * (1000000000 / FPS);
    if (nsec_tmp >= 1000000000)
    {
        nsec_tmp = nsec_tmp - 1000000000;
        ts_acc.tv_sec++;
    }
    ts_acc.tv_nsec = nsec_tmp;
    clock_gettime(CLOCK_MONOTONIC, &ts_cur);
    struct timespec ts_wait = {0, 0};
    if (ts_cur.tv_sec == ts_acc.tv_sec)
    {
        if (ts_cur.tv_nsec < ts_acc.tv_nsec)
            ts_wait.tv_nsec = ts_acc.tv_nsec - ts_cur.tv_nsec;
    }
    else if (ts_cur.tv_sec < ts_acc.tv_sec)
    {
        if (ts_cur.tv_nsec > ts_acc.tv_nsec)
        {
            ts_wait.tv_sec = ts_acc.tv_sec - ts_cur.tv_sec - 1;
            ts_wait.tv_nsec = ts_acc.tv_nsec - ts_cur.tv_nsec + 1000000000;
        }
        else
        {
            ts_wait.tv_sec = ts_acc.tv_sec - ts_cur.tv_sec;
            ts_wait.tv_nsec = ts_acc.tv_nsec - ts_cur.tv_nsec;
        }
    }
    nanosleep(&ts_wait, NULL);
    cairo_paint(cr);
    pt->nread++;
    pthread_cond_signal(&pt->cond_read);
    cairo_surface_destroy(surface);
    return;
}

void *read_frame(gpointer user_data)
{
    struct Buffer *pt = ((struct Arg *)user_data)->buffer_pt;
    pt->readable = true;
    // Acquire a lock
    pthread_mutex_lock(&pt->lock);
    // The buffer is non-empty, read and display
    while (pt->nread <= pt->nwrite)
    {
        while (pt->nread == pt->nwrite)
        {
            if (!pt->writable)
            {
                // Release lock
                pthread_mutex_unlock(&pt->lock);
                g_application_quit(G_APPLICATION(user_data));
                exit(0);
            }
            pthread_cond_signal(&pt->cond_write);
            pthread_cond_wait(&pt->cond_read, &pt->lock);
        }
        ((struct Arg *)user_data)->cur_frame = pt->data[pt->nread % BUFFERSIZE];
        g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), user_data);
        gtk_widget_queue_draw(darea);
        pthread_cond_wait(&pt->cond_read, &pt->lock);
    }
    // Release lock
    pthread_mutex_unlock(&pt->lock);
    return NULL;
}

void activate(GtkApplication *app, void *user_data)
{
    struct Buffer *pt = ((struct Arg *)user_data)->buffer_pt;
    char *fname = ((struct Arg *)user_data)->filename, title[100] = "Video Player - video file \"";
    strcat(title, fname);
    strcat(title, "\"");
    int video_width = (*pt->data)->width, video_height = (*pt->data)->height;
    GtkWidget *screen = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(screen), title);
    gtk_window_set_default_size(GTK_WINDOW(screen), video_width, video_height);
    gtk_window_move(GTK_WINDOW(screen), 280, 100);
    darea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(screen), darea);
    gtk_widget_show_all(screen);
    pthread_t thread_read_id;
    if (pthread_create(&thread_read_id, NULL, &read_frame, user_data) != 0)
        printf("\nCan't create thread for writing");
    return;
}

int main(int argc, char **argv)
{
    struct Buffer bf;
    struct Buffer_A bf_a;
    struct Arg passing_arg = {&bf, &bf_a, *(argv + 1)};
    if (argc < 2)
    {
        fprintf(stderr, "One argument needed as an input txt file\n");
        exit(1);
    }
    buffer_init(&bf);
    buffer_init_a(&bf_a);
    pthread_t thread_write_id, thread_dec_audio_id, thread_play_sound_id;
    if (pthread_create(&thread_write_id, NULL, &write_frame_rgb32, &passing_arg) != 0)
        printf("\nCan't create thread for reading.");
    else
        printf(" Thread for writing_v created successfully\n");
    if (pthread_create(&thread_dec_audio_id, NULL, &decode_audio, &passing_arg) != 0)
        printf("\nCan't create thread for reading.");
    else
        printf(" Thread for writing_a created successfully\n");
    if (pthread_create(&thread_play_sound_id, NULL, &playing_sound, &passing_arg) != 0)
        printf("\nCan't create thread for reading.");
    else
        printf(" Thread for writing created successfully.\n");
    GtkApplication *app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), &passing_arg);
    int status = g_application_run(G_APPLICATION(app), 1, argv);
    g_object_unref(app);
    return status;
}