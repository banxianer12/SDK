/**
* Copyright (C) 2013 Guangzhou Tronlong Electronic Technology Co., Ltd. - www.tronlong.com
* 
* @file soble.cpp
* 
* @brief Sobel Module: 
*   1. update NV21 image from others 
*   2. do soble and update display in own thread
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

#include "sobel.h"

using namespace std;
using namespace cv;

Sobel::Sobel(Display *display_, uint32_t w, uint32_t h) 
    : display(display_), width(w), height(h), quit(false), busy(false) {
    buf = (uint8_t *)malloc(width * (height + 8) * 1.5); 
    sem_init(&sobel_sem, 0, 0); 
}

Sobel::~Sobel() {
    free(buf);
    sem_destroy(&sobel_sem);
}

bool Sobel::start() {
    /* start sobel process loop */
    pthread_create(&tid, NULL, run, this);

    return true;
}

bool Sobel::stop() {
    /* wackup to quit process loop */
    quit = true;
    sem_post(&sobel_sem);

    /* wait process loop exit */
    pthread_join(tid, NULL);

    return true;
}

bool Sobel::process_and_display(const uint8_t *buf_, uint32_t size) {
    /* do nothing if busy */
    if (busy) 
        return false;

    /* update image for next process */
    memcpy(buf, buf_, size);

    /* wakeup to do sobel */
    sem_post(&sobel_sem);

    return true;
}

void *Sobel::run(void *data) {
    Sobel *thiz = (Sobel *)data;
    Mat input;
    Mat output;
    Mat X, Y;

    int timeuse;
    static struct timeval start, end;

    while (! thiz->quit) {
        /* wait new buffer */
        sem_wait(&thiz->sobel_sem);

        thiz->busy = true;

        gettimeofday(&start, NULL);

        /* do sobel */
        input = Mat(thiz->height, thiz->width, CV_8UC1, thiz->buf);
        cv::Sobel(input, X, -1, 1, 0);
        cv::Sobel(input, Y, -1, 0, 1);
        addWeighted(X, 0.5, Y, 0.5, 0, output);

        /* show soble process time */
        gettimeofday(&end, NULL);
        timeuse = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
        printf("sobel: %d us\n", timeuse);

        /* try to update display */
        thiz->display->update(output.data, thiz->width * thiz->height);

        thiz->busy = false;
    }

    return NULL;
}
