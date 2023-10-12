#include <stdio.h>
#include <stdlib.h>


// I did this first when I was stupid and forgot about sscanf, ffscanf etc.
// It's all manual parsing so might be faster than sscanf but it's so much more
// complicated... I leave it here for now but don't use it unless we need to
// Isak
static inline
void
read_header_manually(
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

    // Set reading position to next line so we are ready to read coordinates
    fseek(fp, i + j + 1, SEEK_SET);

    free(width_nums);
    free(height_nums);
    free(header);
}

static inline
void
read_header(
    FILE *fp,
    int *width,
    int *height
    ) {

        short header_sz = 16;
        char *header = (char *) malloc(sizeof(char) * header_sz);

        // make sure width and height are initialised
        *width = 0;
        *height = 0;

        // read first line
        fgets(header, header_sz, fp);
        sscanf(header, "%d %d", width, height);

        free(header);
    }

static inline
void 
read_and_initialise(
    FILE *fp,
    int width,
    int height,
    float **vector
    ) {
        int x, y, linear_ind;
        float init;

        // Let''s read line by line (not sure how to allocate a suitable amount of space otherwise)
        short line_length = 32;
        char *line = (char*) malloc(sizeof(char) * line_length);

        while (fgets(line, line_length, fp) != NULL) {
            sscanf(line, "%d %d %f", &x, &y, &init);
            linear_ind = width * y + x;
            (*vector)[linear_ind] = init;
        }
        free(line);
    }

