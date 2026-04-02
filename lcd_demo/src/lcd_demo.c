/* Copyright 2018 Tronlong Elec. Tech. Co. Ltd. All Rights Reserved. */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <signal.h>
#include <getopt.h>
#include <stdbool.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define DEFAULT_FB_DEV          "/dev/fb0"

#define DISP_LCD_FB0  0x0
#define DISP_LCD_SET_BRIGHTNESS  0x102
#define DISP_LCD_GET_BRIGHTNESS  0x103
#define DISP_LCD_BRIGHTNESS_MAX  255

#define INADEQUATE_CONDITIONS 10

typedef struct {
    char red;
    char green;
    char blue;
    char trans;
} PIXEL;

/* Exit flag */
volatile bool g_quit = false;

/* Short option names */
static const char g_shortopts [] = "vh";

/* Option names */
static const struct option g_longopts [] = {
    { "version",     no_argument,            NULL,        'v' },
    { "help",        no_argument,            NULL,        'h' },
    { 0, 0, 0, 0 }
};

static void usage(FILE *fp, int argc, char **argv) {
    fprintf(fp,
            "Usage: %s [options]\n\n"
            "Options:\n"
            " -v | --version       Display version information\n"
            " -h | --help          Show help content\n\n"
            "", basename(argv[0]));
}

static void opt_parsing_err_handle(int argc, char **argv, int flag) {
    /* Exit if no input parameters are entered  */
    int state = 0;
    /* Feedback Error parameter information then exit */
    if (optind < argc || flag) {
        printf("Error:  Parameter parsing failed\n");
        if (flag)
            printf("\tunrecognized option '%s'\n", argv[optind-1]);

        while (optind < argc) {
            printf("\tunrecognized option '%s'\n", argv[optind++]);
        }

        state = -1;
    }

    if (state == -1) {
        printf("Tips: '-h' or '--help' to get help\n\n");
        exit(2);
    }
}

void sig_handle(int arg) {
    g_quit = true;
}

int get_actual_backlight(int fd) {
    unsigned long args[4] = {0};

    args[0] = DISP_LCD_FB0;
    args[1] = 0; /* no use */
    args[2] = 0; /* no use */

    return ioctl(fd, DISP_LCD_GET_BRIGHTNESS, args);
}

int set_brightness(int fd, int value) {
    unsigned long args[4] = {0};

    args[0] = DISP_LCD_FB0;
    args[1] = value;
    args[2] = 0; /* no use */

    return ioctl(fd, DISP_LCD_SET_BRIGHTNESS, args);
}

void run_control_backlight(int fd, int max_value) {
    /* loop to brighten backlight */
    set_brightness(fd, max_value);
    usleep(500000);

    set_brightness(fd, max_value * 0.8);
    usleep(500000);

    set_brightness(fd, max_value * 0.6);
    usleep(500000);

    set_brightness(fd, max_value * 0.4);
    usleep(500000);

    set_brightness(fd, max_value * 0.2);
    usleep(500000);

    set_brightness(fd, 0);
    usleep(500000);
}

/* Draw RGB pixel to framebuffer. */
void drawPixel(char *fbp, unsigned long location, PIXEL *pixel, int bpp) {
    unsigned short rgb = 0;
    unsigned int r = 0, g = 0, b = 0;
    switch (bpp) {
    case 32:
        *(fbp + location) = pixel->blue;
        *(fbp + location + 1) = pixel->green;
        *(fbp + location + 2) = pixel->red;
        *(fbp + location + 3) = 0xff;
        break;

    case 24:
        *(fbp + location) = pixel->blue;
        *(fbp + location + 1) = pixel->green;
        *(fbp + location + 2) = pixel->red;
        break;

    case 16:
        r = (pixel->red * 0x1f) / 0xff;
        g = (pixel->green * 0x3f) / 0xff;
        b = (pixel->blue * 0x1f) / 0xff;
        rgb = (r << 11) | (g << 5) | b;
        *((unsigned short*)(fbp + location)) = rgb;
        break;

    default:
        printf("Error: No fit bpp\n");
        break;
    }
}

