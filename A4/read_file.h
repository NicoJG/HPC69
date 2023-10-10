#include <stdio.h>
#include <stdlib.h>

static inline
void
read_header(
    FILE *fp,
    int *width,
    int *height
) {
    // Might be overkill?
    fseek(fp, 0, SEEK_SET);

    // make sure width and height are initialised
    *width = 0;
    *height = 0;

    int header_sz = 16;
    char *header = (char *) malloc(sizeof(char) * header_sz);
    int *width_nums = (int *) malloc(sizeof(int) * 8);
    int *height_nums = (int *) malloc(sizeof(int) * 8);
    int base;
    
    // Read header into a char pointer
    fread(header, header_sz, 1, fp);
    
    int i = 0;
    while (*(header + i) != ' ') {
        width_nums[i] = *(header + i) - '0';
        i += 1;
    }

    base = 1;
    for (size_t ix = i - 1; ix != -1; --ix) {
        *width += width_nums[ix] * base;
        base *= 10;
    }
    i += 1;
    int j = 0;
    while (*(header + i + j) != '\n') {
        height_nums[j] = *(header + i + j) - '0';
        j += 1;
    }

    base = 1;
    for (size_t ix = j - 1; ix != -1; --ix) {
        (*height) += height_nums[ix] * base; 
        base *= 10;
    }

    free(width_nums);
    free(height_nums);
    free(header);
}   