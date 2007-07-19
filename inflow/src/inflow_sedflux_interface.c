#include "sed_sedflux.h"
#include "inflow_local.h"
#include "inflow.h"

#define INFLOW_VELOCITY_RANGE               (3.0)
#define INFLOW_INITIAL_WIDTH                (1000.0)
#define INFLOW_INITIAL_HEIGHT               (6.0)
#define INFLOW_INITIAL_VELOCITY             (1.0)
#define INFLOW_INITIAL_CONCENTRATION        (.01)
#define INFLOW_GRAIN_DENSITY                (2650.)
#define INFLOW_SPREADING_ANGLE              (14.)
#define INFLOW_DENSITY_OF_SEDIMENT_GRAINS   (2650.)

void
inflow_erode_sediment( Sed_cube         p          ,
                       Inflow_bathy_st* bathy_data ,
                       gssize           i_start    ,
                       double**         erosion_in_m );
void
inflow_deposit_sediment( Sed_cube p                  ,
                         Inflow_bathy_st* bathy_data ,
                         gssize           i_start    ,
                         double** deposit_in_m );
double*
inflow_set_width_from_cube( Sed_cube p , gssize i_start );
Inflow_bathy_st*
inflow_set_bathy_data_from_cube( Sed_cube p ,
                                 double* width ,
                                 gssize ind_start ,
                                 double dx );
Inflow_sediment_st* inflow_set_sediment_data_from_env( );
void sed_get_phe( Inflow_phe_query_st* data , Sed_cube p );

double inflow_get_equivalent_diameter(double real_diameter);

gboolean
sed_inflow( Sed_cube         p       ,
            Sed_hydro        f       ,
            gint             i_start ,
            double           dx      ,
            Inflow_const_st* c )
{
   gboolean ok = TRUE;

   if ( p && f && c )
   {
      double**            deposit_in_m   = NULL;
      double**            erosion_in_m   = NULL;
      Inflow_flood_st*    daily_flood    = NULL;
      Inflow_bathy_st*    bathy_data     = NULL;
      Inflow_sediment_st* sediment_data  = NULL;
      double*             width          = NULL;
      double              total_t        = sed_hydro_duration(f)*S_SECONDS_PER_DAY;
      double              dt             = S_SECONDS_PER_DAY;
      double              t;

      c->get_phe      = sed_get_phe;
      c->get_phe_data = p;

      width         = inflow_set_width_from_cube       ( p , i_start );
      bathy_data    = inflow_set_bathy_data_from_cube  ( p , width , i_start , dx );
      sediment_data = inflow_set_sediment_data_from_env( );

      eh_require( width         );
      eh_require( bathy_data    );
      eh_require( sediment_data );

      deposit_in_m = eh_new_2( double , sediment_data->n_grains , bathy_data->len );
      erosion_in_m = eh_new_2( double , sediment_data->n_grains , bathy_data->len );

      for ( t=0 ; t<total_t ; t+=dt )
      {
         daily_flood = inflow_set_flood_data( f , c->rho_river_water );

         if ( t+dt > total_t )
            dt = total_t-t;

         daily_flood->duration  = dt;
         daily_flood->velocity += INFLOW_VELOCITY_RANGE*g_random_double();

         ok = inflow_wrapper( bathy_data , daily_flood , sediment_data , c , deposit_in_m , erosion_in_m );

         if ( ok )
         {
            inflow_erode_sediment  ( p , bathy_data , i_start , erosion_in_m );
            inflow_deposit_sediment( p , bathy_data , i_start , deposit_in_m );

            inflow_destroy_bathy_data( bathy_data );
            bathy_data = inflow_set_bathy_data_from_cube( p , width , i_start , dx );
         }

         inflow_destroy_flood_data( daily_flood );
      }

      inflow_destroy_bathy_data( bathy_data );
      eh_free_2( deposit_in_m );
      eh_free_2( erosion_in_m );
   }

   return ok;
}

void
inflow_erode_sediment( Sed_cube         p          ,
                       Inflow_bathy_st* bathy_data ,
                       gssize           i_start    ,
                       double**         erosion_in_m )
{
   if ( p && erosion_in_m )
   {
      gint i,  len;
      gint n,  n_grains = sed_sediment_env_size();
      double** erosion  = eh_new( double* , n_grains );
      double   total_t;
      double   dx       = bathy_data->x[1] - bathy_data->x[0];
      double   bin_size = sed_cube_y_res(p) / dx;

      for ( n=0 ; n<n_grains ; n++ )
      {
         eh_dbl_array_mult_each( erosion_in_m[n] , bathy_data->len , bathy_data->width );
         eh_dbl_array_mult     ( erosion_in_m[n] , bathy_data->len , dx/(sed_cube_x_res(p)*sed_cube_y_res(p)) );

         erosion[n] = eh_dbl_array_rebin( erosion_in_m[n] , bathy_data->len , bin_size , &len );
      }

      // Remove the sediment from the profile.
      for ( i=0 ; i<len ; i++ )
      {
         for ( n=0,total_t=0. ; n<n_grains ; n++ )
            total_t += erosion[n][i];
         sed_column_remove_top( sed_cube_col(p,i+i_start) , total_t );
      }

      for ( n=0 ; n<n_grains ; n++ )
         eh_free( erosion[n] );
      eh_free( erosion );
   }
}

