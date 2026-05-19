// manual run: gcc -fopenmp ./src/*.c -I ./deps -I ./include -o conv -lm
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#include "option_types.h"
#include "core_builder.h"
#include "proc_image.h"
#include "cli_utils.h"
#include "queue.h"

#define IMAGE_DIR "./images"

size_t task_granularity_k = 3;
size_t grid_granularity_k = 1;

int main(int argc, char *argv[])
{
    // default values for args
    Options options = {
        .input = OPTION("--input", VAL_STRING(NULL),
                        "Input file. File must have png, jpg, jpeg or bmp extention and be located in ./images directory.\n                              "
                        "Example: --input=photo.png (file: ./images/photo.png)\n                              "
                        "(default: first file in the ./images directory)"),

        .filter = OPTION("--filter", VAL_FILTER(FILTER_BLUR),
                         "Filter type. Possible values: blur, sharpen,"
                         " edge, emboss, motion (default: blur)"),

        .size = OPTION("--size", VAL_INT(3),
                       "Kernel size. Possible values: any odd number in the range from 3 to 13 (default: 3)"),

        .mode = OPTION("--mode", VAL_MODE(MODE_SEQ),
                       "Processing mode. Possible values: seq, pixel, row, column, block (default: seq)"),

        .queue = OPTION("--queue", VAL_BOOL(false),
                        "Enable queue-based pipeline processing (reader → convolution → writer)"),

        .clean = OPTION("--clean", VAL_BOOL(false),
                        "Remove all files from ./outputs directory before writing new results"),
        .help = OPTION("--help", VAL_BOOL(false),
                       "Print help information"),
    };

    parse_arguments(argc, argv, &options);

    if (options.queue.value.as_bool)
    {
        Kernel *kernel = kernel_builder(options.filter.value.as_filter, options.size.value.as_int);
        conv_queue(options.mode.value.as_mode, *kernel, IMAGE_DIR);
        if (options.input.value.as_string != NULL)
        {
            free(options.input.value.as_string);
        }
        kernel_free(kernel);
        return 0;
    }

    if (options.input.value.as_string == NULL)
    {
        options.input.value.as_string = get_default_input(IMAGE_DIR);
    }
    printf("Input file: %s\n", options.input.value.as_string);

    Kernel *kernel = kernel_builder(options.filter.value.as_filter, options.size.value.as_int);
    char image_path[512];
    snprintf(image_path, 512, "%s/%s", IMAGE_DIR, options.input.value.as_string);
    proc_image(image_path, options.mode.value.as_mode, *kernel);

    free(options.input.value.as_string);
    kernel_free(kernel);

    return 0;
}
