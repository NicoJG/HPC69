#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SZ 512
#define COORD_NBR 5

int 
main (
    int argc,
    char *argv[]
) {

    FILE *fp;
    char *file_name = "data/cells_1e2";

    fp = fopen(file_name, "rb");
    if (fp == NULL) {
        perror("File could not be opened.");
        return 1;
    }

    char buffer[BUFFER_SZ];
    fseek(fp, 0, SEEK_SET);
    fread(buffer, sizeof(buffer), 1, fp);

    int float_length = 8;
    int base, bbase, dec, ddec, dddec, sign;

    int *x = (int *) malloc(sizeof(int) * 20);
    int *y = (int *) malloc(sizeof(int) * 20);
    int *z = (int *) malloc(sizeof(int) * 20);

    for (size_t ix, jx = 0; ix <= 3* COORD_NBR * float_length; ix += 3 * float_length, jx++) {

	// x
        if (buffer[ix] == '-') {
            sign = -1;
        }
        
        base = *(buffer + ix + 1) - '0';
        bbase = *(buffer + ix + 2) - '0';
        dec = *(buffer + ix + 4) - '0';
        ddec = *(buffer + ix + 5) - '0';
        dddec = *(buffer + ix + 6) - '0';

        x[jx] = sign * (
            base * 10000 + 
            bbase * 1000 +
            dec * 100 + 
            ddec * 10 +
            dddec);
        
	// y
        if (buffer[ix + float_length] == '-') {
            sign = -1;
        }

        base = *(buffer + ix + 1 + float_length) - '0';
        bbase = *(buffer + ix + 2 + float_length) - '0';
        dec = *(buffer + ix + 4 + float_length) - '0';
        ddec = *(buffer + ix + 5 + float_length) - '0';
        dddec = *(buffer + ix + 6 + float_length) - '0';

        y[jx] = sign * (
            base * 10000 + 
            bbase * 1000 +
            dec * 100 + 
            ddec * 10 +
            dddec);

	// z
        if (buffer[ix + 2 * float_length] == '-') {
            sign = -1;
        }

        base = *(buffer + ix + 1 + 2 * float_length) - '0';
        bbase = *(buffer + ix + 2 + 2 * float_length) - '0';
        dec = *(buffer + ix + 4 + 2 * float_length) - '0';
        ddec = *(buffer + ix + 5 + 2 * float_length) - '0';
        dddec = *(buffer + ix + 6 + 2 * float_length) - '0';

        z[jx] = sign * (
            base * 10000 + 
            bbase * 1000 +
            dec * 100 + 
            ddec * 10 +
            dddec);
    }
    
    for (size_t ix = 0; ix < COORD_NBR; ix++) {
        printf("%d, %d, %d\n", x[ix], y[ix], z[ix]);
    }
    fseek(fp, 0, SEEK_END);
    printf("The file has size %li\n", ftell(fp));
  
    fclose(fp);
    
    free(x);
    free(y);
    free(z);

    return 0;

}
