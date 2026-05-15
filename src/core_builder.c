#include <stdlib.h>
#include "option_types.h"
#include "core_builder.h"

static Kernel *kernel_init(int size)
{
    Kernel *k = (Kernel *)malloc(sizeof(Kernel));
    if (!k)
        return NULL;

    k->matrix = (int *)malloc(size * size * sizeof(int));
    if (!k->matrix)
    {
        free(k);
        return NULL;
    }
    k->factor = 1.0;
    k->bias = 0.0;
    k->size = size;

    return k;
}

static void create_blur_kernel(Kernel *kernel, size_t size)
{
    kernel->filter_name = "bl";
    for (size_t i = 0; i < size * size; i++)
    {
        kernel->matrix[i] = 1;
    }
    kernel->factor = 1.0 / (size * size);
}

static void create_sharpen_kernel(Kernel *kernel, size_t size)
{

    kernel->filter_name = "sh";
    size_t total_elements = size * size;

    for (size_t i = 0; i < total_elements; i++)
    {
        kernel->matrix[i] = -1;
    }

    kernel->matrix[(size / 2) * (size + 1)] = total_elements;
}

static void create_edge_kernel(Kernel *kernel, size_t size)
{

    kernel->filter_name = "edg";
    size_t total_elements = size * size;

    for (size_t i = 0; i < total_elements; i++)
    {
        kernel->matrix[i] = -1;
    }

    kernel->matrix[(size / 2) * (size + 1)] = total_elements - 1;
}

static void create_emboss_kernel(Kernel *kernel, size_t size)
{

    kernel->filter_name = "emb";
    for (size_t y = 0; y < size; y++)
    {
        for (size_t x = 0; x < size; x++)
        {
            if (x < y)
            {
                kernel->matrix[y * size + x] = -1; // выше диагонали
            }
            else if (x > y)
            {
                kernel->matrix[y * size + x] = 1; // ниже диагонали
            }
            else
            {
                kernel->matrix[y * size + x] = 0; // на диагонали
            }
        }
    }

    kernel->bias = 128.0;
}

static void create_motion_kernel(Kernel *kernel, size_t size)
{

    kernel->filter_name = "mot";
    for (size_t i = 0; i < size * size; i++)
    {
        kernel->matrix[i] = 0;
    }

    for (size_t i = 0; i < size; i++)
    {
        kernel->matrix[i * size + i] = 1;
    }

    kernel->factor = 1.0 / size;
}

Kernel *kernel_builder(FilterType filter, size_t size)
{
    Kernel *kernel = kernel_init(size);
    if (kernel == NULL)
    {
        fprintf(stderr, "Error: failed to allocate memory for filter kernel\n");
        exit(-1);
    }

    switch (filter)
    {
    case FILTER_BLUR:
        create_blur_kernel(kernel, size);
        return kernel;
        break;

    case FILTER_SHARPEN:
        create_sharpen_kernel(kernel, size);
        return kernel;
        break;

    case FILTER_EDGE:
        create_edge_kernel(kernel, size);
        return kernel;
        break;

    case FILTER_EMBOSS:
        create_emboss_kernel(kernel, size);
        return kernel;
        break;

    case FILTER_MOTION:
        create_motion_kernel(kernel, size);
        return kernel;
        break;

    default:
        fprintf(stderr, "Error: unknown filter type\n");
        kernel_free(kernel);
        exit(-1);
    }
}

void kernel_free(Kernel *k)
{
    if (k)
    {
        free(k->matrix);
        free(k);
    }
}
