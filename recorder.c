/*
 * Copyright (c) 2003 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * libavformat API example.
 *
 * @example output.c
 * Output a media file in any supported libavformat format. The default
 * codecs are used.
 */


#include "recorder.h"
#include <time.h>

static int sws_flags = SWS_BICUBIC;

/**************************************************************/
/* audio output */

static int16_t *samples;

/*
 * add an audio output stream
 */
AVStream *add_audio_stream(AVFormatContext *oc, enum AVCodecID codec_id, int samplerate)
{
    AVCodecContext *c;
    AVStream *st;
    AVCodec *codec=NULL;

    /* find the audio encoder */
    codec = avcodec_find_encoder(codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    st = avformat_new_stream(oc, codec);
    if (!st) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }

    c = st->codec;
    c->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    /* put sample parameters */
    c->sample_fmt  = AV_SAMPLE_FMT_S16;
    c->bit_rate    = 128000;
    c->sample_rate = samplerate;//48000;
    c->channels    = 1;
    c->channel_layout = av_get_default_channel_layout(1);
//    c->profile = FF_PROFILE_AAC_LOW;


    // some formats want stream headers to be separate
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

void open_audio(AVFormatContext *oc, AVStream *st,TStream* stream,int samples)
{
    AVCodecContext *c;

    c = st->codec;

    /* open it */
    if (avcodec_open2(c, NULL, NULL) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }

    stream->pts = 0;
    /* init signal generator */
    stream->t     = 0;
    stream->tincr = 2 * M_PI * 110.0 / c->sample_rate;
    /* increment frequency by 110 Hz per second */
    stream->tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;

    if (c->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE)
        stream->audio_input_frame_size = samples;
    else
        stream->audio_input_frame_size = c->frame_size;
    stream->samples = av_malloc(stream->audio_input_frame_size *
                        av_get_bytes_per_sample(c->sample_fmt) *
                        c->channels);
}

/* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
 * 'nb_channels' channels. */
void get_audio_frame(int16_t *samples, int frame_size, int nb_channels,TStream* stream)
{
    int j, i, v;
    int16_t *q;

    q = stream->samples;

    if ( stream->pts < stream->desloc ) {
        for (j = 0; j < frame_size; j++) {

            for (i = 0; i < nb_channels; i++)
                *q++ = 0;
        }
        return;
    }

    for (j = 0; j < frame_size; j++) {
        v = (int)(sin(stream->t) * 10000);
        for (i = 0; i < nb_channels; i++)
            *q++ = v;
        stream->t     += stream->tincr;
        stream->tincr += stream->tincr2;
    }
}

void write_audio_frame(AVFormatContext *oc, AVStream *st,TStream* stream)
{
    AVCodecContext *c;
    AVPacket pkt = { 0 }; // data and size must be 0;
    AVFrame *frame = av_frame_alloc();
    int got_packet;

    av_init_packet(&pkt);
    c = st->codec;

    stream->pts++;
//    frame->pts =40*( stream->desloc +stream->pts++);

    get_audio_frame(samples, stream->audio_input_frame_size, c->channels,stream);
    frame->nb_samples = stream->audio_input_frame_size;
    avcodec_fill_audio_frame(frame, c->channels, c->sample_fmt,
                             (uint8_t *)stream->samples,
                             stream->audio_input_frame_size *
                             av_get_bytes_per_sample(c->sample_fmt) *
                             c->channels, 1);

    avcodec_encode_audio2(c, &pkt, frame, &got_packet);
    if (!got_packet)
        return;

    pkt.stream_index = st->index;

    /* Write the compressed frame to the media file. */
    if (av_interleaved_write_frame(oc, &pkt) != 0) {
        fprintf(stderr, "Error while writing audio frame\n");
        exit(1);
    }
    av_frame_free(&frame);
}
void write_audio_frame2(AVFormatContext *oc, AVStream *st,TStream* stream,int16_t *ssamples,int samples)
{
    AVCodecContext *c;
    AVPacket pkt = { 0 }; // data and size must be 0;
    AVFrame *frame = av_frame_alloc();
    int got_packet;

    av_init_packet(&pkt);
    c = st->codec;

    stream->pts++;
//    frame->pts =40*( stream->desloc +stream->pts++);

//    get_audio_frame(ssamples, stream->audio_input_frame_size, c->channels,stream);
    frame->nb_samples = stream->audio_input_frame_size;
    avcodec_fill_audio_frame(frame, c->channels, c->sample_fmt,
                             (uint8_t *)ssamples,
                             stream->audio_input_frame_size *
                             av_get_bytes_per_sample(c->sample_fmt) *
                             c->channels, 1);

    avcodec_encode_audio2(c, &pkt, frame, &got_packet);
    if (!got_packet)
        return;

    pkt.stream_index = st->index;

    /* Write the compressed frame to the media file. */
    if (av_interleaved_write_frame(oc, &pkt) != 0) {
        fprintf(stderr, "Error while writing audio frame\n");
        exit(1);
    }
    av_frame_free(&frame);
}

void close_audio(AVFormatContext *oc, AVStream *st,TStream* stream)
{
    avcodec_close(st->codec);

    av_free(stream->samples);
}

/**************************************************************/
/* video output */

/**************************************************************/
/* media file output */

int dostart()
{
    /* Initialize libavcodec, and register all codecs and formats. */
    av_register_all();
    return 0;
}



TData * dofirst(int qtMax,int samplerate,int samples,char * record_path)
{
    int i;
    TData *data = malloc(sizeof(TData));
    data->qtparticipants = qtMax;
    data->streams = malloc(data->qtparticipants * sizeof(TStream*));
    data->userids = malloc(data->qtparticipants * sizeof(uint64_t));
    data->samplerate = samplerate;
    data->samples=samples;

    char filename[255];
    char temp[255];
    if ( record_path!=NULL) {
        time_t rawtime;
        struct tm * timeinfo;

        time (&rawtime);
        timeinfo = localtime (&rawtime);

        strftime(temp,255,"%F-%H-%M-%S",timeinfo);
        sprintf(filename,"%s/%s.mkv",record_path,temp);
    }
    else {
        sprintf(filename,"saida.mkv");
    }

    /* Autodetect the output format from the name. default is MPEG. */
    data->fmt = av_guess_format(NULL, filename, NULL);
    if (!data->fmt) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        data->fmt = av_guess_format("mpeg", NULL, NULL);
    }
    if (!data->fmt) {
        fprintf(stderr, "Could not find suitable output format\n");
        return NULL;
    }

    /* Allocate the output media context. */
    data->oc = avformat_alloc_context();
    if (!data->oc) {
        fprintf(stderr, "Memory error\n");
        return NULL;
    }
    data->oc->oformat = data->fmt;
    snprintf(data->oc->filename, sizeof(data->oc->filename), "%s", filename);

    /* Add the audio and video streams using the default format codecs
     * and initialize the codecs. */
    //video_st = NULL;
    for ( i = 0 ; i< data->qtparticipants;i++) {
        data->streams[i] = malloc(sizeof(TStream));
        data->streams[i]->audio_st = NULL;
    }

    data->streams[0]->desloc = 0;
    if (qtMax>1)
        data->streams[1]->desloc = 6;
    if (qtMax>2)
        data->streams[2]->desloc = 13;
    //    if (fmt->video_codec != AV_CODEC_ID_NONE) {
//        video_st = add_video_stream(oc, fmt->video_codec);
//    }
    if (data->fmt->audio_codec != AV_CODEC_ID_NONE) {
        for (i = 0 ; i< data->qtparticipants;i++)
            data->streams[i]->audio_st = add_audio_stream(data->oc, AV_CODEC_ID_FIRST_AUDIO/*fmt->audio_codec*/,samplerate);
    }

    /* Now that all the parameters are set, we can open the audio and
     * video codecs and allocate the necessary encode buffers. */
//    if (video_st)
//        open_video(oc, video_st);
    if (data->streams[0]->audio_st) {
        for (i = 0 ; i< data->qtparticipants;i++)
            open_audio(data->oc, data->streams[i]->audio_st,data->streams[i],samples);
   }

    av_dump_format(data->oc, 0, filename, 1);

    /* open the output file, if needed */
    if (!(data->fmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&data->oc->pb, filename, AVIO_FLAG_WRITE) < 0) {
            fprintf(stderr, "Could not open '%s'\n", filename);
            return NULL;
        }
    }

    /* Write the stream header, if any. */
    avformat_write_header(data->oc, NULL);
    return data;
}
int domiddle2(TData* data,int i ,int16_t *ssamples,int samples)
{
//    write_audio_frame(data->oc, data->streams[i]->audio_st,data->streams[i]);
    write_audio_frame2(data->oc, data->streams[i]->audio_st,data->streams[i],ssamples,samples);

}
int domiddle(TData *data)
{
    int i , j ;
    int audio_pts ;
    for (j =0 ;j<500;j++) {
        /* Compute current audio and video time. */
        for (i = 0 ; i< data->qtparticipants;i++)
            if (data->streams[i]->audio_st)
                audio_pts = (double)data->streams[i]->audio_st->pts.val * data->streams[i]->audio_st->time_base.num / data->streams[i]->audio_st->time_base.den;
/*
        if (video_st)
            video_pts = (double)video_st->pts.val * video_st->time_base.num /
                        video_st->time_base.den;
        else
            video_pts = 0.0;
*/
        if ((!data->streams[0]->audio_st || audio_pts >= STREAM_DURATION) /*&&
            (!video_st || video_pts >= STREAM_DURATION)*/)
            break;

        /* write interleaved audio and video frames */
  //      if (!video_st || (video_st && audio_st && audio_pts < video_pts)) {
        for (i = 0 ; i< data->qtparticipants;i++) {
            if ( j> data->streams[i]->desloc) {
                write_audio_frame(data->oc, data->streams[i]->audio_st,data->streams[i]);
            }
        }
//       write_audio_frame(oc, audio_st2);
 //       write_audio_frame(oc, audio_st3);
  //      } else {
//            write_video_frame(oc, video_st);
    //    }
    }
}
int doend(TData *data)
{
    int i ;

    /* Write the trailer, if any. The trailer must be written before you
     * close the CodecContexts open when you wrote the header; otherwise
     * av_write_trailer() may try to use memory that was freed on
     * av_codec_close(). */
    av_write_trailer(data->oc);

    /* Close each codec. */
//    if (video_st)
//        close_video(oc, video_st);
    if (data->streams[i]->audio_st) {
        for (i = 0 ; i< data->qtparticipants;i++)
            close_audio(data->oc, data->streams[i]->audio_st,data->streams[i]);
    }

    /* Free the streams. */
    for (i = 0; i < data->oc->nb_streams; i++) {
        av_freep(&data->oc->streams[i]->codec);
        av_freep(&data->oc->streams[i]);
    }

    if (!(data->fmt->flags & AVFMT_NOFILE))
        /* Close the output file. */
        avio_close(data->oc->pb);

    /* free the stream */
    av_free(data->oc);

    for ( i = 0 ; i< data->qtparticipants;i++) {
        free (data->streams[i]);
    }
    free(data->userids);
    free(data->streams);
    free(data);

    printf("Free\n");

    return 0;
}
