#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SZ 512
#define COORD_NBR 5 // Could change to BUFFER_SZ / ( 3 * 8 )
#define FLOAT_LENGTH 8


int merge_char(char *buffer, size_t ix, int which_coord);

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


    int *x = (int *) malloc(sizeof(int) * COORD_NBR);
    int *y = (int *) malloc(sizeof(int) * COORD_NBR);
    int *z = (int *) malloc(sizeof(int) * COORD_NBR);

    for (size_t ix = 0, jx = 0; ix <= 3* COORD_NBR * FLOAT_LENGTH; ix += 3 * FLOAT_LENGTH, jx++) {
        
	x[jx] = merge_char(buffer, ix, 0);
	y[jx] = merge_char(buffer, ix, 1);
	z[jx] = merge_char(buffer, ix, 2);
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

int merge_char(char *buffer, size_t ix, int which_coord){

    	int base, bbase, dec, ddec, dddec, sign = 1, num;
        
	if (buffer[ix + which_coord * FLOAT_LENGTH] == '-') {
            sign = -1;
        }

        base = *(buffer + ix + which_coord * FLOAT_LENGTH + 1) - '0';
        bbase = *(buffer + ix + which_coord * FLOAT_LENGTH + 2 ) - '0';
        dec = *(buffer + ix + which_coord * FLOAT_LENGTH + 4 ) - '0';
        ddec = *(buffer + ix + which_coord * FLOAT_LENGTH + 5 ) - '0';
        dddec = *(buffer + ix + which_coord * FLOAT_LENGTH + 6 ) - '0';

        num = sign * (
            base * 10000 + 
            bbase * 1000 +
            dec * 100 + 
            ddec * 10 +
            dddec);

	return num;
}
