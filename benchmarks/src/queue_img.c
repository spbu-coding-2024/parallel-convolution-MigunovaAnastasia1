#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli_utils.h"
#include "proc_image.h"
#include "option_types.h"
#include "core_builder.h"
#include "proc_image_utils.h"
#include "queue.h"

#define FILTER FILTER_MOTION
#define FILTER_SIZE 5

size_t task_granularity_k = 3;
size_t grid_granularity_k = 1;

#define NUM_RUNS 20
#define IMAGE_DIR "./../build/images/RANDOM"
#define OUTPUT "./../benchmarks/results/pipeline_results.csv"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

static double bench_seq_queue(Kernel kernel, FILE *out)
{
    char **image_paths = get_image_paths(IMAGE_DIR);
    double start;
    double end;
    double average = 0;

    for (int k = 0; k < NUM_RUNS; k++)
    {
        start = omp_get_wtime();
        for (int j = 0; image_paths[j] != NULL; j++)
        {
            proc_image(image_paths[j], MODE_SEQ, kernel);
        }
        end = omp_get_wtime();
        average += end - start;

        if (k == 0)
        {
            fprintf(out, "\nSeq: %f", end - start);
            fflush(out);
        }
        else
        {
            fprintf(out, ", %f", end - start);
            fflush(out);
        }
    }
    average /= NUM_RUNS;
    return average;
}

static double bench_pipeline_queue(Kernel kernel, FILE *out)
{
    double start;
    double end;
    double average = 0;

    for (int k = 0; k < NUM_RUNS; k++)
    {
        start = omp_get_wtime();
        conv_queue(MODE_SEQ, kernel, IMAGE_DIR);
        end = omp_get_wtime();
        average += end - start;

        if (k == 0)
        {
            fprintf(out, "\nPipeline: %f", end - start);
            fflush(out);
        }
        else
        {
            fprintf(out, ", %f", end - start);
            fflush(out);
        }
    }
    average /= NUM_RUNS;
    return average;
}

int main()
{
    Kernel *kernel = kernel_builder(FILTER, FILTER_SIZE);
    FILE *out = fopen(OUTPUT, "w");
    if (!out)
    {
        perror("fopen");
        exit(-1);
    }

    // замеры для сравнения производительности потоковой обработки изображений
    // последовательной и параллельной реализаций.
    bench_seq_queue(*kernel, out);
    bench_pipeline_queue(*kernel, out);

    // замеры pipeline_speedup
    char **paths = get_paths(IMAGE_DIR); // изначально в папке 24 изображения
    for (int j = 0; paths[j + 1] != NULL; j++)
    {
        if (remove(paths[j]) == 0 && remove(paths[j + 1]) == 0)
        {
            double seq_res = bench_seq_queue(*kernel, out);
            double pipe_res = bench_pipeline_queue(*kernel, out);
            fprintf(out, "\n%d изображений: pipeline быстрее seq в %f раз\n\n", 24 - (j + 2), seq_res / pipe_res);
        }
        else
        {
            perror("remove");
            exit(-1);
        }
        j++;
    }

    fclose(out);
    return 0;
}
