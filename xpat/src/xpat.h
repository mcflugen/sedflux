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

#if !defined( XPAT_H )
#define XPAT_H

typedef struct
{
   double phi;      // median grain size
   double phi_min;  // minimum grain size
   double phi_max;  // maximum grain size
   double ws;       // settling velocity
   double tau_cr;   // critical shear stress for motion
}
Grain_class;

typedef struct
{
   double x_1;
   double x_2;
   double k;
   double v;
}
Advec_element;

typedef struct
{
   double dt;
   double t_end;
   int    bc_lower_type;
   double bc_lower_val;
   int    bc_upper_type;
   double bc_upper_val;
   double alpha;
}
Advec_const;

#define BC_TYPE_DIRICHLET (1)
#define BC_TYPE_NEUMAN    (2)

double pat_get_ripple_height( double d0 , double mean_grain_size );
double pat_get_vertical_diffusion( double z , double u_star_c , double delta_c , double u_star_w , double delta_w );
double *pat_get_diffusion_with_depth( double *k , double *z , double u_star_c , double u_star_w , double u_star_cw , double coriolis_freq , double wave_freq , int n );
double pat_get_coriolis_frequency( double lat );
Complex *pat_solve_velocity_profile( Complex *vel , double *k , double *z , double coriolis , Complex vel_geo , int n_bot , Complex vel_bottom , int n_top , Complex vel_top , int n );
double pat_get_skin_friction_shear( double tau_total , double roughness , double ripple_height , double ripple_length );
double pat_get_tau_max( double tau_sf , double tau_total , double ripple_height , double ripple_length );
double *pat_get_shear_stress_profile( double *tau , double *z , double *u , double *k , int n );
double *pat_get_total_current_stress( double *tau_total , double *tau_x , double *tau_y , int n );
double *pat_get_total_stress( double *tau_total , double *tau_x , double *tau_y , double *tau_wave , double wave_dir , int n );
double pat_get_u_star( double tau );
Complex *pat_iterate_velocity( Complex *vel_cur , double *k , double *z , int n , Complex vel_bbl_wave , Complex vel_bbl_cur , Complex vel_geo , double coriolis_freq , double wave_freq , double wave_dir , int n_bbl );
double pat_get_velocity_residual( double *z , Complex *vel , double *k , int n , double f , Complex vel_geo );
double average_grains( Grain_class *grain , double *amount , int n_grains , Grain_class *avg );
void set_grain_class( Grain_class *g , double phi_min , double phi , double phi_max , double ws , double tau_cr );

double get_roughness( double d_50 , double tau_cr , double tau_sf , int method );
double get_roughness_nik( double d_50 , double tau_sf );
double get_roughness_wiberg( double d_50 , double tau_cr , double tau_sf );
double get_roughness_dietrich( double d_50 , double tau_cr , double tau_sf );

double pat_get_mixing_depth_silt( double tau_sfm , double tau_cr , double d_50 );
double pat_get_mixing_depth_ripp( Grain_class *g , double *frac , int n_grains , double tau_sfm , double period , double c_b , double lambda_rip , double d_50 );
double pat_get_bedload_transport_rate( double grain_density , double water_density , double tau_sfm , double tau_cr );


double *pat_solve_concentration( double *c , double *z , double *k , int n , double w_s , double dc_dz , double dt );
double *advec( Advec_element *e , double *u , int n_nodes , Advec_const const );

#endif
