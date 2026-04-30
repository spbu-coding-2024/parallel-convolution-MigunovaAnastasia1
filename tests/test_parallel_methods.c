#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include "proc_image.h"
#include "core_builder.h"
#include "proc_image_utils.h"

#define INPUT_IMAGE "./../images/569x339.jpg"
typedef struct
{
	unsigned char *input_image;
	unsigned char *seq_result;
	unsigned char *parallel_result;
	Kernel *kernel;
	int width;
	int height;
	int channels;
} TestContext;

#define TEST(method_under_test)                                                                         \
	seq_conv(ctx->input_image, ctx->width, ctx->height, *(ctx->kernel), ctx->seq_result);               \
	method_under_test(ctx->input_image, ctx->width, ctx->height, *(ctx->kernel), ctx->parallel_result); \
	assert_images_equal(ctx->seq_result, ctx->parallel_result, ctx->width, ctx->height);

// ============ Setup & Teardown ============

static int setup(void **state)
{
	TestContext *ctx = malloc(sizeof(TestContext));
	assert_non_null(ctx);

	ctx->input_image = load_image(INPUT_IMAGE,
								  &ctx->width, &ctx->height, &ctx->channels);

	ctx->input_image = RGB2grayscale(ctx->input_image,
									 ctx->width, ctx->height, ctx->channels);

	ctx->kernel = kernel_builder(FILTER_MOTION, 3);

	ctx->seq_result = malloc(ctx->width * ctx->height);
	assert_non_null(ctx->seq_result);

	ctx->parallel_result = malloc(ctx->width * ctx->height);
	assert_non_null(ctx->parallel_result);

	*state = ctx;
	return 0;
}

static int teardown(void **state)
{
	TestContext *ctx = *state;
	if (ctx)
	{
		free(ctx->input_image);
		free(ctx->seq_result);
		free(ctx->parallel_result);
		kernel_free(ctx->kernel);
		free(ctx);
	}
	return 0;
}

//=========== Image equality function =========

static void assert_images_equal(unsigned char *expected, unsigned char *actual,
								int width, int height)
{
	for (int i = 0; i < width * height; i++)
	{
		assert_int_equal(expected[i], actual[i]);
	}
}

// ============ Tests for parallel methods ============

static void test_pixels_method(void **state)
{
	TestContext *ctx = *state;

	TEST(pixel_parallel_conv)
}

static void test_rows_method(void **state)
{
	TestContext *ctx = *state;

	TEST(row_parallel_conv)
}

static void test_columns_method(void **state)
{
	TestContext *ctx = *state;

	TEST(column_parallel_conv)
}

static void test_blocks_method(void **state)
{
	TestContext *ctx = *state;

	TEST(block_parallel_conv)
}

// ============ Main ============

int main(void)
{
	const struct CMUnitTest tests[] = {

		cmocka_unit_test_setup_teardown(test_pixels_method,
										setup, teardown),
		cmocka_unit_test_setup_teardown(test_rows_method,
										setup, teardown),
		cmocka_unit_test_setup_teardown(test_columns_method,
										setup, teardown),
		cmocka_unit_test_setup_teardown(test_blocks_method,
										setup, teardown),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
};