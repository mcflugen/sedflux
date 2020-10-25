#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hydroalloc_mem.h"

/*      FUNCTION ALLOCATE_1D FILE */
FILE**
allocate_1d_F(int nrows)
{
    FILE** i;

    i = (FILE**)malloc(nrows * sizeof(FILE*));

    if (!i) {
        perror("allocate_1d_F");
        exit(-1);
    }

    return i;
}


/************************************************/
/***...for allocating a 1D "matrix" of whatever!***/
/*NOTE: I'll need to typecast the result of this to whatever
  is being turned into a matrix.  e.g., double, struct, etc!*/

/* note that sizeof(void*) is the same as sizeof(double*) is the
   same as sizeof(float*), etc.  A pointer always has the same
   size (the size of the memory address.) */

void*
matrixalloc1D(int max_width, long size)
{

    void* atemp;

    atemp = (void*) malloc(max_width * size);

    if (!atemp) {
        perror("matrixalloc_1");
        sleep(5);
        exit(1);
    }

    return atemp;
}


/************************************************/
/***...for allocating a 2D matrix of whatever!***/
/*NOTE: I'll need to typecast the result of this to whatever
  is being turned into a matrix.  e.g., double, struct, etc!*/

/* note that sizeof(void*) is the same as sizeof(double*) is the
   same as sizeof(float*), etc.  A pointer always has the same
   size (the size of the memory address.) */

void**
matrixalloc2D(int max_width, int max_length, long size)
{
    int i;

    void** atemp;

    atemp = (void**) malloc(max_width * sizeof(void*));

    if (!atemp) {
        perror("matrixalloc_2");
        sleep(5);
        exit(1);
    }

    for (i = 0; i < max_width; ++i) {
        atemp[i] = (void*) malloc(max_length * size);

        if (!atemp[i]) {
            perror("matrixalloc_3");
            sleep(5);
            exit(1);
        }
    }

    return atemp;
}

/************************************************/
/***...for allocating a 3D matrix of whatever!***/
/*NOTE: I'll need to typecast the result of this to whatever
  is being turned into a matrix.  e.g., double, struct, etc!*/

void***
matrixalloc3D(int max_width, int max_length, int max_height, long size)
{
    int i, j;

    void*** atemp;

    atemp = (void***) malloc(max_width * sizeof(void*));

    if (!atemp) {
        perror("matrixalloc_4");
        sleep(5);
        exit(1);
    }

    for (i = 0; i < max_width; ++i) {
        atemp[i] = (void**) malloc(max_length * sizeof(void*));

        if (!atemp[i]) {
            perror("matrixalloc_5");
            sleep(5);
            exit(1);
        }
    }

    for (i = 0; i < max_width; ++i)
        for (j = 0; j < max_length; ++j) {
            atemp[i][j] = (void*) malloc(max_height * size);

            if (!atemp[i][j]) {
                perror("matrixalloc_6");
                sleep(5);
                exit(1);
            }
        }

    return atemp;
}
