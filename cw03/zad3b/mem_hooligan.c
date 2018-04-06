#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    size_t blocks;
    size_t size;
    if (argc > 2) {
        blocks = strtoul(argv[1], 0, 10);
        size = strtoul(argv[2], 0, 10);
    } else {
        blocks = 10;
        size = 10;
    }
    printf("blocks: %zu\nsize: %zu\n", blocks, size);
    size_t total = 0;
    char *block;
    size_t i;
    for (i = 0; i < blocks; ++i) {
        printf("Allocating %zu MiB for a total of %zu MiB.\n", size, total + size);
        block = malloc(size * 1024 * 1024);
        memset(block, 1, size * 1024 * 1024);
        total += size;
    }
    return 0;
}
