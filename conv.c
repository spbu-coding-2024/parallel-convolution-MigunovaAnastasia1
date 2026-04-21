#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Options.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

bool input_is_valid(const char *input)
{

    if (!strcmp(input, ""))
    {
        return false;
    }
    // file exists?
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "./images/%s", input);
    FILE *file = fopen(full_path, "rb");

    if (file == NULL)
    {
        fprintf(stderr, "file '%s' not found in ./images directory\n", input);
        return false;
    }
    fclose(file);

    // valid extension?
    char *dot_sign = strchr(input, '.');
    if (dot_sign == NULL)
    {
        fprintf(stderr, "file '%s' must have (.png) extension\n", input);
        return false;
    }
    else
    {
        char *extention = dot_sign + 1;
        if (strcmp(extention, "png"))
        {
            fprintf(stderr, "file '%s' must have (.png) extension\n", input);
            return false;
        }
    }
    return true;
}

void print_option_usage(Option option, char *invalid_value)
{

    fprintf(stderr, "error: invalid value '%s' for '%s=<%s>'\n", invalid_value, option.cmd_name, option.cmd_name + 2);
    printf("Option usage:\n");
    printf("    %s=<%s>          %s\n", option.cmd_name, option.cmd_name + 2, option.description);
    printf("For more information, try '--help'.\n");
    exit(-1);
}

void print_options(Options options)
{
    printf("Options:\n");
    printf("    --input=<input_file>      %s\n", options.input.description);
    printf("    --filter=<filter>         %s\n", options.filter.description);
    printf("    --size=<size>             %s\n", options.size.description);
    printf("    --mode=<mode>             %s\n\n", options.mode.description);
    printf("Flags:\n");
    printf("    --clean, -c               %s\n", options.clean.description);
    printf("    --help,  -h               %s\n", options.help.description);
}

void print_help(Options options)
{
    printf("Apply convolution filters (blur, sharpen, edge detection, etc.) to images\n\n");
    printf("Usage: conv [OPTION]...\n\n");
    print_options(options);
    exit(0);
}

void invalid_arg(char *invalid_arg, Options options)
{
    if (!(strncmp("-", invalid_arg, 1)))
    {
        fprintf(stderr, "error: unexpected option '%s' found\n", invalid_arg);
        printf("\nConv capabilities...\n");
        print_options(options);
    }
    else
    {
        fprintf(stderr, "error: unexpected argument '%s' found\n", invalid_arg);
    }
    printf("\nFor more information, try '--help'.\n");
    exit(-1);
}

void parse_arguments(int argc, char *argv[], Options *options)
{
    for (int i = 1; i < argc; i++)
    {
        char *equal_sign = strchr(argv[i], '=');

        if (equal_sign != NULL)
        {
            int n = equal_sign - argv[i];

            if (n == 0)
            {
                invalid_arg(argv[i], *options);
            }

            char *value = equal_sign + 1;
            *equal_sign = '\0';

            if (!strcmp(options->input.cmd_name, argv[i]))
            {
                if (input_is_valid(value))
                {
                    options->input.value.as_string = value;
                }
                else
                {
                    print_option_usage(options->input, value);
                }
            }
            else if (!strcmp(options->filter.cmd_name, argv[i]))
            {
                if (!strcmp(value, "blur"))
                {
                    options->filter.value.as_filter = FILTER_BLUR;
                }
                else if (!strcmp(value, "edge"))
                {
                    options->filter.value.as_filter = FILTER_EDGE;
                }
                else if (!strcmp(value, "emboss"))
                {
                    options->filter.value.as_filter = FILTER_EMBOSS;
                }
                else if (!strcmp(value, "gaussian"))
                {
                    options->filter.value.as_filter = FILTER_GAUSSIAN;
                }
                else if (!strcmp(value, "motion"))
                {
                    options->filter.value.as_filter = FILTER_MOTION;
                }
                else if (!strcmp(value, "sharpen"))
                {
                    options->filter.value.as_filter = FILTER_SHARPEN;
                }
                else
                {
                    print_option_usage(options->filter, value);
                }
            }
            else if (!strcmp(options->size.cmd_name, argv[i]))
            {
                for (int i = 0; value[i] != '\0'; i++)
                {
                    if (!isdigit((unsigned char)value[i]))
                    {
                        print_option_usage(options->size, value);
                    }
                }

                int size = atoi(value);
                if (size % 2 == 1 && size >= 3 && size <= 13)
                {
                    options->size.value.as_int = size;
                }
                else
                {
                    print_option_usage(options->size, value);
                }
            }
            else if (!strcmp(options->mode.cmd_name, argv[i]))
            {
                if (!strcmp(value, "seq"))
                {
                    options->mode.value.as_mode = MODE_SEQ;
                }
                else if (!strcmp(value, "pixel"))
                {
                    options->mode.value.as_mode = MODE_PIXEL;
                }
                else if (!strcmp(value, "row"))
                {
                    options->mode.value.as_mode = MODE_ROW;
                }
                else if (!strcmp(value, "column"))
                {
                    options->mode.value.as_mode = MODE_COLUMN;
                }
                else if (!strcmp(value, "block"))
                {
                    options->mode.value.as_mode = MODE_BLOCK;
                }
                else
                {
                    print_option_usage(options->mode, value);
                }
            }
            else
            {
                invalid_arg(argv[i], *options);
            }
        }
        else if (!strcmp(options->clean.cmd_name, argv[i]) || !strcmp("-c", argv[i]))
        {
            options->clean.value.as_bool = true;
        }
        else if (!strcmp(options->help.cmd_name, argv[i]) || !strcmp("-h", argv[i]))
        {
            print_help(*options);
        }
        else
        {
            invalid_arg(argv[i], *options);
        }
    }
}

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
                         " edge, emboss, gaussian, motion (default: blur)"),

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

    return 0;
}
