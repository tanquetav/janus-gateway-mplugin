#ifndef RECORDER_H
#define RECORDER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "libavutil/mathematics.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

/* 5 seconds stream duration */
#define STREAM_DURATION   5.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_NB_FRAMES  ((int)(STREAM_DURATION * STREAM_FRAME_RATE))
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */

struct TStream {
AVStream * audio_st;
float t;
float tincr;
float tincr2;
int16_t *samples;
int audio_input_frame_size;
int pts;
int desloc;
} ;
typedef struct TStream TStream;

struct TData {
AVOutputFormat *fmt;
AVFormatContext *oc;
TStream **streams;
int qtparticipants;
uint64_t *userids;
int samplerate;
int samples;
};
typedef struct TData TData;


int dostart();
TData* dofirst(int qtMax,int samplerate,int samples,char * record_path);
int domiddle(TData* data);
int domiddle2(TData* data,int pos ,int16_t *ssamples,int samples);
int doend(TData* data);
void close_audio(AVFormatContext *oc, AVStream *st,TStream* stream);
void write_audio_frame(AVFormatContext *oc, AVStream *st,TStream* stream);
void get_audio_frame(int16_t *samples, int frame_size, int nb_channels,TStream* stream);
void open_audio(AVFormatContext *oc, AVStream *st,TStream* stream,int samples);

#endif // RECORDER_H

