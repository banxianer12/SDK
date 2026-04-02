/**
* Copyright (C) 2013 Guangzhou Tronlong Electronic Technology Co., Ltd. - www.tronlong.com
* 
* @file rtsp_camera.cpp
* 
* @brief RTSPCamera Module: capture H264 image from IP Camera RTSP stream
*
* @author Tronlong <support@tronlong.com>
* 
* @version V1.0
* 
* @date 2022-5-26
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rtsp_camera.h"

#define LAUNCH_FMT "rtspsrc location=%s ! rtph264depay ! video/x-h264, stream-format=byte-stream ! h264parse ! appsink name=appsink"

RTSPCamera::RTSPCamera(const char *url_) {
    url = strdup(url_);
    launch = (char *)malloc(256);
    snprintf(launch, 256, LAUNCH_FMT, url); 
}

RTSPCamera::~RTSPCamera() {
    free(url);
    free(launch);
}

bool RTSPCamera::start() {
    /* create pipeline */
    pipeline = gst_parse_launch(launch, NULL);

    /* get appsink */
    sink = (GstAppSink*)gst_bin_get_by_name(GST_BIN(pipeline), "appsink");
    gst_app_sink_set_wait_on_eos(sink, false);

    /* start pipeline */
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    /* start to watch bus message */
    pthread_create(&tid, NULL, bus_watch, this);

    g_print("RTSPCamera: start to capture video in %s\n", url);

    return true;
}

bool RTSPCamera::stop() {
    g_print("RTSPCamera: stop\n");

    /* send end-of-stream event to pipeline */
    gst_element_send_event(pipeline, gst_event_new_eos());

    /* wait watch bus thread exit */
    pthread_join(tid, NULL);

    /* release */
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return true;
}

bool RTSPCamera::capture(uint8_t *buf, uint32_t *size) {
    GstSample *sample;
    GstBuffer *buffer;
    GstMapInfo map;

    /* get buffer */
    sample = gst_app_sink_pull_sample(sink);
    if (! sample) {
        g_error("failed to pull sample");
        return false;
    }
    buffer = gst_sample_get_buffer(sample);

    /* map buffer */
    gst_buffer_map(buffer, &map, GST_MAP_READ);

    /* get h264 segment data */
    memcpy(buf, map.data, map.size);
    *size = map.size;

    /* release */
    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);

    return true;
}

void *RTSPCamera::bus_watch(void *data) {
    RTSPCamera *thiz = (RTSPCamera *)data;

    /* get pipeline bus */
    thiz->bus = gst_element_get_bus(thiz->pipeline);

    /* wait for message */
    GstMessage *msg = gst_bus_timed_pop_filtered(thiz->bus, 
            GST_CLOCK_TIME_NONE, 
            (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    /* show message */
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR:
        g_error("RTSPCamera: ERROR");
    case GST_MESSAGE_EOS:
        g_print("RTSPCamera: end of stream\n");
    default:
        ;
    }

    gst_message_unref (msg);

    return NULL;
}
