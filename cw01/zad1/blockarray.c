
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "blockarray.h"

const char MIN_CHAR = 'A';
const char MAX_CHAR = 'z';

char static_content[BLK_MAX_QTTY][BLK_MAX_SIZE];

int blk_sum(const char *);

BlockArray *arr_create(size_t blk_qtty, size_t blk_size, bool is_static) {
    BlockArray *arr = malloc(sizeof(BlockArray));
    arr->block_qtty = blk_qtty;
    arr->block_size = blk_size;
    arr->is_static = is_static;
    if (is_static)
        arr->arr = NULL;
    else
        arr->arr = calloc(blk_qtty, sizeof(char *));
    return arr;
}

void arr_delete(BlockArray *barr) {
    for (size_t i = 0; i < barr->block_qtty; ++i) blk_delete(barr, i);
    if (barr->is_static) return;
    free(barr->arr);
    free(barr);
}

char *blk_create(const BlockArray *barr, size_t k) {
    if (barr->is_static) {
        memset(static_content[k], 0, barr->block_size); // fill array with zeros
        return &static_content[k][0];
    } else {
        barr->arr[k] = calloc(barr->block_size, sizeof(char));
        return barr->arr[k];
    }
}

void blk_delete(const BlockArray *barr, size_t k) {
    if (barr->is_static) {
        static_content[k][0] = 0;
    } else {
        free(barr->arr[k]);
        barr->arr[k] = NULL;
    }
}

size_t blk_find(const BlockArray *barr, size_t k) {
    int sum;
    int min = INT_MAX;
    size_t ind = 0;

    if (barr->is_static)
        sum = blk_sum(static_content[k]);
    else
        sum = blk_sum(barr->arr[k]);

    for (size_t i = 0; i < barr->block_qtty; ++i) {
        if (i != k) {
            int dist;
            if (barr->is_static)
                dist = abs(blk_sum(static_content[i]) - sum);
            else
                dist = abs(blk_sum(barr->arr[i]) - sum);

            if (dist < min) {
                min = dist;
                ind = i;
            }
        }
    }
    return ind;
}

int blk_sum(const char *blk) {
    int sum = 0;
    for (int i = 0; blk[i] != 0; ++i) sum += blk[i];
    return sum;
}

char *blk_get(BlockArray *barr, size_t k) {
    if (barr->is_static)
        return static_content[k];
    else
        return barr->arr[k];
}

void blk_put(char *blk, char *content, size_t size) {
    memcpy(blk, content, size);
}

void blk_put_string(char *blk, char *content) {
    size_t size = strlen(content);
    memcpy(blk, content, size);
}

void blk_put_random(char *blk, size_t size) {
    for (size_t i = 0; i < size - 1; ++i)
        blk[i] = (char) (rand() % (MAX_CHAR - MIN_CHAR + 1) + MIN_CHAR);
    blk[size - 1] = 0;
}

