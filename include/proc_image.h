#include "CoreBuilder.h"

void proc_image(const char *image_name, ModeType mode, Kernel kernel);
void seq_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image);
void row_parallel_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image);
void column_parallel_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image);
void pixel_parallel_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image);
void block_parallel_conv(unsigned char *image_data, int width, int height, Kernel kernel, unsigned char *result_image);