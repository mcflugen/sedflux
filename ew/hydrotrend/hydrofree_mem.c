#include "hydrofree_mem.h"
#include <stdlib.h>

/**********************************************/
/*** for freeing memory (1D "matrices") ***/
/* format is: matrix name  */


void
freematrix1D(void* matrixname)
{
    free(matrixname);
}


/**********************************************/
/*** for freeing memory (2D matrices) ***/
/* format is: matrix name, number of rows */


void
freematrix2D(void** matrixname, int num_of_rows)
{
    int i;

    for (i = 0; i < num_of_rows; ++i) {
        free(matrixname[i]);
    }

    free(matrixname);
}


/**********************************************/
/*** for freeing memory (3D matrices) ***/
/* format is: matrix name, number of rows, number of columns */


void
freematrix3D(void*** matrixname, int num_of_rows, int num_of_columns)
{
    int i, j;

    for (i = 0; i < num_of_rows; ++i)
        for (j = 0; j < num_of_columns; ++j) {
            free(matrixname[i][j]);
        }

    for (i = 0; i < num_of_rows; ++i) {
        free(matrixname[i]);
    }

    free(matrixname);
}

