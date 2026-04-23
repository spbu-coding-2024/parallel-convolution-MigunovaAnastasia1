#include <stdlib.h>
#include <sys/stat.h>
#include "proc_image.h"
#include "utils.h"

unsigned char *seq_conv(unsigned char *image_data, int width, int height, Kernel kernel)
{
    size_t image_size = width * height;
    size_t kernel_size = kernel.size;
    unsigned char *convolution_data = (unsigned char *)malloc(image_size);
    if (convolution_data == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory to store convolution result.\n");
        exit(-1);
    }

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int pixel_value = 0;
            int start_x = x - kernel_size / 2;
            int start_y = y - kernel_size / 2;

            // Вычисляем сумму произведений значений пикселей и фильтра
            for (int filterY = 0; filterY < kernel_size; filterY++)
            {
                for (int filterX = 0; filterX < kernel_size; filterX++)
                {
                    // достраиваем изображение по краям с помощью wrapping around
                    int imageX = (start_x + filterX + width) % width;
                    int imageY = (start_y + filterY + height) % height;

                    // Индекс пикселя в исходном изображении
                    int image_index = (imageY * width + imageX);
                    int kernel_index = (filterY * kernel_size + filterX);
                    pixel_value += image_data[image_index] * kernel.matrix[kernel_index];
                }
            }
            double final_pixel_value = pixel_value * kernel.factor + kernel.bias;
            // Ограничиваем значения в диапазоне 0-255 и преобразуем тип
            convolution_data[y * width + x] =
                (unsigned char)(final_pixel_value > 255.0 ? 255.0 : (final_pixel_value < 0.0 ? 0.0 : final_pixel_value));
        }
    }
    return convolution_data;
}

void proc_image(const char *image_name, ModeType mode, Kernel kernel)
{

    char input_path[512];
    snprintf(input_path, 512, "./images/%s", image_name);

    int width, height, channels;
    unsigned char *image = load_image(input_path, &width, &height, &channels);
    image = RGB2grayscale(image, width, height, channels);

    unsigned char *result_image;

    {
        switch (mode)
        {
        case MODE_SEQ:
            result_image = seq_conv(image, width, height, kernel);
            break;
        case MODE_PIXEL:

            break;
        case MODE_ROW:

            break;
        case MODE_COLUMN:

            break;
        case MODE_BLOCK:

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
