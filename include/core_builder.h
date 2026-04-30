#ifndef COREBUILDER_H
#define COREBUILDER_H
#include "option_types.h"

typedef struct Kernel
{
    char *filter_name; // for output file's naming
    int *matrix;
    size_t size;
    double factor;
    double bias;
} Kernel;

Kernel *kernel_builder(FilterType filter, size_t size);

void kernel_free(Kernel *k);

#endif
