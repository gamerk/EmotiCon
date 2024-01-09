#include "list.h"
#include "error.h"

#include <stdio.h>

#define CONV_IND(l, i) ((l->start_ind + (i)) % l->size)

/**
 * @brief Creates a new List given an optional underlying list and a size
 * 
 * @param arr The underlying array to use, or NULL to automatically create a new array.
 * @param size The size of the new List. If arr is NULL, this is the max size, and the actual size is set to 0.
 * @return The new List.
 * 
 * @throws Error if the function failed to allocate memory
 */
List create_list(Token *arr, size_t size){
    List l = (List){
        .size = arr ? size : 0,
        .max_size = size,
        .start_ind = 0
    };

    if (arr) {
        l.arr = arr;
    } else {
        l.arr = (Token *)calloc(l.max_size, sizeof(Token));
        if (!l.arr){
            ERROR("Failed to allocate memory for a list of max_size %zu", l.max_size);
        }
    }

    return l;
}


/**
 * Inserts an element at an index into a list
 * 
 * ### Parameters ###
 * `l` - List to insert into
 * `ind` - That the element will be at after insertion
 * `el` - Element to insert
 * 
 * ### Returns ###
 * Exit code of function
 * `0` - Success 
 * `1` - Out of bounds of list
 * `2` - Could not allocate enough memory for the new list. (The original will not be changed)
 */
int linsert(List *l, size_t ind, Token el){

    // TODO: Insert end
    if (ind > l->size){
        return 1;
    }

    if (ind == 0 && l->size == 0){
        if (l->max_size == 0) {
            Token *new_arr = (Token *)calloc(1, sizeof(Token));
            if (!new_arr){
                return 2;
            }
            l->arr = new_arr;
        }

        l->arr[0] = el;
        l->size++;

        return 0;
    } else if (l->size >= l->max_size) {

        // Try to allocate more memory (2x original best, original + 1 otherwise)
        Token *new_arr = (Token *)calloc(l->max_size * 2, sizeof(Token));
        if (!new_arr){
            new_arr = (Token *)calloc(l->max_size + 1, sizeof(Token));
            if (!new_arr) {
                // fprintf(stderr, "Could not allocate more space for list array of size %zu", l->max_size + 1);
                return 2;
            }
            l->max_size += 1;
        } else {
            l->max_size *= 2;
        }

        // Transfer old array to new array, such that start_ind == 0
        // Also include newly inserted element
        for (size_t i = 0; i <= l->size; i++){
            if (i < ind){
                new_arr[i] = l->arr[CONV_IND(l, i)];
            } else if (i == ind) {
                new_arr[i] = el;
            } else {
                new_arr[i] = l->arr[CONV_IND(l, i - 1)];
            }
        }
        l->size += 1;
        TFREE(l->arr);

        l->arr = new_arr;
        l->start_ind = 0;
        return 0;
    } else {
        l->size += 1;

        const size_t ci = CONV_IND(l, ind);
        for (size_t i = l->size - 1; i >= ci; i--){
            l->arr[i + 1] = l->arr[i];
        }
        l->arr[ci] = el;
        
        if (ci < l->start_ind){
            l->start_ind += 1;
        }

        return 0;
    }
}

int lremove(List *l, size_t ind){
    
    if (ind >= l->size){
        return 1;
    }

    const size_t ci = CONV_IND(l, ind);
    free_tkn(l->arr[ci]);
    for (size_t i = ci; i < l->size - 1; i++){
        l->arr[i] = l->arr[i + 1];
    }

    if (ci < l->start_ind){
        l->start_ind -= 1;
    }

    l->size -= 1;

    return 0;
}

/**
 * @brief Gets a token from a list and stores it in a variable
 * 
 * @param l The list to get from
 * @param ind The index of the element to get
 * @param dest A pointer to the variable to put the result in
 * @return `1` if out of bounds, `0` otherwise 
 */
int lget(List l, size_t ind, Token *dest){
    if (ind >= l.size){
        return 1;
    }

    *dest = l.arr[(l.start_ind + ind) % l.size];
    return 0;
}

void lrotate(List *l, long long amount){
    if (l->size == 0){
        return;
    }

    l->start_ind = (l->start_ind + amount) % l->size;
}

void lreverse(List *l){
    for (size_t i = 0; i < l->size / 2; i++){
        size_t ci = CONV_IND(l, i);
        size_t copp = CONV_IND(l, l->size - i - 1);
        
        Token temp = l->arr[ci];
        l->arr[ci] = l->arr[copp];
        l->arr[copp] = temp;
    }
}

List lcopy(const List l){
    List newl = (List){
        .max_size = l.max_size,
        .size = l.size,
        .start_ind = l.start_ind
    };

    newl.arr = (Token *) calloc(l.max_size, sizeof(Token));
    if (!newl.arr){
        newl.arr = (Token *) calloc(l.size, sizeof(Token));
        if (!newl.arr){
            ERROR("Failed to allocate memory for a copy of list of size %zu (originally tried %zu)", l.size, l.max_size);
        }
    }

    for (size_t i = 0; i < l.size; i++){
        newl.arr[i] = copy_tkn(l.arr[i]);
    }

    return newl;
}

void lfree(List l){
    if (l.arr == NULL){
        return;
    }

    for (size_t i = 0; i < l.size; i++){
        free_tkn(l.arr[i]);
        l.arr[i] = (Token){ 0 };
    }

    free(l.arr);
}