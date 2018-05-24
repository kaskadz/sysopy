#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

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
#define DEBUG(_MSG_, ...)                                                      \
    {                                                                         \
        fprintf(stdout, "[" YEL "INFO" RESET "] " _MSG_ "\n", ##__VA_ARGS__); \
    }

// - |GLOBAL VARIABLES| ------------------ //
double **K;
int **I;
int **J;
int c, w, h;
int thread_count;
pthread_t *threads;
int **arg;

// - |FUNCTION DECLARATIONS| ------------- //
void load_image(const char *filename);
void load_filter(const char *filename);
void init_image_buffers();
void free_image_buffers();
void init_filter_buffer();
void free_filter_buffer();
void init_threads_arrays();
void free_threads_arrays();
void save_image_buffer(const char *filename);
void filter(int arg[2]);
int max(int a, int b);
int min(int a, int b);
int get_index(int x, int bound);

// - |MAIN| ------------------------------ //
int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        ERR_EXIT("Wrong parameters given!");
    }

    thread_count = strtoul(argv[1], NULL, 10);
    const char *input_image_filename = argv[2];
    const char *input_filter_filename = argv[3];
    const char *output_image_filename = argv[4];

    init_threads_arrays();

    load_image(input_image_filename);
    load_filter(input_filter_filename);

    DEBUG("w = %d, h = %d, c = %d", w, h, c)

    for (int i = 0; i < thread_count; i++)
    {
        arg[i][0] = (w / thread_count) * i;
        arg[i][1] = w / thread_count;
        DEBUG("Starting thread (%d, %d)", arg[i][0], arg[i][1])
        pthread_create(&threads[i], NULL, (void *(*)(void *)) & filter, arg[i]);
        // filter(arg);
    }
    arg[thread_count][0] = (w / thread_count) * thread_count;
    arg[thread_count][1] = w % thread_count;
    DEBUG("Starting thread (%d, %d)", arg[thread_count][0], arg[thread_count][1])
    pthread_create(&threads[thread_count], NULL, (void *(*)(void *)) & filter, arg[thread_count]);
    // filter(arg);

    for (int i = 0; i < thread_count + 1; ++i)
    {
        pthread_join(threads[i], NULL);
    }

    save_image_buffer(output_image_filename);

    free_image_buffers();
    free_filter_buffer();
    free_threads_arrays();

    exit(EXIT_SUCCESS);
}

// - |FUNCTION IMPLEMENTATIONS| ------------------------- //
int get_index(int x, int bound)
{
    if (x < 0)
        return 0;
    if (x >= bound)
        return bound - 1;
    return x;
}

double sum(int x, int y)
{
    double result = 0;
    for (int i = 0; i < c; i++)
    {
        for (int j = 0; j < c; j++)
        {
            int a = get_index(x - (int)ceil((double)c / 2.0) + i, w);
            int b = get_index(y - (int)ceil((double)c / 2.0) + j, h);
            result += (double)I[a][b] * K[i][j];
        }
    }
    return result;
}

void filter(int *arg)
{
    int from = arg[0];
    int len = arg[1];
    DEBUG("Hello from thread %lu: (%d,%d)", pthread_self(), from, len)
    INFO("Started filtering")
    for (int i = 0; i < len; i++)
    {
        for (int j = 0; j < h; j++)
        {
            J[i + from][j] = round(sum(i + from, j));
        }
    }
    INFO("Filtering finished")
}

void load_image(const char *filename)
{
    INFO("Loading input image")
    FILE *file = fopen(filename, "r");
    if (file == NULL)
        PERR_EXIT("Error opening a file")

    int maxval;
    fscanf(file, "P2 %d %d %d", &w, &h, &maxval);
    if (maxval != 255)
        ERR_EXIT("Wrong file format: Wrong max value!")

    init_image_buffers();

    // read image contents
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            int a;
            if (fscanf(file, "%d", &a) == 1)
                I[i][j] = a;
            else
                ERR_EXIT("Wrong file format: Not enough contents or wrong dimensions!")
        }
    }

    INFO("Image loading finished");

    fclose(file);
}

void load_filter(const char *filename)
{
    INFO("Loading filter")
    FILE *file = fopen(filename, "r");
    if (file == NULL)
        PERR_EXIT("Error opening a filter file")

    fscanf(file, "%d", &c);

    init_filter_buffer();

    // read image contents
    for (int i = 0; i < c; i++)
    {
        for (int j = 0; j < c; j++)
        {
            double a;
            if (fscanf(file, "%lf", &a) == 1)
                K[i][j] = a;
            else
                ERR_EXIT("Wrong file format: Not enough contents or wrong matrix size!")
        }
    }

    fclose(file);
    INFO("Filter loading finished")
}

void init_threads_arrays()
{
    // initialize thread ids array
    threads = calloc(thread_count + 1, sizeof(int));
    // initialize thread args array
    arg = calloc(thread_count + 1, sizeof(int*));
    for(int i = 0; i < thread_count + 1; i++)
    {
        arg[i] = calloc(2, sizeof(int));
    }
}

void free_threads_arrays()
{
    free(threads);
    for(int i = 0; i < thread_count + 1; i++)
    {
        free(arg[i]);
    }
    free(arg);
}

void init_image_buffers()
{
    I = calloc(w, sizeof(int *));
    J = calloc(w, sizeof(int *));

    for (int i = 0; i < w; i++)
    {
        I[i] = calloc(h, sizeof(int));
        J[i] = calloc(h, sizeof(int));
    }
}

void init_filter_buffer()
{
    K = calloc(c, sizeof(double *));

    for (int i = 0; i < c; i++)
    {
        K[i] = calloc(c, sizeof(double));
    }
}

void free_filter_buffer()
{
    for (int i = 0; i < c; i++)
    {
        free(K[i]);
    }

    free(K);
}

void free_image_buffers()
{
    for (int i = 0; i < w; i++)
    {
        free(I[i]);
        free(J[i]);
    }

    free(I);
    free(J);
}

void save_image_buffer(const char *filename)
{
    INFO("Saving output image to file")
    FILE *file = fopen(filename, "w+");
    if (file == NULL)
        PERR_EXIT("Error opening output file")

    fprintf(file, "P2\n%d %d\n%d\n", w, h, 255);

    // read image contents
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            fprintf(file, "%d ", J[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    INFO("Saving output image finished")
}

int max(int a, int b)
{
    return (a > b) ? a : b;
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}