/**
* Copyright (C) 2013 Guangzhou Tronlong Electronic Technology Co., Ltd. - www.tronlong.com
* 
* @file display.cpp
* 
* @brief Display Module: update NV21 image from others and display as RGB image in own thread 
*
* @author Tronlong <support@tronlong.com>
* 
* @version V1.0
* 
* @date 2022-5-26
**/

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "display.h"

using namespace cv;

Display::Display(const char *dev, enum MODE mode, uint32_t w, uint32_t h) 
    : mode(mode), width(w), height(h), quit(false) {
    disp = disp_device_open(dev, mode, w, h, w, h);
    buf = (uint8_t *)malloc(width * height * 4);
    sem_init(&disp_sem, 0, 0); 
}

Display::~Display() {
    disp_device_release(disp);
    free(buf);
    sem_destroy(&disp_sem);
}

bool Display::start() {
    /* start display loop */
    pthread_create(&tid, NULL, run, this);

    return true;
}

bool Display::stop() {
    /* wakup to quit */
    quit = true; 
    sem_post(&disp_sem);

    /* wait to display thread exit */
    pthread_join(tid, NULL);

    return true;
}

bool Display::update(const uint8_t *buf_, uint32_t size) {
    /* update display buffer */
    memcpy(buf, buf_, size);

    /* wakup to display */
    sem_post(&disp_sem);

    return true;
}

void *Display::run(void *data) {
    Display *thiz = (Display *)data;
    Mat gray;
    Mat rgb;

    while (! thiz->quit) {
        /* wait for new buffer */
        sem_wait(&thiz->disp_sem);

        switch (thiz->mode) {
        case MODE_CAP_DISPLAY:
            /* NV21 image to display */
            disp_device_draw(thiz->disp, thiz->buf);
            break;

        case MODE_CAP_SOBEL:
            /* Gray8 convert RGB image to display */
            gray = Mat(thiz->height, thiz->width, CV_8UC1, thiz->buf);
            cvtColor(gray, rgb, COLOR_GRAY2RGB);

            disp_device_draw(thiz->disp, rgb.data);
            break;

        default:
            break;
        }
    }

    return NULL;
}
