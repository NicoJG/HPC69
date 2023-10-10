#pragma once

#include "global_vars.h"

typedef struct {
    int r;
    int g;
    int b;
} RGBColor;

#define MAX_COLOR_VALUE 255
#define CHARS_PER_CHANNEL 4
#define CHARS_PER_PIXEL (3 * CHARS_PER_CHANNEL)

char color_strings[10][CHARS_PER_PIXEL+1] = {
    "000 000 000 ",   // Black (not converged)
    "255 000 000 ",   // Red
    "000 255 000 ",   // Green
    "000 000 255 ",   // Blue
    "255 128 000 ",   // Orange
    "128 000 128 ",   // Purple
    "255 255 000 ",   // Yellow
    "000 255 255 ",   // Cyan
    "255 000 255 ",   // Magenta
    "128 128 128 "    // Gray
};

char greyscale_strings[MAX_COLOR_VALUE+1][CHARS_PER_PIXEL+1];

static inline
void prepare_greyscale_strings() {
    for (int i = 0; i < MAX_COLOR_VALUE+1; i++) {
        sprintf(greyscale_strings[i], "%03d %03d %03d\n", i, i, i);
    }
}

static inline
void write_ppm_header(FILE* file) {
    prepare_greyscale_strings(); 
    fprintf(file, "P3\n");
    fprintf(file, "%d %d\n", image_size, image_size);
    fprintf(file, "%d\n", MAX_COLOR_VALUE);
}

static inline
void copy_str(char* from_str, char* to_str, int length) {
    for (int i_char=0; i_char < length; i_char++) {
        to_str[i_char] = from_str[i_char]; 
    }
}

static inline
void write_attractors_row(FILE *file, TYPE_ATTR *root_idxs_row) {
    int str_length = CHARS_PER_PIXEL * image_size;
    char* str_line = (char*) malloc(sizeof(char) * str_length);
    for (int jx = 0; jx < image_size; jx++) {
        char *color_str = color_strings[root_idxs_row[jx] + 1];
        copy_str(color_str, str_line + jx*CHARS_PER_PIXEL, CHARS_PER_PIXEL);
    }
    fwrite(str_line, sizeof(char), image_size*CHARS_PER_PIXEL, file);
    free(str_line);
}


static inline
void write_convergence_row(FILE *file, TYPE_CONV *n_its_row) {
    int str_length = CHARS_PER_PIXEL * image_size;
    char* str_line = (char*) malloc(sizeof(char) * str_length);
    for (int jx = 0; jx < image_size; jx++) {
        int greyscale_value = (((double)n_its_row[jx]) / MAX_ITERATIONS) * MAX_COLOR_VALUE;
        char *greyscale_str = greyscale_strings[greyscale_value];
        copy_str(greyscale_str, str_line + jx*CHARS_PER_PIXEL, CHARS_PER_PIXEL);
    }
    fwrite(str_line, sizeof(char), image_size*CHARS_PER_PIXEL, file);
    free(str_line);
}

