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

RGBColor colors[10] = {
    {0, 0, 0},       // Black (not converged)
    {255, 0, 0},     // Red
    {0, 255, 0},     // Green
    {0, 0, 255},     // Blue
    {255, 128, 0},   // Orange
    {128, 0, 128},   // Purple
    {255, 255, 0},   // Yellow
    {0, 255, 255},   // Cyan
    {255, 0, 255},   // Magenta
    {128, 128, 128}  // Gray
};

void write_ppm_header(FILE* file) {
    fprintf(file, "P3\n");
    fprintf(file, "%d %d\n", image_size, image_size);
    fprintf(file, "%d\n", MAX_COLOR_VALUE);
}

void write_attractors_row(FILE *file, short *root_idxs_row) {
    int str_length = CHARS_PER_PIXEL * image_size;
    char *str_row = (char*) malloc(sizeof(char) * (str_length + 1)); // +1 for the string terminating character

    for (int jx = 0; jx < image_size; jx++) {
        RGBColor color = colors[root_idxs_row[jx] + 1];
        sprintf(str_row + jx*CHARS_PER_PIXEL, "%03d %03d %03d ", color.r, color.g, color.b);
    }
    sprintf(str_row + str_length - 1, "\n");

    fwrite(str_row, sizeof(char), str_length, file);

    free(str_row);
}

void write_convergence_row(FILE *file, short *n_its_row) {
    int str_length = CHARS_PER_PIXEL * image_size;
    char *str_row = (char*) malloc(sizeof(char) * (str_length + 1)); // +1 for the string terminating character

    for (int jx = 0; jx < image_size; jx++) {
        int greyscale_value = (((double)n_its_row[jx]) / MAX_ITERATIONS) * MAX_COLOR_VALUE;
        sprintf(str_row + jx*CHARS_PER_PIXEL, "%03d %03d %03d ", greyscale_value, greyscale_value, greyscale_value);
    }
    sprintf(str_row + str_length - 1, "\n");

    fwrite(str_row, sizeof(char), str_length, file);

    free(str_row);
}