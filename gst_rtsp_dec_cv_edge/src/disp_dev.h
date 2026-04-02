#ifndef DISP_DEV_H
#define DISP_DEV_H

#include <stdint.h>
#include <unistd.h>

#include "parameter_parser.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct _DispDevice DispDevice;

DispDevice *disp_device_open(const char *dev_name, enum MODE mode, uint32_t w,
                             uint32_t h, uint32_t disp_w, uint32_t disp_h);
DispDevice *disp_device_draw(DispDevice *disp_device, const void *addr);
void disp_device_release(DispDevice *disp_device);

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* DISP_CONTROL_H */
