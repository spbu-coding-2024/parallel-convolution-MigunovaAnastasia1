#include <stdlib.h>
#include <sys/stat.h>
#include <omp.h>
#include <float.h>
#include <math.h>

#include "proc_image.h"
#include "proc_image_utils.h"

#define CONVOLUTION_CORE                                                          \
                                                                                  \
    int pixel_value = 0;                                                          \
    int start_x = x - kernel.size / 2;                                            \
    int start_y = y - kernel.size / 2;                                            \
    for (size_t filter_y = 0; filter_y < kernel.size; filter_y++)                 \
    {                                                                             \
        for (size_t filter_x = 0; filter_x < kernel.size; filter_x++)             \
        {                                                                         \
            /* wrapping around */                                                 \
            int image_x = (start_x + filter_x + width) % width;                   \
            int image_y = (start_y + filter_y + height) % height;                 \
            int image_index = (image_y * width + image_x);                        \
            int kernel_index = (filter_y * kernel.size + filter_x);               \
            pixel_value += image_data[image_index] * kernel.matrix[kernel_index]; \
        }                                                                         \
    }                                                                             \
    pixel_value = pixel_value * kernel.factor + kernel.bias;                      \
    result_image[y * width + x] =                                                 \
        (unsigned char)(pixel_value > 255.0 ? 255.0 : (pixel_value < 0.0 ? 0.0 : pixel_value));

static void choose_block_grid(int N, int width, int height, int *rows, int *cols)
{
    int best_rows = 1, best_cols = N;
    double best_aspect = DBL_MAX;
    for (int r = 1; r <= N; r++)
    {
        if (N % r == 0)
        {
            int c = N / r;
            double block_w = (double)width / c;
            double block_h = (double)height / r;
            double aspect_diff = fmax(block_w / block_h, block_h / block_w);
            if (aspect_diff < best_aspect)
            {
                best_aspect = aspect_diff;
                best_rows = r;
                best_cols = c;
            }
        }
    }
    *rows = best_rows;
    *cols = best_cols;
}

void seq_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image)
{

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            CONVOLUTION_CORE
        }
    }
}

void row_parallel_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image)
{

#pragma omp parallel for
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            CONVOLUTION_CORE
        }
    }
}

void column_parallel_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image)
{
#pragma omp parallel for
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            CONVOLUTION_CORE
        }
    }
}

void pixel_parallel_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image)
{

#pragma omp parallel for
    for (int pixel_id = 0; pixel_id < width * height; pixel_id++)
    {
        int x = pixel_id % width;
        int y = pixel_id / width;

        CONVOLUTION_CORE
    }
}

void block_parallel_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image)
{

#pragma omp parallel
    {

        int blocks_count = omp_get_num_threads();
        int grid_rows;
        int grid_cols;
        choose_block_grid(blocks_count, width, height, &grid_rows, &grid_cols);
        int block_h = height / grid_rows;
        int block_w = width / grid_cols;

#pragma omp for
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
}

void proc_image(const char *image_name, ModeType mode, Kernel kernel)
{

    char input_path[512];
    snprintf(input_path, 512, "./images/%s", image_name);

    int width, height, channels;
    unsigned char *image = load_image(input_path, &width, &height, &channels);
    image = RGB2grayscale(image, width, height, channels);

    unsigned char *result_image = (unsigned char *)malloc(width * height);
    if (result_image == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory to store convolution result.\n");
        exit(-1);
    }

    {
        switch (mode)
        {
        case MODE_SEQ:
            seq_conv(image, width, height, kernel, result_image);
            break;
        case MODE_PIXEL:
            pixel_parallel_conv(image, width, height, kernel, result_image);
            break;
        case MODE_ROW:
            row_parallel_conv(image, width, height, kernel, result_image);
            break;
        case MODE_COLUMN:
            column_parallel_conv(image, width, height, kernel, result_image);
            break;
        case MODE_BLOCK:
            block_parallel_conv(image, width, height, kernel, result_image);
            break;
        default:
            fprintf(stderr, "Error: Unknown mode\n");
            exit(-1);
        }
    }

    mkdir("./outputs", 0755);
    char output_path[512];
    snprintf(output_path, 512, "./outputs/%s_%ld_%s", kernel.filter_name, kernel.size, image_name);

    store_image(output_path, width, height, result_image);

    free(image);
    free(result_image);
}
