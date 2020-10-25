/*
 *  HydroAlloc_Mem.h
 *
 *  Allocate memory
 *
 *  Author:         A.J. Kettner (September 2002)
 *
 */


#ifndef HYDROALLOC_MEM_H_
#define HYDROALLOC_MEM_H_

#include <stdio.h>
#define malloc1d( m , type) ( (type*)matrixalloc1D( m, sizeof(type)) )
#define malloc2d( m , n , type ) ( (type**)matrixalloc2D( m , n , sizeof(type)) )
#define malloc3d( m , n , o , type ) ( (type***)matrixalloc3D( m , n , o , sizeof(type)) )


/*      FUNCTION DEFINITIONS */
FILE**
allocate_1d_F(int nrows);
void*
matrixalloc1D(int, long);
void**
matrixalloc2D(int, int, long);
void***
matrixalloc3D(int, int, int, long);
#endif

