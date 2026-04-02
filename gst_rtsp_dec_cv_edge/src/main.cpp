/**
* Copyright (C) 2013 Guangzhou Tronlong Electronic Technology Co., Ltd. - www.tronlong.com
* 
* @file main.cpp
* 
* @brief Example application main file.
*   1. Capture H264 image from IP Camera RTSP stream
*   2. Decode H264 to NV21
*   3. Do sobel by OpenCV
*   4. Display image by Display device
*
* @author Tronlong <support@tronlong.com>
* 
* @version V1.0
* 
* @date 2023-7-21
**/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#include <gst/gst.h>

#include "parameter_parser.h"
#include "rtsp_camera.h"
#include "display.h"
#include "sobel.h"

#include "AWVideoDecoder.h"

#define DISP_DEVICE "/dev/disp"

using namespace awvideodecoder;

volatile bool g_quit = false;

typedef struct VideoDecode {
    uint32_t nv21_buf_size;
    uint8_t *nv21_buf;

    AWVideoDecoder *decoder;
} VideoDecode;

static void sig_handle(int signal) {
    g_quit = true;
}

void print_decode_fps() {
    int timeuse;
    static int frame_cnt = 0;
    static struct timeval start, end;

    if (frame_cnt == 0)
        gettimeofday( &start, NULL );

    frame_cnt++;

    gettimeofday( &end, NULL );
    timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec -start.tv_usec;

    /* print fps every second */
    if (timeuse > 1 * 1000 * 1000) {
        float fps = (frame_cnt - 1) / ((float)timeuse / 1000 / 1000);
        printf("decode fps:%.2f\n", fps);
        frame_cnt = 0;
    }
}

VideoDecode* video_decoder_create(DecodeParam *dec_param) {
    VideoDecode *vdecoder = (VideoDecode *)malloc(sizeof(VideoDecode));
    if (vdecoder == NULL) {
        perror("vdecoder malloc false.");
        return NULL;
    }

    /* alloc NV21 buffer to store decoded data.
     * allwiner decoder would fill more data then the actual rows in H264
     * stream after decode. so add 8 rows memory to carry the additional data
     * */
    vdecoder->nv21_buf_size = dec_param->srcW * (dec_param->srcH + 8) * 1.5;
    vdecoder->nv21_buf = (uint8_t *)malloc(vdecoder->nv21_buf_size);

    /* get the instance of AWVideoDecoder */
    vdecoder->decoder = AWVideoDecoder::create(dec_param);

    return vdecoder;
}

int video_decoder_decode(VideoDecode *vdecoder, void *input_buf, uint32_t input_buf_size) {
    AVPacket picBuf;

    /* h264 decode to NV21 image */
    if (vdecoder->decoder->decodeAsync(input_buf, input_buf_size) < 0) {
        g_print("decode failed, again\n");
        return -1;
    }
    memset(&picBuf, 0, sizeof(AVPacket));

    vdecoder->decoder->requestPicture(&picBuf);
    if (picBuf.handler == NULL)
        return -1;

    memcpy(vdecoder->nv21_buf, picBuf.pAddrVir0, picBuf.dataLen0);

    vdecoder->decoder->releasePicture(&picBuf);

    return 0;
}

void video_decoder_destory(VideoDecode *vdecoder) {
    if (vdecoder == NULL) {
        return;
    }

    AWVideoDecoder::destroy(vdecoder->decoder);

    free(vdecoder->nv21_buf);
    free(vdecoder);

    vdecoder = NULL;
}

int main(int argc, char *argv[]) {
    CmdLineParams params;
    DecodeParam dec_param;
    VideoDecode *vdecoder = NULL;

    uint32_t h264_buf_size;
    uint8_t *h264_buf = NULL;

    signal(SIGINT, sig_handle);

    gst_init(&argc, &argv);

    /* parse command line parameters */
    if (! parse_parameter(&params, argc, argv)) {
        g_error("invalid command line parameters\n");
        return -1;
    }

    /* to fetch data form IP Camera */
    RTSPCamera camera(params.url);

    /* to display Gray8 image in own thread */
    Display display(DISP_DEVICE, params.mode, params.width, params.height);

    /* to do soble in own thread and send output image to display */
    Sobel sobel(&display, params.width, params.height);

    /* allocal buffer to store h264 segment data from IP Camera */
    h264_buf_size = params.width * params.height * 1.5;
    h264_buf = (uint8_t *)malloc(h264_buf_size);

    /* create H264 decoder which would output NV21 format data */
    memset(&dec_param, 0, sizeof(dec_param));
    dec_param.srcW = params.width; 
    dec_param.srcH = params.height;
    dec_param.dstW = params.width;
    dec_param.dstH = params.height;
    dec_param.rotation = Angle_0;
    dec_param.scaleRatio = ScaleNone;
    dec_param.codecType = CODEC_H264;
    dec_param.pixelFormat = PIXEL_NV21;
    vdecoder = video_decoder_create(&dec_param);
    if (vdecoder == NULL) {
        goto release;
    }

    /* start to work */
    display.start();

    if (params.mode == MODE_CAP_SOBEL)
        sobel.start();

    camera.start();

    while (! g_quit) {
        /* fetch H264 segment data from IP Camera */
        if (! camera.capture(h264_buf, &h264_buf_size)) {
            g_error("capture failed");
            break;
        }

        /* decode to NV21 image */
        if (video_decoder_decode(vdecoder, h264_buf, h264_buf_size) < 0)
            continue;

        switch (params.mode) {
        case MODE_CAP_DISPLAY:
            display.update((uint8_t *)vdecoder->nv21_buf, vdecoder->nv21_buf_size);
            break;

        case MODE_CAP_SOBEL:
            /* do sobel process and send output image to display.
            * if sobel process is busy, the input image would be drop.
            * */
            sobel.process_and_display((uint8_t *)vdecoder->nv21_buf, vdecoder->nv21_buf_size);
            break;
        default:
            break;
        }
        /* print decode fps */
        print_decode_fps();
    }

release:
    /* release */
    camera.stop();

    if (params.mode == MODE_CAP_SOBEL)
        sobel.stop();
    
    display.stop();

    video_decoder_destory(vdecoder);
    free(h264_buf);

    return 0;
}
