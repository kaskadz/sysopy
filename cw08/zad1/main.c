#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

#include "rainbow.h"

// - |MACROS| ---------------------------- //
#define PERR_EXIT(_MSG_)                          \
    {                                             \
        perror("[" RED "PERROR" RESET "]" _MSG_); \
        exit(EXIT_FAILURE);                       \
    }

#define ERR_EXIT(_MSG_, ...)                                                   \
    {                                                                          \
        fprintf(stderr, "[" RED "ERROR" RESET "] " _MSG_ "\n", ##__VA_ARGS__); \
        exit(EXIT_FAILURE);                                                    \
    }

#define WARNING(_MSG_, ...)                                                      \
    {                                                                            \
        fprintf(stdout, "[" YEL "WARNING" RESET "] " _MSG_ "\n", ##__VA_ARGS__); \
    }
#define INFO(_MSG_, ...)                                                      \
    {                                                                         \
        fprintf(stdout, "[" CYN "INFO" RESET "] " _MSG_ "\n", ##__VA_ARGS__); \
    }

// - |GLOBAL VARIABLES| ------------------ //
double **K;
unsigned char **I;
unsigned char **J;
unsigned int c, w, h;

// - |FUNCTION DECLARATIONS| ------------- //
void load_image(const char *filename);
void init_image_buffers();

// - |MAIN| ------------------------------ //
int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        ERR_EXIT("Wrong parameters given!");
    }

    unsigned int thread_count = strtoul(argv[1], NULL, 10);
    const char *input_image_filename = argv[2];
    const char *input_filter_filename = argv[3];
    const char *output_image_filename = argv[4];

    load_image(input_image_filename);

    free_image_buffers();

    exit(EXIT_SUCCESS);
}

// - |FUNCTION IMPLEMENTATIONS| ------------------------- //

void load_image(const char *filename)
{
    INFO("Loading input image")
    FILE *file = fopen(filename, "r");
    if (file == NULL)
        PERR_EXIT("Error opening a file")

    unsigned char maxval;
    fscanf(file, "P2 %d %d %d", &w, &h, &maxval);
    if (maxval != 255)
        ERR_EXIT("Wrong file format: Wrong max value!")

    init_image_buffers();

    // read image contents
    for (size_t i = 0; i < w; i++)
    {
        for (size_t j = 0; j < h; j++)
        {
            unsigned char a;
            if (fscanf(file, "%hhd", &a) == 1)
                I[i][j] = a;
            else
                ERR_EXIT("Wrong file format: Not enough contents or wrong dimensions!")
        }
    }

    INFO("Read the file")

    fclose(file);
}

void init_image_buffers()
{
    I = calloc(w, sizeof(unsigned char *));
    J = calloc(w, sizeof(unsigned char *));

    for (size_t i = 0; i < w; i++)
    {
        I[i] = calloc(h, sizeof(unsigned char));
        J[i] = calloc(h, sizeof(unsigned char));
    }
}

void free_image_buffers()
{
    for (size_t i = 0; i < w; i++)
    {
        free(I[i]);
        free(J[i]);
    }

    free(I);
    free(J);
}