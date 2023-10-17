#include <stdio.h>
#include <stdlib.h>

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

        short line_length = 32;
        char *line = (char*) malloc(sizeof(char) * line_length);

        while (fgets(line, line_length, fp) != NULL) {
            sscanf(line, "%d %d %f", &x, &y, &init);

            // remember that we use a zero border offsetting all indices by one
            linear_ind = width * (y + 1) + (x + 1);
            (*vector)[linear_ind] = init;
        }
        free(line);
    }

