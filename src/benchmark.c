#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli_utils.h"
#include "proc_image.h"
#include "option_types.h"
#include "core_builder.h"
#include "proc_image_utils.h"

#define NUM_RUNS 20
#define INPUT_FILE "./../images/7680x4320.jpg"

#define BENCH(method)                                        \
                                                             \
    for (int i = 0; i < NUM_RUNS; i++)                       \
    {                                                        \
        start = omp_get_wtime();                             \
        method(image, width, height, *kernel, result_image); \
        end = omp_get_wtime();                               \
        times[i] = end - start;                              \
    }

static double average(double times[])
{
    double sum = 0;
    for (int i = 0; i < NUM_RUNS; i++)
        sum += times[i];
    return sum / NUM_RUNS;
}

static void run_benchmark(unsigned char *image, int width, int height, Kernel *kernel, ModeType mode, double *times)
{

    unsigned char *result_image = (unsigned char *)malloc(width * height);
    if (result_image == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory to store convolution result.\n");
        exit(-1);
    }

    double start;
    double end;

    {
        switch (mode)
        {
        case MODE_SEQ:
            BENCH(seq_conv)
            break;
        case MODE_PIXEL:
            BENCH(pixel_parallel_conv)
            break;
        case MODE_ROW:
            BENCH(row_parallel_conv)
            break;
        case MODE_COLUMN:
            BENCH(column_parallel_conv)
            break;
        case MODE_BLOCK:
            BENCH(block_parallel_conv)
            break;
        default:
            fprintf(stderr, "Error: Unknown mode\n");
            exit(-1);
        }
    }
    free(result_image);
}

int main(int argc, char **argv)
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

    Kernel *kernel = kernel_builder(options.filter.value.as_filter, options.size.value.as_int);

    int width, height, channels;
    unsigned char *image = load_image(INPUT_FILE, &width, &height, &channels);
    image = RGB2grayscale(image, width, height, channels);

    /*=======run benchmarks=============*/

    double seq_times[NUM_RUNS];
    double pixel_times[NUM_RUNS];
    double row_times[NUM_RUNS];
    double column_times[NUM_RUNS];
    double block_times[NUM_RUNS];

    run_benchmark(image, width, height, kernel, MODE_SEQ, seq_times);
    printf("Seq: %.4f", seq_times[0]);
    for (int i = 1; i < NUM_RUNS; i++)
    {
        printf(",%.4f", seq_times[i]);
    }
    printf("\n\n");

    run_benchmark(image, width, height, kernel, MODE_PIXEL, pixel_times);
    printf("Pixel: %.4f", pixel_times[0]);
    for (int i = 1; i < NUM_RUNS; i++)
    {
        printf(",%.4f", pixel_times[i]);
    }
    printf("\n\n");

    run_benchmark(image, width, height, kernel, MODE_ROW, row_times);
    printf("Row: %.4f", row_times[0]);
    for (int i = 1; i < NUM_RUNS; i++)
    {
        printf(",%.4f", row_times[i]);
    }
    printf("\n\n");

    run_benchmark(image, width, height, kernel, MODE_COLUMN, column_times);
    printf("Column: %.4f", column_times[0]);
    for (int i = 1; i < NUM_RUNS; i++)
    {
        printf(",%.4f", column_times[i]);
    }
    printf("\n\n");

    run_benchmark(image, width, height, kernel, MODE_BLOCK, block_times);
    printf("Block: %.4f", block_times[0]);
    for (int i = 1; i < NUM_RUNS; i++)
    {
        printf(",%.4f", block_times[i]);
    }

    printf("\n\n\n");

    printf("TOTAL RESULTS (avarage times):\n");
    printf("    Seq:.....................%.4f\n", average(seq_times));
    printf("    Pixel:...................%.4f\n", average(pixel_times));
    printf("    Row:.....................%.4f\n", average(row_times));
    printf("    Column:..................%.4f\n", average(column_times));
    printf("    Block:...................%.4f\n", average(block_times));

    /*=======end benchmarks=============*/

    free(image);
    free(options.input.value.as_string);
    kernel_free(kernel);

    return 0;
}