void
inflow_deposit_sediment( Sed_cube p                  ,
                         Inflow_bathy_st* bathy_data ,
                         gssize           i_start    ,
                         double** deposit_in_m )
{
   if ( p && deposit_in_m )
   {
      gint i,  len          = bathy_data->len;
      gint n,  n_grains     = sed_sediment_env_size();
      double*  deposit_at_x = eh_new( double , n_grains );
      double** deposit      = eh_new( double* , n_grains );
      Sed_cell deposit_cell = sed_cell_new_env( );
      double   dx           = bathy_data->x[1] - bathy_data->x[0];
      double   bin_size     = sed_cube_y_res(p) / dx;

      for ( n=0 ; n<n_grains ; n++ )
      {
         eh_dbl_array_mult_each( deposit_in_m[n] , bathy_data->len , bathy_data->width );
         eh_dbl_array_mult     ( deposit_in_m[n] , bathy_data->len , dx/(sed_cube_x_res(p)*sed_cube_y_res(p)) );

         deposit[n] = eh_dbl_array_rebin( deposit_in_m[n] , bathy_data->len , bin_size , &len );
      }

      // Add the sediment to the profile.
      for ( i=0 ; i<len ; i++ )
      {
         for ( n=0 ; n<n_grains ; n++ )
            deposit_at_x[n] = deposit[n][i];

         sed_cell_clear     ( deposit_cell );
         sed_cell_set_age   ( deposit_cell      , sed_cube_age(p) );
         sed_cell_set_facies( deposit_cell      , S_FACIES_TURBIDITE );
         sed_cell_add_amount( deposit_cell      , deposit_at_x );

         sed_column_add_cell( sed_cube_col(p,i+i_start) , deposit_cell );
      }
      sed_cell_destroy( deposit_cell );
      eh_free( deposit_at_x );

      for ( n=0 ; n<n_grains ; n++ )
         eh_free( deposit[n] );
      eh_free( deposit );
   }
}

double*
inflow_set_width_from_cube( Sed_cube p , gssize i_start )
{
   double* width = NULL;

   {
      gint   i;
      double flow_width;
      gint   len   = sed_cube_n_y(p);
      double dx    = sed_cube_y_res(p);
      double alpha = tan( INFLOW_SPREADING_ANGLE*S_RADS_PER_DEGREE );

      width = eh_dbl_array_new_set( len , INFLOW_INITIAL_WIDTH );

      // Create a spreading angle.
      for ( i=i_start+1 ; i<len ; i++ )
      {
         flow_width = width[i-1] + alpha*dx;
         if ( flow_width < sed_cube_x_res(p) )
            width[i] = flow_width;
      }
   }

   return width;
}

Sed_hydro
inflow_flood_from_cell( Sed_cell c , double area )
{
   Sed_hydro h = NULL;

   if ( c )
   {
      h = sed_hydro_new( sed_cell_n_types(c)-1 );

      sed_hydro_set_width   ( h , INFLOW_INITIAL_WIDTH    );
      sed_hydro_set_depth   ( h , INFLOW_INITIAL_HEIGHT   );
      sed_hydro_set_velocity( h , INFLOW_INITIAL_VELOCITY );
      sed_hydro_set_bedload ( h , 0.                                 );

      {
         gint n;
         double* f = sed_cell_copy_fraction( NULL , c );

         eh_dbl_array_mult( f , sed_hydro_size(h) , INFLOW_INITIAL_CONCENTRATION );

         for ( n=0 ; n<sed_hydro_size(h) ; n++ )
            sed_hydro_set_nth_concentration( h , n , f[n] );

         eh_free( f );
      }

      {
         double volume_of_sediment = sed_cell_size_0( c )
                                   * area
                                   * sed_cell_density( c )
                                   / INFLOW_GRAIN_DENSITY;
         gint   n_days             = volume_of_sediment
                                   / sed_hydro_suspended_flux(h)
                                   * S_DAYS_PER_SECOND;

         sed_hydro_set_duration( h , n_days );
      }

      eh_require( sed_hydro_check( h , NULL ) );
   }

   return h;
}

