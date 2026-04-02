/* Copyright 2022 Tronlong Elec. Tech. Co. Ltd. All Rights Reserved. */

#ifndef RTSP_CAMERA_H
#define RTSP_CAMERA_H

#include <stdint.h>
#include <pthread.h>
#include <string>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class RTSPCamera {
public:
    RTSPCamera(const char *url_);
    ~RTSPCamera();
    
    bool start();
    bool stop();

    bool capture(uint8_t *buf, uint32_t *size);

private:
    static void *bus_watch(void *data);

private:
    char *launch;
    char *url;

    pthread_t tid;

    GstBus *bus;
    GstElement *pipeline;
    GstAppSink *sink;
};

#endif
