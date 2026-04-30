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


int main(int argc, char *argv[])
{
    // default values for args
    Options options = {
        .input = OPTION("--input", VAL_STRING(NULL),
                        "Input file. File must have .png extention and be located in 'images' directory relative\n                             "
                        "to the current working directory. Example: ./images/photo.png\n                             "
                        "(default: first file in the ./images directory)"),

        .filter = OPTION("--filter", VAL_FILTER(FILTER_BLUR),
                         "Filter type. Possible values: blur, sharpen,"
                         " edge, emboss, motion (default: blur)"),

        .size = OPTION("--size", VAL_INT(3),
                       "Kernel size. Possible values: any odd number in the range from 3 to 13 (default: 3)"),

        .mode = OPTION("--mode", VAL_MODE(MODE_SEQ),
                       "Processing mode. Possible values: seq, pixel, row, column, block (default: seq)"),

        .clean = OPTION("--clean", VAL_BOOL(false),
                        "Remove all files from ./output directory before writing new results"),
        .help = OPTION("--help", VAL_BOOL(false),
                       "Print help information"),
    };

    parse_arguments(argc, argv, &options);

    if (options.input.value.as_string == NULL)
    {
        options.input.value.as_string = get_default_input(); // FREE
    }

    Kernel *kernel = kernel_builder(options.filter.value.as_filter, options.size.value.as_int);
    proc_image(options.input.value.as_string, options.mode.value.as_mode, *kernel);

    free(options.input.value.as_string);
    kernel_free(kernel);

    return 0;
}
