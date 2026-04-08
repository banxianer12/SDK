/**
* Copyright (C) 2013 Guangzhou Tronlong Electronic Technology Co., Ltd. - www.tronlong.com
* 
* @file parameter_parser.c
* 
* @brief Parameter Parser Module: parse command line arguments
*
* @author Tronlong <support@tronlong.com>
* 
* @version V1.0
* 
* @date 2022-5-26
**/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <libgen.h>

#include "parameter_parser.h"

const char * const VERSION = "1.0";

static const char short_opts [] = "u:w:h:d:v";
static const struct option long_opts [] = {
    { "url",       required_argument,    NULL, 'u' },
    { "width",     required_argument,    NULL, 'w' },
    { "height",    required_argument,    NULL, 'h' },
    { "display",    required_argument,   NULL, 'd' },
    { "version",   no_argument,          NULL, 'v' },
    { "help",      no_argument,          NULL, 0   },
    { 0, 0, 0, 0 }
};

static void usage(char *prog_name) {
    printf ("Usage: %s [options]\n\n"
            "Options:\n"
            " -u | --url        IP Camera url.\n"
            " -w | --width      IP Camera width.\n"
            " -h | --height     IP Camera height.\n"
            " -d | --display    0: Raw image to display.\n"
            "                   1: Sobel image to display.\n"
            " -v | --version    Version Info.\n"
            " --help            Show this message.\n\n"
            "e.g. :\n"
            "   ./%s -u rtsp://ipc_stream_url -w 1920 -h 1080 -d 1\n"
            "\n", prog_name, prog_name);
}


/* Parsing input parameters */
bool parse_parameter(struct _Params *params, int argc, char **argv) {
    params->width = 1920;
    params->height = 1080;
    params->mode = MODE_INVALID;

    int c = 0;
    while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL))!= -1) {
        switch (c) {
        case 'u':
            params->url = optarg;
            break;
        case 'w':
            params->width = strtoul(optarg, 0, 10);
            break;
        case 'h':
            params->height = strtoul(optarg, 0, 10);
            break;
        case 'd':
            params->mode = strtoul(optarg, 0, 10);
            break;
        case 'v':
            printf("version : %s\n", VERSION);
            exit(0);
        case 0: //--help
            usage(basename(argv[0]));
            exit(0);
        default :
            return false;
        }
    }

    if (params->width > 1920 || params->height > 1080 || params->mode == MODE_INVALID)
        return false;

    return true;
}
