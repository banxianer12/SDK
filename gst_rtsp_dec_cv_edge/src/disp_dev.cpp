/**
 * Copyright (C) 2013 Guangzhou Tronlong Electronic Technology Co., Ltd. - www.tronlong.com
 *
 * @file disp_dev.cpp
 *
 * @brief disp_dev module: Output the image from decoder buffer to display device.
 *
 * @author Tronlong <support@tronlong.com>
 *
 * @version V1.0
 *
 * @date 2023-7-21
 **/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdbool.h>

/* disp SDK header file */
#include "hwdisp2.h"
#include "sunxiMemInterface.h"

#include "disp_dev.h"

using namespace android;

struct _DispDevice {
    uint32_t     fd;
    uint32_t     screen_id;
    uint32_t     channel;
    uint32_t     layer_id;
    struct src_info src_info;
    struct view_info disp_frame;
    struct view_info rect;
    struct view_info crop;
    HwDisplay* g_disp;
    dma_mem_des_t disp_mem;
};

typedef struct {
    uint32_t type;
    const char *name;
} DispOutputType;

DispOutputType disp_out_type[]={
    {DISP_OUTPUT_TYPE_LCD,  "LCD"},
    {DISP_OUTPUT_TYPE_TV ,  "TV"},
    {DISP_OUTPUT_TYPE_HDMI, "HDMI"},
    {DISP_OUTPUT_TYPE_VGA,  "VGA"},
};

static int disp_device_alloc(dma_mem_des_t* mem, int size) {
    int iRet = 0;

    iRet = allocOpen(MEM_TYPE_DMA, mem, NULL);
    if (iRet < 0) {
        printf("ion_alloc_open failed\n");
        return iRet;
    }
    mem->size = size;

    iRet = allocAlloc(MEM_TYPE_DMA, mem, NULL);
    if (iRet < 0) {
        printf("allocAlloc failed\n");
        return iRet;
    }
    printf("mem.vir=%p mem.phy=%p dmafd=%d,alloc len=%d\n", mem->vir,
           mem->phy, mem->ion_buffer.fd_data.aw_fd, mem->size);

    return 0;
}

static int disp_device_free(dma_mem_des_t* mem) {
    allocFree(MEM_TYPE_DMA, mem, NULL);

    return 0;
}

DispDevice *disp_device_open(const char *dev_name, enum MODE mode, uint32_t w, uint32_t h,
                             uint32_t disp_w, uint32_t disp_h) {
    int screen_id = 0, i = 0;
    enum disp_output_type output_type = DISP_OUTPUT_TYPE_NONE;
    unsigned int arg[3];
    int ret = 0;

    DispDevice *disp_device = (DispDevice *)malloc(sizeof(DispDevice));

    disp_device->fd = open(dev_name, O_RDWR);
    if (disp_device->fd < 0) {
        perror("Error: cannot open hwdisplay device.");
        return NULL;
    }

    /* Get current output type */
    for (screen_id = 0; screen_id < 4; screen_id++) {
        arg[0] = screen_id;

        output_type = (enum disp_output_type)ioctl(disp_device->fd, DISP_GET_OUTPUT_TYPE, (void *)arg);
        if (output_type != DISP_OUTPUT_TYPE_NONE) {
            for (i = 0; i < (sizeof(disp_out_type) / sizeof(disp_out_type[0])); i++) {
                if (disp_out_type[i].type == output_type) {
                    printf("Output type: %s\n", disp_out_type[i].name);
                }
            }
            break;
        }
    }
    close(disp_device->fd);

    disp_device->screen_id = screen_id;
    disp_device->channel = 0;
    disp_device->layer_id = 0;
    disp_device->disp_frame = {0, 0, w, h};
    disp_device->crop = {0, 0, w, h};

    /* Screen display area */
    disp_device->rect = {0, 0, disp_w, disp_h};

    /* Get the disp driver device object */
    disp_device->g_disp = HwDisplay::getInstance();

    switch (mode) {
    case MODE_CAP_DISPLAY:
        disp_device->src_info = {w, h + 8, DISP_FORMAT_YUV420_SP_VUVU};

        /**
         * Alloc a DMA buffer to store NV21 data
         * The decoded NV21 image has 8 lines of extra 0 data per frame
         **/ 
        ret = disp_device_alloc(&disp_device->disp_mem, w * (h + 8) * 3 / 2);
        if (ret < 0) {
            printf("disp_device_alloc failed.\n");
            return NULL;
        }
        break;
    case MODE_CAP_SOBEL:
        disp_device->src_info = {w, h, DISP_FORMAT_RGB_888};

        /* Alloc a DMA buffer to store RGB data */
        ret = disp_device_alloc(&disp_device->disp_mem, w * h * 3);
        if (ret < 0) {
            printf("disp_device_alloc failed.\n");
            return NULL;
        }
        break;
    default:
        break;
    }
    
    memset((void*)disp_device->disp_mem.vir, 0, disp_device->disp_mem.size);

    return disp_device;
}

DispDevice *disp_device_draw(DispDevice *disp_device, const void *addr) {
    int layer;

    memcpy((void *)disp_device->disp_mem.vir, addr, disp_device->disp_mem.size);

    layer = disp_device->g_disp->aut_hwd_layer_request(&disp_device->disp_frame, disp_device->screen_id,
                                                        disp_device->channel, disp_device->layer_id);

    disp_device->g_disp->aut_hwd_layer_sufaceview(layer, &disp_device->rect);

    disp_device->g_disp->aut_hwd_layer_set_src(layer, &disp_device->src_info, 0,
                        disp_device->disp_mem.ion_buffer.fd_data.aw_fd);

    disp_device->g_disp->aut_hwd_layer_set_rect(layer, &disp_device->crop);

    disp_device->g_disp->aut_hwd_layer_set_zorder(layer, 10);
    disp_device->g_disp->aut_hwd_layer_set_alpha(layer, 1, 255);

    /* Display layer */
    disp_device->g_disp->aut_hwd_layer_open(layer);

    return 0;
}

void disp_device_release(DispDevice *disp_device) {
    int layer;
    layer = disp_device->g_disp->aut_hwd_layer_request(&disp_device->disp_frame, disp_device->screen_id,
                                                        disp_device->channel, disp_device->layer_id);
    disp_device->g_disp->aut_hwd_layer_close(layer);

    disp_device_free(&disp_device->disp_mem);

    free(disp_device);
    disp_device = NULL;
}
