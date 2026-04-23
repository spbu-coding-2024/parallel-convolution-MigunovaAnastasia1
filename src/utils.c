#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

unsigned char *load_image(const char *filename, int *width, int *height, int *channels)
{
    unsigned char *image = stbi_load(filename, width, height, channels, 0);
    if (image == NULL)
    {
        fprintf(stderr, "Error: Could not load image '%s': %s\n", filename, stbi_failure_reason());
        exit(-1);
    }
    return image;
}

void store_image(const char *filename, int width, int height, unsigned char *data) // только для одноканальных изображений
{
    int result = 0;
    const char *ext = strrchr(filename, '.');
    if (ext == NULL)
    {
        fprintf(stderr, "Error: Filename '%s' has no extension.\n", filename);
        exit(-1);
    }

    if (strcmp(ext, ".png") == 0)
    {
        result = stbi_write_png(filename, width, height, 1, data, width);
    }
    else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0)
    {
        result = stbi_write_jpg(filename, width, height, 1, data, 95); // качество 95%
    }
    else if (strcmp(ext, ".bmp") == 0)
    {
        result = stbi_write_bmp(filename, width, height, 1, data);
    }
    else
    {
        fprintf(stderr, "Error: Unsupported file format '%s'.\n", ext);
        exit(-1);
    }

    if (result == 0)
    {
        fprintf(stderr, "Error: Could not store image '%s'.\n", filename);
        exit(-1);
    }
}

unsigned char *RGB2grayscale(unsigned char *image_data, int width, int height, int channels)
{
    if (channels == 1) // уже в grayscale
    {
        return image_data;
    }

    size_t grayscale_size = width * height;
    unsigned char *grayscale_data = (unsigned char *)malloc(grayscale_size);
    if (grayscale_data == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory for grayscale image.\n");
        exit(2);
    }

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int pixel_index = y * width + x;          // Индекс пикселя в grayscale_data
            int image_index = pixel_index * channels; // Индекс соответствующего пикселя в исходном изображении

            double grayscale_value;
            if (channels == 3 || channels == 4)
            { // RGB or RGBA
                // Стандартная формула преобразования RGB в grayscale (взвешенное усреднение), альфа не учитываем
                grayscale_value = 0.299 * image_data[image_index + 0] +
                                  0.587 * image_data[image_index + 1] +
                                  0.114 * image_data[image_index + 2];
            }
            else
            {
                fprintf(stderr, "Error: Unsupported number of channels: %d\n", channels);
                exit(-1);
            }

            // Ограничиваем значение в диапазоне 0-255 и записываем в grayscale_data
            grayscale_data[pixel_index] = (unsigned char)(grayscale_value > 255 ? 255 : (grayscale_value < 0 ? 0 : grayscale_value));
        }
    }
    return grayscale_data;
}