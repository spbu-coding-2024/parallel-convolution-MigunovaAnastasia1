#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include "cli_utils.h"
#include "proc_image.h"
#include "option_types.h"
#include "core_builder.h"
#include "proc_image_utils.h"
#include "queue.h"

size_t task_granularity_k = 3;
size_t grid_granularity_k = 1;

#define NUM_RUNS 10
#define GRID_GRANULARITY_INPUT "./../images"
#define GRID_GRANULARITY_OUTPUT "./../benchmarks/results/grid_granurity.csv"
#define TASK_GRANULARITY_INPUT "./images"

/*
 * Expected TASK_GRANULARITY_INPUT directory structure:
 * ./images/
 *    SMALL/     - small size images
 *    BIG/       - large size images
 *    MIDDLE/    - medium size images
 *    RANDOM/    - random size images
 */

#define TASK_GRANULARITY_OUTPUT "./../benchmarks/results/task_granurity.csv"

#define QUEUE_SIZE 10
#define FILTER FILTER_MOTION
#define FILTER_SIZE 5

int main()
{
    Kernel *kernel = kernel_builder(FILTER, FILTER_SIZE);

    //============grid_granurity_k benchmark========================

    FILE *out = fopen(GRID_GRANULARITY_OUTPUT, "w");
    if (!out)
    {
        perror("fopen");
        exit(-1);
    }
    fprintf(out, "Image size (h x w), Grid_granularity_k, Time (sec) \n");

    char **image_paths = get_image_paths(GRID_GRANULARITY_INPUT);

    for (int j = 0; image_paths[j] != NULL; j++)
    {

        for (size_t i = 1; i <= 32; i++)
        {
            grid_granularity_k = i;

            int width, height, channels;
            unsigned char *image = load_image(image_paths[j], &width, &height, &channels);
            image = RGB2grayscale(image, width, height, channels);

            unsigned char *result_image = (unsigned char *)malloc(width * height);
            if (result_image == NULL)
            {
                perror("malloc");
                exit(-1);
            }

            double start;
            double end;
            double average = 0;

            for (int k = 0; k < NUM_RUNS; k++)
            {
                start = omp_get_wtime();
                block_parallel_conv(image, width, height, *kernel, result_image);
                end = omp_get_wtime();
                average += end - start;
            }

            average /= NUM_RUNS;
            fprintf(out, "%d x %d, %ld, %.6f\n", height, width, grid_granularity_k, average);
            fflush(out); // чтобы сразу видели результаты

            free(image);
            free(result_image);
        }
    }

    fclose(out);
    free_paths(image_paths);
    grid_granularity_k = 1;

    //==========task_granurity_k benchmark===========

    FILE *out1 = fopen(TASK_GRANULARITY_OUTPUT, "w");
    if (!out1)
    {
        perror("fopen");
        exit(-1);
    }
    fprintf(out1, "Pipeline size (imgs), Image size, Task_granularity_k, Time (sec) \n");

    char **dir_paths = get_paths(TASK_GRANULARITY_INPUT);

    for (int j = 0; dir_paths[j] != NULL; j++) // Image size
    {
        for (size_t m = 1; m <= 12; m++) // Task_granularity_k
        {
            task_granularity_k = m;
            double start;
            double end;
            double average = 0;

            for (int n = 0; n < NUM_RUNS; n++)
            {
                start = omp_get_wtime();
                conv_queue(MODE_ROW, *kernel, dir_paths[j]);
                end = omp_get_wtime();
                average += end - start;
            }
            average /= NUM_RUNS;
            fprintf(out1, "%d, %s, %ld, %.6f\n", QUEUE_SIZE, strrchr(dir_paths[j], '/') + 1, task_granularity_k, average);
            fflush(out1); // чтобы сразу видели результаты
        }
    }
    fclose(out1);
    free_paths(dir_paths);

    return 0;
}
