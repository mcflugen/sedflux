//---
//
// This file is part of sedflux.
//
// sedflux is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// sedflux is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with sedflux; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//---

#if !defined(PLUME_TYPES_H)
# define PLUME_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double* x;   // i's
    double* y;   // j's
    double*** z; // (n,i,j)
    int imax, jmax, nmax;
    double total_mass;
} grid_type_3d;

typedef struct {
    double* x;   // i's
    double** z;  // (n,i)
    int xmax;
} grid_type_2d;

typedef struct {
    double Cw;         // ocean sediment concentration (kg/m^3) [0:min(Cs)]
    double vo;         // alongshore current magnitude (m/s) [-3:3]
    double vdirection; // alongshore current direction (degN) [0:360]
    double cc;         // Coastal Current width = cc*inertial length scale [0.1:1]
    double So;         // Conservative Tracer Property, River concentration
    double Sw;         // Conservative Tracer Property, Ocean concentration
}
Plume_ocean;

typedef struct {
    double* Cs;        // River Concentration (kg/m^3) [0.001:100]
    double Q;          // discharge (m^3/s) [1:1e6]
    double u0;         // velocity (m/s) [0.01:10]
    double rdirection; // river mouth direction (degN) [0:360]
    double b0;         // river mouth width (m) [1.0:1e5]
    double d0;         // river depth (m) [calculated]
    double rma;        // River mouth angle (degrees), + is in plus y dir.
}
Plume_river;

typedef struct {
    double lambda;    // removal rate coefficient, input (1/day) [0.1:40]
    // immediately convert to (1/s)
    double rho;       // density of sediment (kg/m^3) [1100:2600]
    double grainsize; // used by SEDFLUX3D
    double diff_coef; // used by SEDFLUX3D
}
Plume_sediment;

// user defined variables for the 2DSEDFLUX plume.
typedef struct {
    double current_velocity;
    double ocean_concentration;
    double plume_width;
    int ndx;
    int ndy;
}
Plume_inputs;

typedef struct {
    int fjrd;
    int kwf;
    int strt;
    int o1;
    int o2;
    int o3;
}
Plume_options;

// the various grids used for the plume.
typedef struct {
    double*** ccnc;
    double*** ncnc;
    double*** deps;
    double*** dist;
    double** ualb;
    double** pcent;
    double** sln;
    double* xval;
    double* yval;
    int lc;
    int lpc;
    int lx;
    int ly;
    int lz;
    int zx;
    int zy;
    double ymax;
    double ymin;
    double xmax;
    double xmin;
    double dy;
    double dx;
    double max_len;
    int y_len;
    int x_len;
    int ndy;
    int ndx;
}
Plume_grid;

typedef struct {
    double Qsr;
    double Qsw[4];
    double Tsr;
    double Tsd[2];
    double merr;
}
Plume_mass_bal;

typedef struct {
    Plume_river*    river;
    Plume_sediment* sed;
    Plume_ocean*    ocean;
    int             n_grains;
    int             lat;
}
Plume_enviro;

typedef Plume_grid Plume_data;

#ifdef __cplusplus
}
#endif

#endif

