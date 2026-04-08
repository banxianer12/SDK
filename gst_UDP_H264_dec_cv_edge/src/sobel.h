#ifndef SOBEL_H
#define SOBEL_H

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#include "display.h"

class Sobel {
public:    
    Sobel(Display *display, uint32_t w, uint32_t h);
    ~Sobel();

    bool start();
    bool stop();

    bool process_and_display(const uint8_t *buf, uint32_t size);

private:
    static void *run(void *data);

private:
    pthread_t tid;
    sem_t sobel_sem;

    Display *display;

    uint32_t width;
    uint32_t height;
    uint8_t *buf;

    volatile bool quit;

    volatile bool busy;
};

#endif
