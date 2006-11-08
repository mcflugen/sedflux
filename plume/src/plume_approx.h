#if !defined( PLUME_APPROX_H )
#define PLUME_APPROX_H

#define PLUME_M1    (1.2)
#define PLUME_M2    (0.6)
#define PLUME_P1    (0.8)
#define PLUME_F2    (0.9)
#define PLUME_SIGMA (6.4872)
#define PLUME_XA    (5.176)

#include "sed_sedflux.h"

double plume_centerline_inv_at( double x , double l );
double* plume_centerline_inv_nd( double* inv , double* x , gssize len , double l );
double* plume_centerline_inv( double* inv , double* x   , gssize len ,
                              double  l   , double  i_0 , Sed_hydro r );
double* plume_centerline_deposit_nd( double* dep , double* x , gssize len , double l );
Sed_cell_grid plume_centerline_deposit( Sed_cell_grid g , Sed_hydro r , Sed_sediment s );

double* plume_inv_nd( double* dep , double* x , double* s ,  gssize len , double l );
Sed_cell_grid plume_width_averaged_deposit( Sed_cell_grid g , Sed_hydro r , Sed_sediment s , double dy );
double* plume_width_averaged_deposit_nd( double* dep , double* x , gssize len , double l );

double** plume_i_bar( double* x , gssize n_x , double l , gssize* n_y , double dy );
double* plume_i_bar_at_x( double x , double dx , double dy , gssize* n_y , double l , double* i_bar_last );

double plume_non_dim_distance( double x , Sed_hydro r );
double plume_non_dim_lambda( double l , Sed_hydro r );

double plume_half_width         ( double x );
double plume_established_width  ( double x );
double plume_establishment_width( double x );
double plume_plug_width         ( double x );

Sed_cell_grid plume_width_averaged_deposit_num( Sed_cell_grid g , Sed_hydro r , Sed_sediment s , double dy );
double* plume_width_averaged_deposit_nd_num( double* dep , double* x , gssize len , double l );

#endif

