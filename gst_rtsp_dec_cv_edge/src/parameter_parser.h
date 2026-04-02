/* Copyright 2022 Tronlong Elec. Tech. Co. Ltd. All Rights Reserved. */

#ifndef PARAMETER_PARSER_H
#define PARAMETER_PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#if defined (__cplusplus)
extern "C" {
#endif

enum MODE {
    MODE_CAP_DISPLAY,
    MODE_CAP_SOBEL,
    MODE_INVALID = -1
};

typedef struct _Params {
    enum MODE   mode;
    char       *url;
    uint32_t    width;
    uint32_t    height;
}CmdLineParams;

bool parse_parameter(struct _Params *params, int argc, char **argv);

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* PARAMETER_PARSER_H */
