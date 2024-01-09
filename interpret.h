#ifndef __INTERPRET_H__
#define __INTERPRET_H__

#include "list.h"
#include "stdio.h"

typedef struct {
    FILE *input;
    FILE *output;
    FILE *code;
} InterpeterOptions;

typedef struct {
    char *name;
    List list;
} EmoList;

EmoList *lists;

int interpret(InterpeterOptions o);

#endif