void run_draw(char *fbp, struct fb_var_screeninfo *vinfo, struct fb_fix_screeninfo *finfo) {
    /* Display test */
    PIXEL pixel = { 0x00, 0x00, 0x00, 0x00 };

    int section = vinfo->yres / 8;
    int factor = 0;
    static bool reversal = false;
    unsigned long location = 0;
    int x = 0, y = 0;

    reversal = !reversal;
    for ( y = 0; y < vinfo->yres; y++ ) {
        factor = y / section;
        if ( reversal ) {
            /* reverse the color bar. */
            factor = 7 - factor;
        }
        for ( x = 0; x < vinfo->xres; x++ ) {
            location = (x+vinfo->xoffset) * (vinfo->bits_per_pixel / 8) + (y+vinfo->yoffset) * finfo->line_length;
            /* Red border */
            if (x == 0 || x >= vinfo->xres - 1 || y == 0 || y >= vinfo->yres - 1) {
                pixel.red = 0xff;
                pixel.green = 0x00;
                pixel.blue = 0x00;
                pixel.trans = 0x00;
            }
            /* Black border inside red border. */
            else if (x <= 3 || x >= vinfo->xres - 4 || y <= 3 || y >= vinfo->yres - 4) {
                pixel.red = 0x00;
                pixel.green = 0x00;
                pixel.blue = 0x00;
                pixel.trans = 0x00;
            }
            else {
                switch (factor) {
                case 0: // Black
                    pixel.red = 0x00;
                    pixel.green = 0x00;
                    pixel.blue = 0x00;
                    pixel.trans = 0x00;
                    break;
                case 1: // Red
                    pixel.red = 0xff;
                    pixel.green = 0x00;
                    pixel.blue = 0x00;
                    pixel.trans = 0x00;
                    break;
                case 2: // Purple
                    pixel.red = 0xff;
                    pixel.green = 0x00;
                    pixel.blue = 0xff;
                    pixel.trans = 0x00;
                    break;
                case 3: // Yellow
                    pixel.red = 0xff;
                    pixel.green = 0xff;
                    pixel.blue = 0x00;
                    pixel.trans = 0x00;
                    break;
                case 4: // Green
                    pixel.red = 0x00;
                    pixel.green = 0xff;
                    pixel.blue = 0x00;
                    pixel.trans = 0x00;
                    break;
                case 5: // Light blue
                    pixel.red = 0x00;
                    pixel.green = 0xff;
                    pixel.blue = 0xff;
                    pixel.trans = 0x00;
                    break;
                case 6: // Blue
                    pixel.red = 0x00;
                    pixel.green = 0x00;
                    pixel.blue = 0xff;
                    pixel.trans = 0x00;
                    break;
                case 7: // White
                    pixel.red = 0xff;
                    pixel.green = 0xff;
                    pixel.blue = 0xff;
                    pixel.trans = 0x00;
                    break;
                default:
                    break;
                }
            }
            drawPixel(fbp, location, &pixel, vinfo->bits_per_pixel);
        }
    }
}

int main(int argc, char *argv[]) {
    int dispfd = 0;
    int fbfd = 0;
    char *fbp = NULL;

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    int actual_value = -1;
    int max_value = DISP_LCD_BRIGHTNESS_MAX;

    int c = 0;
    int flag = 0;

    /* Parsing input parameters */
    while ((c = getopt_long(argc, argv, g_shortopts, g_longopts, NULL)) != -1) {
        switch (c) {
        case 'v':
            /* Display the version */
            printf("version : 1.0\n");
            exit(0);

        case 'h':
            usage(stdout, argc, argv);
            exit(0);

        default :
            flag = 1;
            break;
        }
    }

    opt_parsing_err_handle(argc, argv, flag);
    
    /* Ctrl+c handler */
    signal(SIGINT, sig_handle);

    /* Open device to control display */
    dispfd = open("/dev/disp", O_RDWR, 0);

    /* Get actual brightness */
    actual_value = get_actual_backlight(dispfd);
    if (actual_value < 0) {
        printf("Error: Get actual brightness value error\n");
        return INADEQUATE_CONDITIONS;
    }
    printf("Actual backlight is: %d\n", actual_value);

    /* Open the file for reading and writing */
    fbfd = open(DEFAULT_FB_DEV, O_RDWR);
    if (!fbfd) {
        printf("Error: cannot open framebuffer device.\n");
        exit(INADEQUATE_CONDITIONS);
    }

    /* Get fixed screen information */
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
        printf("Error reading fixed information.\n");
        close(fbfd);
        exit(INADEQUATE_CONDITIONS);
    }

    /* Get variable screen information */
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        printf("Error reading variable information.\n");
        close(fbfd);
        exit(INADEQUATE_CONDITIONS);
    }
    else
        printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    /* Map the device to memory */
    fbp = (char *)mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (fbp == MAP_FAILED) {
        printf("Error: failed to map framebuffer device to memory.\n");
        close(fbfd);
        exit(INADEQUATE_CONDITIONS);
    }

    /*  
    1. Show color bar of black,red,purple,yellow,green,light blue,blue,white, from top to bottom. 
    2. Control backlight brightness 
    */
    while (!g_quit) {
        run_draw(fbp, &vinfo, &finfo);
        run_control_backlight(dispfd, max_value);
    }

    /* Get back to actual backlight */
    set_brightness(dispfd, actual_value);

    /* Release framebuffer */
    munmap(fbp, finfo.smem_len);
    close(fbfd);

    /* Release display */
    close(dispfd);

    return 0;
}
