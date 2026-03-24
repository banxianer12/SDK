/*
 * This confidential and proprietary software should be used
 * under the licensing agreement from Allwinner Technology.

 * Copyright (C) 2020-2030 Allwinner Technology Limited
 * All rights reserved.

 * Author:zhengwanyu <zhengwanyu@allwinnertech.com>

 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from Allwinner Technology Limited.
 */
#include <stdio.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>

#include <renderengine_uapi.h>
#include <drm_fourcc.h>
#include <ion.h>

//#define ON_SCREEN

struct ion_mem
{
	unsigned int phy;     /* physical address */
	unsigned char *virt;  /* virtual address */
	unsigned long size;   /* ion buffer size */
	int dmafd;            /* ion dmabuf fd */
	int handle;           /* ion handle */
};

int getDisplayScreenInfo(unsigned int *width, unsigned int *height)
{
	struct fb_var_screeninfo vinfo;
	int fd;

	fd = open("/dev/fb0", O_RDWR);
	if (fd < 0) {
		printf("failed to open dev/fb0\n");
		return -1;
	}

	memset(&vinfo, 0, sizeof(vinfo));
	if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
		printf("failed to get fb info\n");
		return -1;
	}

	*width = vinfo.xres;
	*height = vinfo.yres;

	printf("fbdev size:%dx%d\n", *width, *height);

	close(fd);
	return 0;
}

int loadFile(struct ion_mem *mem, const char *path)
{
	FILE *file;
	ssize_t n;

	file = fopen(path, "w+");
	if (file < 0) {
		printf("fopen %s err.\n", path);
		return -1;
	}

	n = fwrite(mem->virt, 1, mem->size, file);
	if (n < mem->size) {
		printf("fwrite is unormal, wrote totally:%d  but need:%ld\n",
			(unsigned int)n, mem->size);
	}

	fclose(file);

	return 0;
}


int sendForDisplay(struct ion_mem *mem)
{
	struct fb_var_screeninfo vinfo;
	int fd;
	unsigned char *fbbuf;

	fd = open("/dev/fb0", O_RDWR);
	if (fd < 0) {
		printf("failed to open dev/fb0\n");
		return 0;
	}

	memset(&vinfo, 0, sizeof(vinfo));
	if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
		printf("failed to get fb info\n");
		return 0;
	}

	if ((fbbuf = (unsigned char *)mmap(0, mem->size,
		PROT_READ | PROT_WRITE, MAP_SHARED,
		fd, 0)) == (void*) -1) {
		printf("fbdev map video error\n");
		return -1;
	}

//Note: This is only dma buffer transmission. so do NOT need to flush cache
	memcpy(fbbuf, mem->virt, mem->size);

	if (munmap(fbbuf, mem->size)) {
		printf("munmap failed\n");
		return -1;
	}

	close(fd);
	return 0;
}

static int ion_open(void)
{
	int ion_fd;

	ion_fd = open("/dev/ion", O_RDWR, 0);
	if (ion_fd <= 0) {
		printf("open ion failed!\n");
		return -1;
	}

	return ion_fd;
}

static void ion_close(int ion_fd)
{
	if (ion_fd > 0)
		close(ion_fd);
}

static int dma_alloc(int ion_fd, struct ion_mem *mem, unsigned int size)
{
	int ret;
	void *addrmap = NULL;
	struct ion_allocation_data sAllocInfo;
	struct ion_fd_data data;

	/*For the performance reason, we are suggested to use Manual Cache*/
	sAllocInfo.len		= size;
	sAllocInfo.align	= 4096;
	//sAllocInfo.heap_id_mask	= ION_HEAP_CARVEOUT_MASK;
	//sAllocInfo.heap_id_mask = ION_HEAP_SYSTEM_CONTIG_MASK;
	sAllocInfo.heap_id_mask = ION_HEAP_SYSTEM_MASK;
	//sAllocInfo.flags		= 0;
	sAllocInfo.flags                = ION_FLAG_CACHED; //Auto Cache, for Test
	//sAllocInfo.flags      = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC; //Manual Cache
	sAllocInfo.handle     = 0,

	ret = ioctl(ion_fd, ION_IOC_ALLOC, &sAllocInfo);
	if (ret < 0) {
		printf("alloc ion err.\n");
		printf("failed to alloc: ionfd:%d  size:%d\n", ion_fd, size);
		close(ion_fd);
		return  -1;
	}

	//printf("succeed to alloc: ionfd:%d  size:%d\n", ion_fd, size);

	data.handle = sAllocInfo.handle;
	ret = ioctl(ion_fd, ION_IOC_SHARE, &data);
	//ret = ioctl(ion_fd, ION_IOC_MAP, &data);
	if (ret < 0)
		return ret;

	if (data.fd < 0) {
		printf("ION_IOC_MAP failed!\n");
		ioctl(ion_fd, ION_IOC_FREE, &sAllocInfo);
		return -1;
	}

	/* mmap to user */
	addrmap = mmap(NULL, sAllocInfo.len, PROT_READ | PROT_WRITE, MAP_SHARED, data.fd, 0);
	if(MAP_FAILED == addrmap) {
		printf("mmap err!\n");
		addrmap = NULL;
		ioctl(ion_fd, ION_IOC_FREE, &sAllocInfo);
		return -1;
	}
	memset(addrmap, 0, sAllocInfo.len);

	mem->phy = 0;
	mem->virt = (unsigned char *)addrmap;
	mem->size = size;
	mem->dmafd = data.fd;
	mem->handle = sAllocInfo.handle;

	return data.fd;
}

