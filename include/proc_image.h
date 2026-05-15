#include "core_builder.h"

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

// for conv utility
void proc_image(const char *image_name, ModeType mode, Kernel kernel);
// for bench and test
void choose_block_grid(int N, int width, int height, int *rows, int *cols);
void seq_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image);
void row_parallel_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image);
void column_parallel_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image);
void pixel_parallel_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image);
void block_parallel_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image);