Inflow_bathy_st*
inflow_set_bathy_data_from_cube( Sed_cube p , double* width , gssize ind_start , double dx )
{
   Inflow_bathy_st* b = NULL;

   if ( p )
   {
      gssize      len;
      gssize*     id        = eh_id_array( ind_start , sed_cube_size(p)-1 , &len );
      double**    bathy     = eh_new_2( double , 3 , sed_cube_size(p) );
      Eh_dbl_grid g         = sed_cube_water_depth_grid( p , id );
      double      basin_len;

      eh_require( id       );
      eh_require( bathy    );
      eh_require( bathy[0] );
      eh_require( bathy[1] );
      eh_require( bathy[2] );
      eh_require( g        );

      eh_require( len>=0                     );
      eh_require( len<sed_cube_size(p)       );
      eh_require( ind_start>=0               );
      eh_require( ind_start<sed_cube_size(p) );

      bathy[0] = sed_cube_y( p , id );
      bathy[1] = eh_dbl_grid_data(g)[0];
      bathy[2] = width + ind_start;

      eh_require( bathy[0] );
      eh_require( bathy[1] );
      eh_require( bathy[2] );

      basin_len = bathy[0][len-2] - bathy[0][0];

      b = inflow_set_bathy_data( bathy , len , dx , basin_len );

      eh_free( bathy[0] );
      eh_free( bathy    );
      eh_free( id       );
      eh_grid_destroy( g , TRUE );
   }

   return b;
}

Inflow_sediment_st*
inflow_set_sediment_data_from_env( )
{
   Inflow_sediment_st* s = eh_new( Inflow_sediment_st , 1 );
   gint n;

   s->n_grains      = sed_sediment_env_size();
   s->size_equiv    = sed_sediment_property( NULL , &sed_type_grain_size_in_meters );
   s->lambda        = sed_sediment_property( NULL , &sed_type_lambda_in_per_seconds );
   s->bulk_density  = sed_sediment_property( NULL , &sed_type_rho_sat );
   s->grain_density = eh_dbl_array_new_set( s->n_grains , INFLOW_GRAIN_DENSITY );

   for ( n=0 ; n<s->n_grains ; n++ )
      s->size_equiv[n] = inflow_get_equivalent_diameter( s->size_equiv[n] );

   return s;
}

/** Get the grain size distribution of bottom sediments.

Get the fractions of each grain type of bottom sediments from a
Sed_cube.  This function is intended to be used within another program that
needs to communicate with the sedflux architecture but is otherwise separate
from sedflux.

Note that the member, eroded_depth may be changed to reflect the actual amount
of bottom sediment available to be eroded.  That is, we may be trying to erode
more sediment than is actually present.

\param data    A structure that contains the necessary data for the function to
               retreive the grain type fracitons.
\param p       A Sed_cube to query

\return       A pointer to the array of grain type fractions.

*/

void
sed_get_phe( Inflow_phe_query_st* data , Sed_cube p )
{
   Sed_cube prof  = (Sed_cube)p;
   double dx      = data->dx;
   double x       = data->x;
   double depth   = data->erode_depth;
   double *phe    = data->phe;
   Sed_cell avg;
   double volume;
   int i, n, n_grains;
   
   n_grains = sed_sediment_env_size();
   avg      = sed_cell_new_env( );

   if ( depth > 0 )
   {
      // Determine which column of the profile to remove sediment from.
      i = (int)(x/sed_cube_y_res(prof));
      if ( i<0 ) i=0;
      eh_lower_bound( i , 0 );
      eh_upper_bound( i , sed_cube_n_y(prof)-1 );

      // The grid used by inflow will be smaller than that used by sedflux.
      // As such, we reduce the erosion depth but remove from the entire width
      // of the cell in such a way that mass is conserved.
      depth *= dx/sed_cube_y_res( prof );

      eh_upper_bound( depth , sed_cube_thickness(prof,0,i) );
   
      // Remove different grain sizes equally.  We can change this so that
      // sands are more easily eroded than clays -- or whatever we want,
      // really.
      sed_column_extract_top( sed_cube_col(prof,i) , depth , avg );

      for ( n=0 ; n<n_grains ; n++ )
         phe[n] = sed_cell_nth_fraction( avg , n );

      // We want to return the amount of sediment that was removed.  That is,
      // sediment - JUST SEDIMENT, DRY SEDIMENT - not sediment plus water.
      for ( n=0 , volume=0. ; n<n_grains ; n++ )
         volume += depth
                 * phe[n]
                 * sed_type_rho_sat( sed_sediment_type( NULL , n ) )
                 / INFLOW_DENSITY_OF_SEDIMENT_GRAINS;
   }
   else
   {
      for ( n=0 ; n<n_grains ; n++ )
         phe[n] = 0.;
      volume = 0.;
   }

   sed_cell_destroy( avg );

   // save the volume that was actually eroded.
   data->erode_depth = volume;

   return;
}

/** Calculate the equivalent grain size.

Sediment grains falling through water will settle a rates that are greater than
those predicted because of flocculation.  That is, a grain will settle at a rate predicted
for a larger grain.  This function calculates this "equivalent" grain size, given the real
grain size.

\param real_diameter The real grain size in meters.

\return The equivalent grain size in meters.
*/

double inflow_get_equivalent_diameter(double real_diameter)
{
/* double m=0.2,b=220e-6;
   double m=0.2,b=100e-6;
   double m=0.2,b=200e-6;
   return m*real_diameter+b;
*/
   double a=39.8e-3, b=.6;
   return a*pow(real_diameter,b);
}

