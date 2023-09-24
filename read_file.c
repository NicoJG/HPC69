#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SZ 512

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
    int base, bbase, dec, ddec, dddec, sign, num;
    int coord = 0;
    int count_x = 0;
    int count_y = 0;
    int count_z = 0;

    int *x = (int *) malloc(sizeof(int) * 100);
    int *y = (int *) malloc(sizeof(int) * 100);
    int *z = (int *) malloc(sizeof(int) * 100);

    for (size_t ix = 0; ix <= 30 * float_length; ix += float_length) {

        if (buffer[ix] == '-') {
            sign = -1;
        }
        
        base = *(buffer + ix + 1) - '0';
        bbase = *(buffer + ix + 2) - '0';
        dec = *(buffer + ix + 4) - '0';
        ddec = *(buffer + ix + 5) - '0';
        dddec = *(buffer + ix + 6) - '0';

        num = sign * (
            base * 10000 + 
            bbase * 1000 +
            dec * 100 + 
            ddec * 10 +
            dddec);
        
        sign = 1;
        
        if (coord == 0) {
            x[count_x] = num;
            count_x++;
            coord +=1;
        } else if (coord == 1) {
            y[count_y] = num;
            count_y++;
            coord += 1;
        } else {
            z[count_z] = num;
            count_z++;
            coord = 0;
        }
    }
    
    for (size_t ix = 0; ix < 100; ix++) {
        printf("%d, %d, %d\n", x[ix], y[ix], z[ix]);
    }
    fseek(fp, 0, SEEK_END);
    printf("The file has size %li\n", ftell(fp));
  
    fclose(fp);
    return 0;

}