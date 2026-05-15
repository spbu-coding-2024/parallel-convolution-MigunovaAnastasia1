#include "option_types.h"

void parse_arguments(int argc, char *argv[], Options *options);
char *get_default_input(char *dirctory);
char **get_image_paths(char *directory);
char **get_output_paths();
char **get_paths(char *directory);
void free_paths(char **paths);