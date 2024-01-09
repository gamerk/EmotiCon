#ifndef LIST_H
#define LIST_H

#include "lex.h"
#include <stdlib.h>

typedef struct {
    Token *arr;
    size_t size;
    size_t max_size;
    size_t start_ind;
} List;

List create_list(Token *arr, size_t size);
int linsert(List *l, size_t ind, Token el);
int lremove(List *l, size_t start);
int lget(List l, size_t ind, Token *dest);

void lrotate(List *l, long long amount);
void lreverse(List *l);
List lcopy(const List l);

void lfree(List l);


#endif