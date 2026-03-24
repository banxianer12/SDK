/*
 * =====================================================================================
 *
 *       Filename:  log.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2020年04月15日 14时56分04秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jilinglin
 *        Company:  allwinnertech
 *
 * =====================================================================================
 */
#ifndef CDC_LOG_H
#define CDC_LOG_H

#include <linux/kernel.h>

#ifndef LOG_TAG
#define LOG_TAG "ve"
#endif

#define LOG_LEVEL_DEBUG   "debug"
#define LOG_LEVEL_INFO    "info"
#define LOG_LEVEL_WARNING "warn"
#define LOG_LEVEL_ERROR   "error"

#define CLOSE_LOG_LEVEL (0)

//linux kernel not support logv
#if CLOSE_LOG_LEVEL

#define logv(fmt, arg...)
#define logd(fmt, arg...)
#define logi(fmt, arg...)
#define logw(fmt, arg...)

#else

#define logv(fmt, arg...)
#define logd(fmt, arg...) printk(KERN_DEBUG"%s:%s <%s:%u>"fmt"\n",   \
		LOG_LEVEL_DEBUG, LOG_TAG, __FUNCTION__, __LINE__, ##arg)
#define logi(fmt, arg...) printk(KERN_INFO"%s:%s <%s:%u>"fmt"\n",    \
		LOG_LEVEL_INFO, LOG_TAG, __FUNCTION__, __LINE__, ##arg)
#define logw(fmt, arg...) printk(KERN_WARNING"%s:%s <%s:%u>"fmt"\n", \
		LOG_LEVEL_WARNING, LOG_TAG, __FUNCTION__, __LINE__, ##arg)
#define loge(fmt, arg...) printk(KERN_ERR"%s:%s <%s:%u>"fmt"\n",     \
		LOG_LEVEL_ERROR, LOG_TAG, __FUNCTION__, __LINE__, ##arg)

#endif

#endif
