#include <omp.h>
#include <stdlib.h>
#include <stdlib.h>

#include "queue.h"
#include "proc_image_utils.h"
#include "proc_image.h"
#include "cli_utils.h"
#include "proc_image.h"
#include "config.h"

typedef struct
{
    unsigned char *img;
    unsigned char *output;
    int width, height, channels;
} TaskData;

static void row_parallel_conv_q(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image)
{

    size_t T = omp_get_num_threads();
#pragma omp taskloop num_tasks(task_granularity_k *T)
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            CONVOLUTION_CORE
        }
    }
}

static void column_parallel_conv_q(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image)
{
    size_t T = omp_get_num_threads();
#pragma omp taskloop num_tasks(task_granularity_k *T)
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            CONVOLUTION_CORE
        }
    }
}

static void pixel_parallel_conv_q(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image)
{
    size_t T = omp_get_num_threads();
#pragma omp taskloop num_tasks(task_granularity_k *T)
    for (int pixel_id = 0; pixel_id < width * height; pixel_id++)
    {
        int x = pixel_id % width;
        int y = pixel_id / width;

        CONVOLUTION_CORE
    }
}

static void block_parallel_conv_q(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image)
{

    int T = omp_get_num_threads();
    int blocks_count = grid_granularity_k * T

                       * T;
    int grid_rows;
    int grid_cols;
    choose_block_grid(blocks_count, width, height, &grid_rows, &grid_cols);
    // printf(" %d x %d = (h x w)\n", grid_rows, grid_cols);
    int block_h = height / grid_rows;
    int block_w = width / grid_cols;
    // printf(" %d x %d = (h x w)\n", block_h, block_w);

#pragma omp taskloop num_tasks(task_granularity_k *T)
    for (int block_id = 0; block_id < blocks_count; block_id++)
    {

        int block_x = block_id % grid_cols;
        int block_y = block_id / grid_cols;

        for (int y = block_y * block_h; (y < (block_y + 1) * block_h) ||
                                        (block_y + 1 == grid_rows && y < height);
             y++)
        {
            for (int x = block_x * block_w; (x < (block_x + 1) * block_w) ||
                                            (block_x + 1 == grid_cols && x < width);
                 x++)

            {
                CONVOLUTION_CORE
            }
        }
    }
}

void conv_queue(ModeType mode, Kernel kernel, char *directory)
{
    char **image_paths = get_image_paths(directory);
    char **output_paths = get_output_paths(image_paths);

#pragma omp parallel
    {
#pragma omp single
        {
            for (int i = 0; image_paths[i] != NULL; i++)
            {
                TaskData *data = malloc(sizeof(TaskData));

// READ
#pragma omp task depend(out : data->img) firstprivate(data, i)
                {
                    data->img = load_image(image_paths[i], &data->width, &data->height, &data->channels);
                    data->img = RGB2grayscale(data->img, data->width, data->height, data->channels);
                }

// CONVOLVE
#pragma omp task depend(in : data->img) depend(out : data -> output) firstprivate(data)
                {
                    data->output = malloc(data->width * data->height);
                    {
                        switch (mode)
                        {
                        case MODE_SEQ:
                            seq_conv(data->img, data->width, data->height, kernel, data->output);
                            break;
                        case MODE_PIXEL:
                            pixel_parallel_conv_q(data->img, data->width, data->height, kernel, data->output);
                            break;
                        case MODE_ROW:
                            row_parallel_conv_q(data->img, data->width, data->height, kernel, data->output);
                            break;
                        case MODE_COLUMN:
                            column_parallel_conv_q(data->img, data->width, data->height, kernel, data->output);
                            break;
                        case MODE_BLOCK:
                            block_parallel_conv_q(data->img, data->width, data->height, kernel, data->output);
                            break;
                        default:
                            fprintf(stderr, "Error: Unknown mode\n");
                            exit(-1);
                        }
                    }
                    free(data->img);
                }

// WRITE
#pragma omp task depend(in : data->output) firstprivate(data)
                {
                    store_image(output_paths[i], data->width, data->height, data->output);
                    free(data->output);
                    free(data);
                }
            }
        }
    }

    free_paths(image_paths);
    free_paths(output_paths);
}
