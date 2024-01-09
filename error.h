#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdio.h>
#include <stdlib.h>

#define ERROR(msg, ...) {fprintf(\
    stderr, "[Error]" __FILE__ "@%d:" msg "\n", __LINE__ __VA_OPT__(,) __VA_ARGS__); exit(1);}

#define TFREE(v) {\
    if (v) {\
        free(v);\
        v = NULL;\
    }\
}

#endif