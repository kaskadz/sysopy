//
// Created by wojciech on 3/7/18.
//

#ifndef TASK_STRINGLIB_H
#define TASK_STRINGLIB_H

#include <stdbool.h>

#define BLK_MAX_QTTY 1000000
#define BLK_MAX_SIZE 1000

typedef struct BlockArray {
    char **arr;
    size_t block_qtty;
    size_t block_size;
    bool is_static;
} BlockArray;

BlockArray *arr_create(size_t blk_qtty, size_t blk_size, bool is_static);

void arr_delete(BlockArray *barr);

char *blk_create(const BlockArray *barr, size_t k);

void blk_delete(const BlockArray *barr, size_t k);

char *blk_get(BlockArray *barr, size_t k);

void blk_put(char *blk, char *content, size_t size);

void blk_put_string(char *blk, char *content);

void blk_put_random(char *blk, size_t size);

size_t blk_find(const BlockArray *barr, size_t k);

#endif
