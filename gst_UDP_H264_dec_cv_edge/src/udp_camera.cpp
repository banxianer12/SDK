#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "udp_camera.h"

// 关键变化：去掉 RTP 相关元素，直接收裸 H264
#define LAUNCH_FMT "udpsrc port=%d buffer-size=4194304 ! " \
                   "h264parse ! " \
                   "video/x-h264,stream-format=byte-stream ! " \
                   "appsink name=appsink"

UDPCamera::UDPCamera(int udp_port) {
    port = udp_port;
    stop_requested = false;
    bus = NULL;
    pipeline = NULL;
    sink = NULL;
    tid = 0;
}

UDPCamera::~UDPCamera() {
    cleanup();
}

void UDPCamera::cleanup() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline = NULL;
    }
    if (bus) {
        gst_object_unref(bus);
        bus = NULL;
    }
    sink = NULL;
}

bool UDPCamera::start() {
    GError *error = NULL;
    gchar *launch_str = g_strdup_printf(LAUNCH_FMT, port);
    g_print("UDP Pipeline: %s\n", launch_str);
    
    pipeline = gst_parse_launch(launch_str, &error);
    g_free(launch_str);
    
    if (!pipeline) {
        g_error("Failed to create pipeline: %s", error ? error->message : "unknown");
        if (error) g_error_free(error);
        return false;
    }

    sink = (GstAppSink*)gst_bin_get_by_name(GST_BIN(pipeline), "appsink");
    if (!sink) {
        g_error("Failed to get appsink");
        cleanup();
        return false;
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    stop_requested = false;
    pthread_create(&tid, NULL, bus_watch, this);

    g_print("UDPCamera: listening on UDP port %d (raw H264)\n", port);
    return true;
}

bool UDPCamera::stop() {
    g_print("UDPCamera: stopping...\n");
    stop_requested = true;
    
    if (pipeline) {
        gst_element_send_event(pipeline, gst_event_new_eos());
    }
    
    if (tid) {
        pthread_join(tid, NULL);
        tid = 0;
    }
    
    cleanup();
    g_print("UDPCamera: stopped\n");
    return true;
}

bool UDPCamera::capture(uint8_t *buf, uint32_t *size) {
    GstSample *sample;
    GstBuffer *buffer;
    GstMapInfo map;

    sample = gst_app_sink_pull_sample(sink);
    if (!sample) {
        return false;
    }
    
    buffer = gst_sample_get_buffer(sample);
    gst_buffer_map(buffer, &map, GST_MAP_READ);

    memcpy(buf, map.data, map.size);
    *size = map.size;

    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);

    return true;
}

void *UDPCamera::bus_watch(void *data) {
    UDPCamera *thiz = (UDPCamera *)data;
    thiz->bus = gst_element_get_bus(thiz->pipeline);
    
    while (!thiz->stop_requested) {
        GstMessage *msg = gst_bus_timed_pop_filtered(thiz->bus, 
                100 * GST_MSECOND,
                (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
        
        if (msg) {
            switch (GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_ERROR: {
                GError *err = NULL;
                gchar *debug = NULL;
                gst_message_parse_error(msg, &err, &debug);
                g_error("UDPCamera ERROR: %s", err ? err->message : "unknown");
                if (err) g_error_free(err);
                if (debug) g_free(debug);
                thiz->stop_requested = true;
                break;
            }
            case GST_MESSAGE_EOS:
                g_print("UDPCamera: end of stream\n");
                thiz->stop_requested = true;
                break;
            default:
                break;
            }
            gst_message_unref(msg);
        }
    }
    
    return NULL;
}