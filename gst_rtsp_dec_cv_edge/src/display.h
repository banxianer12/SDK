#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#include "disp_dev.h"
#include "parameter_parser.h"

class Display {
public:    
    Display(const char *dev, enum MODE mode, uint32_t w, uint32_t h);
    ~Display();

    bool start();
    bool stop();

    bool update(const uint8_t *buf_, uint32_t size);

private:
    static void *run(void *data);

private:
    pthread_t tid;
    sem_t disp_sem;

    DispDevice *disp;

    uint32_t width;
    uint32_t height;
    uint8_t *buf;

    enum MODE mode;

    volatile bool quit;
};

#endif
