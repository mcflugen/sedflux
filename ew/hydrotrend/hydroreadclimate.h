#ifndef HYDROREADCLIMATE_H_
#define HYDROREADCLIMATE_H_



typedef struct {
    long n_steps, dt;
    double** R, P;
    double** T, *Tperyear;
} gw_rainfall_etc;

#endif

