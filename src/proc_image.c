#include <stdlib.h>
#include <sys/stat.h>
#include <omp.h>

#include "proc_image.h"
#include "proc_image_utils.h"

#define CONVOLUTION_CORE                                                          \
                                                                                  \
    int pixel_value = 0;                                                          \
    int start_x = x - kernel.size / 2;                                            \
    int start_y = y - kernel.size / 2;                                            \
    for (size_t filter_y = 0; filter_y < kernel.size; filter_y++)                    \
    {                                                                             \
        for (size_t filter_x = 0; filter_x < kernel.size; filter_x++)                \
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

    for (int pixel_id = 0; pixel_id < width * height; pixel_id++)
    {
        int x = pixel_id % width;
        int y = pixel_id / width;

        CONVOLUTION_CORE
    }
}

void block_parallel_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image)
{

// создаем потоки заранее для того, чтобы узнать их количество, так как хотим динамически задать
// количество блоков на основе количества потоков, чтобы обеспечить работой каждый поток (load-balance).
#pragma omp parallel
    {

        int num_threads = omp_get_num_threads();
        int blocks_amount;
        int blocks_row_amount;
        int blocks_column_amount;
        int block_width;
        int block_height;

        if (num_threads % 2 == 0)
        {
            blocks_amount = num_threads;
        }
        else
        {
            blocks_amount = num_threads * 2;
        }

        if (width <= height)
        {
            blocks_column_amount = 2;
            blocks_row_amount = blocks_amount / 2;
            block_width = width / blocks_column_amount;
            block_height = height / blocks_row_amount;
        }
        else
        {
            blocks_row_amount = 2;
            blocks_column_amount = blocks_amount / 2;
            block_width = width / blocks_column_amount;
            block_height = height / blocks_row_amount;
        }

#pragma omp for
        for (int block_id = 0; block_id < blocks_amount; block_id++)
        {
            int block_x;
            int block_y;
            if (blocks_column_amount == 2)
            {
                block_x = block_id / blocks_row_amount;
                block_y = block_id % blocks_row_amount;
            }
            else
            {
                block_x = block_id % blocks_column_amount;
                block_y = block_id / blocks_column_amount;
            }

            for (int y = block_y * block_height; y < (block_y + 1) * block_height ||
                                                 ((block_y + 1) == blocks_row_amount && y < height);
                 y++)
            {
                for (int x = block_x * block_width; x < (block_x + 1) * block_width ||
                                                    ((block_x + 1) == blocks_column_amount && x < width);
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
