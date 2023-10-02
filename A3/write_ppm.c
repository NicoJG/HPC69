#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int r;
    int g;
    int b;
} RGBColor;

int 
main (
    int argc,
    char *argv[]
) {
    
    int L = 1000;
    int m = 255;
    int no_colors = 9;
    int rgb_str_len = 10;

    RGBColor colors[9] = {
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


    char *str = (char *) malloc(sizeof(char) * rgb_str_len * no_colors);
    char **rgb_strings = (char **) malloc(sizeof(char *) * no_colors);

    for (size_t i = 0, j = 0; i < no_colors; i++, j += rgb_str_len) {
        rgb_strings[i] = str + j;
    }

    for (size_t i = 0; i < no_colors; ++i) {
        sprintf(rgb_strings[i], "%d %d %d\n", colors[i].r, colors[i].g, colors[i].b);
    }

    printf("%s", rgb_strings[8]);

    free(rgb_strings);
    free(str);

}