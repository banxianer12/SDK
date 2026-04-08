#ifndef UDP_CAMERA_H
#define UDP_CAMERA_H

#include <stdint.h>
#include <pthread.h>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class UDPCamera {
public:
    UDPCamera(int udp_port);
    ~UDPCamera();
    
    bool start();
    bool stop();
    bool capture(uint8_t *buf, uint32_t *size);
    bool has_data();  // 简化实现，兼容旧版本

private:
    static void *bus_watch(void *data);
    void cleanup();

private:
    int port;
    volatile bool stop_requested;
    pthread_t tid;
    GstBus *bus;
    GstElement *pipeline;
    GstAppSink *sink;
};

#endif