void ion_free(int ion_fd, struct ion_mem *mem)
{
	struct ion_handle_data handle_data;

	if (mem->virt) {
		munmap((void*)mem->virt, mem->size);
		mem->virt = 0;
	}

	if (mem->dmafd > 0) {
		close(mem->dmafd);
		mem->dmafd = -1;
	}

	if (mem->handle > 0) {
		handle_data.handle = mem->handle;
		ioctl(ion_fd, ION_IOC_FREE, &handle_data);
		mem->handle = 0;
	}
}

int
main(int argc, char *argv[])
{
	int ret;
	void *addr_file;
	int file_fd;
	unsigned int in_size, in_width, in_height;
	char *in_file;
	unsigned int in_format, out_format;
#ifndef ON_SCREEN
	unsigned int out_width, out_height;
	void *sync;
#endif
	int ion_fd;
	struct ion_mem in_mem, out_mem;

	ion_fd = ion_open();
	if (ion_fd < 0) {
		printf("ion open failed\n");
		return -1;
	}

//!!!NOTE, before run this demo,please fill the parameters belows
	in_size = 1920 * 1080 * 3 / 2;
	in_file = "./pic_bin/1080_1920x1080.nv21";
	in_width = 1920;
	in_height = 1080;
	in_format = DRM_FORMAT_NV21;

	struct RE_rect crop = {
		.left = 0,
		.top = 0,
		.right = 1920,
		.bottom = 1080,
	};


	struct RE_rect dstWinRect = {
		.left = 550,
		.top = 200,
		.right = 750,
		.bottom = 400,
	};

	struct RE_rect dstWinRect1 = {
		.left = 200,
		.top = 100,
		.right = 600,
		.bottom = 700,
	};



#ifdef ON_SCREEN
	out_format = DRM_FORMAT_ARGB8888;
	RenderEngine_t render = renderEngineCreate(true, 1,
				in_format, out_format);
#else
	getDisplayScreenInfo(&out_width, &out_height);
	out_format = DRM_FORMAT_NV21;
	//create out memory
	ret = dma_alloc(ion_fd, &out_mem, out_width * out_height * 3 / 2);
	if (ret < 0) {
		printf("dma alloc for out memory\n");
		return ret;
	}
	RenderEngine_t render = renderEngineCreate(false, 1,
					in_format, out_format);
	renderEngineSetOffScreenTarget(render,
				out_width,
				out_height,
				out_mem.dmafd,
				0);
#endif

	float degree = 0.0f;

	while(1) {
//create in memory
		ret = dma_alloc(ion_fd, &in_mem, in_size);
		if (ret < 0) {
			printf("dma alloc for in memory\n");
			return ret;
		}

		file_fd = open(in_file, O_RDWR);
		if (file_fd < 0) {
			printf("open %s  err.\n", in_file);
			return -1;
		}
		addr_file = (void *)mmap((void *)0, in_size,
					 PROT_READ | PROT_WRITE, MAP_SHARED,
					 file_fd, 0);
		memcpy(in_mem.virt, addr_file , in_size);
		munmap(addr_file, in_size);
		close(file_fd);

		struct RESurface surface;

		surface.srcFd = in_mem.dmafd;
		surface.srcDataOffset = 0;
		surface.srcWidth = in_width;
		surface.srcHeight = in_height;
		memcpy(&surface.srcCrop, &crop, sizeof(crop));
		memcpy(&surface.dstWinRect, &dstWinRect, sizeof(dstWinRect));
		surface.rotateAngle = degree;

		renderEngineSetSurface(render, &surface);


		struct RESurface surface1;

		surface1.srcFd = in_mem.dmafd;
		surface1.srcDataOffset = 0;
		surface1.srcWidth = in_width;
		surface1.srcHeight = in_height;
		memcpy(&surface1.srcCrop, &crop, sizeof(crop));
		memcpy(&surface1.dstWinRect, &dstWinRect1, sizeof(dstWinRect1));

		renderEngineSetSurface(render, &surface1);


#ifdef ON_SCREEN
		ret = renderEngineDrawOnScreen(render);
		if (ret < 0) {
			printf("drawLayer failed\n");
			return -1;
		}
#else
		ret = renderEngineDrawOffScreen(render, &sync);
		if (ret < 0) {
			printf("drawLayer failed\n");
			return -1;
		}

		ret = renderEngineWaitSync(render, sync);
		if (ret < 0) {
			printf("waitSync failed\n");
			return -1;
		}

		loadFile(&out_mem, "1280x800.nv21");
		//sendForDisplay(&out_mem);
	
		/* NOTE!!! Must wait for finishing displaying the layer,
		 * Or will cause screen tearing*/
		usleep(16667);
#endif
		ion_free(ion_fd,&in_mem);

		getchar();
		degree += 45.0f;

		if (degree == 360.0f)
			degree = 0.0f;
	}


	ion_free(ion_fd,&out_mem);
	renderEngineDestroy(render);

	ion_close(ion_fd);

	printf("render demo exit\n");
	return 0;
}
