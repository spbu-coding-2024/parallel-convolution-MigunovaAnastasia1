// util functions for proc_image.c

unsigned char *load_image(const char *filename, int *width, int *height, int *channels);

void store_image(const char *filename, int width, int height, unsigned char *data);

unsigned char *RGB2grayscale(unsigned char *image_data, int width, int height, int channels);
