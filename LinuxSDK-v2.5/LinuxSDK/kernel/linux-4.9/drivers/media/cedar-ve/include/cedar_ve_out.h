/*
 * =====================================================================================
 *
 *       Filename:  cedar_ve_out.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2020年04月18日 10时51分06秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *        Company:
 *
 * =====================================================================================
 */
#ifndef CEDAR_VE_OUT
#define CEDAR_VE_OUT

#include <linux/device.h>


void cedar_open(void);
void cedar_close(void);
void cedar_reset_ve(void);
char *cedar_get_ve_base_addr(void);
int cedar_wait_interrupt(int dec_flag, int timeout);
struct device *cedar_get_device(void);
int cedar_set_ve_freq(int freq);
#endif
