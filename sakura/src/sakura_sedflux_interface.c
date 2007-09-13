#include "sed_sedflux.h"
#include "sakura_local.h"
#include "sakura.h"

#define SAKURA_VELOCITY_RANGE               (3.0)
#define SAKURA_INITIAL_WIDTH                (1000.0)
#define SAKURA_INITIAL_HEIGHT               (6.0)
#define SAKURA_INITIAL_VELOCITY             (1.0)
#define SAKURA_INITIAL_CONCENTRATION        (.01)
#define SAKURA_GRAIN_DENSITY                (2650.)
#define SAKURA_SPREADING_ANGLE              (14.)
#define SAKURA_DENSITY_OF_SEDIMENT_GRAINS   (2650.)

void
sakura_erode_sediment( Sed_cube         p          ,
                       Sakura_bathy_st* bathy_data ,
                       gssize           i_start    ,
                       double**         erosion_in_m );
void
sakura_deposit_sediment( Sed_cube p                  ,
                         Sakura_bathy_st* bathy_data ,
                         gssize           i_start    ,
                         double** deposit_in_m );
double*
sakura_set_width_from_cube( Sed_cube p , gssize i_start );
Sakura_bathy_st*
sakura_set_bathy_data_from_cube( Sed_cube p ,
                                 double* width ,
                                 gssize ind_start ,
                                 double dx );
Sakura_sediment_st* sakura_set_sediment_data_from_env( );
/*
void   sed_get_phe  ( Sakura_array_st* bed_data    , Sed_cube p );
double sed_add      ( Sakura_array_st* add_data    , gint ind , double dh );
double sed_remove   ( Sakura_array_st* remove_data , gint ind , double dh );
double sed_get_depth( Sakura_array_st* depth_data  , gint ind );
*/

double sakura_get_equivalent_diameter(double real_diameter);

gboolean
sed_sakura( Sed_cube         p       ,
            Sed_hydro        f       ,
            gint             i_start ,
            double           dx      ,
            Sakura_const_st* c )
{
   gboolean ok = TRUE;

   if ( p && f && c )
   {
      double**            deposit_in_m   = NULL;
      Sakura_flood_st*    daily_flood    = NULL;
      Sakura_bathy_st*    bathy_data     = NULL;
      Sakura_sediment_st* sediment_data  = NULL;
      double*             width          = NULL;
      double              total_t        = sed_hydro_duration(f)*S_SECONDS_PER_DAY;
      double              dt             = S_SECONDS_PER_DAY;
      double              t;

      c->get_phe      = (Sakura_phe_func)sakura_sed_get_phe;
      c->add          = (Sakura_add_func)sakura_sed_add_sediment;
      c->remove       = (Sakura_add_func)sakura_sed_remove_sediment;
      c->get_depth    = (Sakura_get_func)sakura_sed_get_depth;

      c->get_phe_data = p;
      c->add_data     = p;
      c->remove_data  = p;
      c->depth_data   = p;

      width         = sakura_set_width_from_cube       ( p , i_start );
      bathy_data    = sakura_set_bathy_data_from_cube  ( p , width , i_start , dx );
      sediment_data = sakura_set_sediment_data_from_env( );

      eh_require( width         );
      eh_require( bathy_data    );
      eh_require( sediment_data );

      deposit_in_m = eh_new_2( double , sediment_data->n_grains , bathy_data->len );

      for ( t=0 ; t<total_t ; t+=dt )
      {
         daily_flood = sakura_set_flood_data( f , c->rho_river_water );

         eh_require( daily_flood );

         if ( t+dt > total_t )
            dt = total_t-t;

         daily_flood->duration  = dt;
//         daily_flood->velocity += SAKURA_VELOCITY_RANGE*g_random_double();

         ok = sakura_wrapper( bathy_data , daily_flood , sediment_data , c , deposit_in_m );

         sakura_destroy_flood_data( daily_flood );
      }

      sakura_destroy_bathy_data( bathy_data );
      eh_free_2( deposit_in_m );
   }

   return ok;
}

void
sakura_erode_sediment( Sed_cube         p          ,
                       Sakura_bathy_st* bathy_data ,
                       gssize           i_start    ,
                       double**         erosion_in_m )
{
   if ( p && erosion_in_m )
   {
      gint i,  len;
      gint n,  n_grains = sed_sediment_env_n_types();
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
sakura_deposit_sediment( Sed_cube p                  ,
                         Sakura_bathy_st* bathy_data ,
                         gssize           i_start    ,
                         double** deposit_in_m )
{
   if ( p && deposit_in_m )
   {
      gint i,  len          = bathy_data->len;
      gint n,  n_grains     = sed_sediment_env_n_types();
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
sakura_set_width_from_cube( Sed_cube p , gssize i_start )
{
   double* width = NULL;

   {
      gint   i;
      double flow_width;
      gint   len   = sed_cube_n_y(p);
      double dx    = sed_cube_y_res(p);
      double alpha = tan( SAKURA_SPREADING_ANGLE*S_RADS_PER_DEGREE );

      width = eh_dbl_array_new_set( len , SAKURA_INITIAL_WIDTH );

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
sakura_flood_from_cell( Sed_cell c , double area )
{
   Sed_hydro h = NULL;

   if ( c )
   {
      h = sed_hydro_new( sed_cell_n_types(c)-1 );

      sed_hydro_set_width   ( h , SAKURA_INITIAL_WIDTH    );
      sed_hydro_set_depth   ( h , SAKURA_INITIAL_HEIGHT   );
      sed_hydro_set_velocity( h , SAKURA_INITIAL_VELOCITY );
      sed_hydro_set_bedload ( h , 0.                                 );

      {
         gint n;
         double* f = sed_cell_copy_fraction( NULL , c );

         eh_dbl_array_mult( f , sed_hydro_size(h) , SAKURA_INITIAL_CONCENTRATION );

         for ( n=0 ; n<sed_hydro_size(h) ; n++ )
            sed_hydro_set_nth_concentration( h , n , f[n] );

         eh_free( f );
      }

      {
         double volume_of_sediment = sed_cell_size_0( c )
                                   * area
                                   * sed_cell_density( c )
                                   / SAKURA_GRAIN_DENSITY;
         gint   n_days             = volume_of_sediment
                                   / sed_hydro_suspended_flux(h)
                                   * S_DAYS_PER_SECOND;

         sed_hydro_set_duration( h , n_days );
      }

      eh_require( sed_hydro_check( h , NULL ) );
   }

   return h;
}

Sakura_bathy_st*
sakura_set_bathy_data_from_cube( Sed_cube p , double* width , gssize ind_start , double dx )
{
   Sakura_bathy_st* b = NULL;

   if ( p )
   {
      gssize      len;
      gssize*     id        = eh_id_array( ind_start , sed_cube_size(p)-1 , &len );
      double**    bathy     = eh_new_2( double , 3 , sed_cube_size(p) );
      double      basin_len;
      Eh_dbl_grid g         = sed_cube_water_depth_grid( p , id );

      eh_require( ind_start>=0               );
      eh_require( ind_start<sed_cube_size(p) );

      bathy[0] = sed_cube_y( p , id );
      bathy[1] = eh_dbl_grid_data(g)[0];
      bathy[2] = width + ind_start;

      basin_len = bathy[0][len-2] - bathy[0][0];

      b = sakura_set_bathy_data( bathy , len , dx , basin_len );

      eh_free( bathy[0] );
      eh_free( bathy    );
      eh_free( id       );
      eh_grid_destroy( g , TRUE );
   }

   return b;
}

Sakura_sediment_st*
sakura_set_sediment_data_from_env( )
{
   Sakura_sediment_st* s = eh_new( Sakura_sediment_st , 1 );
   gint n;

   s->n_grains      = sed_sediment_env_n_types();
   s->size_equiv    = sed_sediment_property( NULL , &sed_type_grain_size_in_meters );
   s->lambda        = sed_sediment_property( NULL , &sed_type_lambda_in_per_seconds );
   s->bulk_density  = sed_sediment_property( NULL , &sed_type_rho_sat );
   s->grain_density = eh_dbl_array_new_set( s->n_grains , SAKURA_GRAIN_DENSITY );
   s->u_settling    = eh_dbl_array_new_set( s->n_grains , 0.                   );
   s->reynolds_no   = eh_dbl_array_new_set( s->n_grains , 0.                   );

   for ( n=0 ; n<s->n_grains ; n++ )
   {
      s->size_equiv[n]  = sakura_get_equivalent_diameter( s->size_equiv[n] );
      s->u_settling[n]  = sakura_settling_velocity      ( s->grain_density[n]   , s->size_equiv[n] ,
                                                          sed_rho_fresh_water() , sed_mu_water() );
      s->reynolds_no[n] = sakura_reynolds_number        ( s->grain_density[n]   , s->size_equiv[n] ,
                                                          sed_rho_fresh_water() , sed_mu_water() );
   }

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
sakura_sed_get_phe( Sed_cube p , double y , Sakura_phe_st* phe_data )
{
   eh_require( p        );
   eh_require( phe_data );

   if ( p && phe_data )
   {
      double   volume   = 0.;
      gint     ind      = sed_cube_column_id( p , 0 , y );
      double*  phe      = phe_data->phe;
      double   depth    = phe_data->val / ( sed_cube_x_res(p)*sed_cube_y_res(p) );
      gint     n_grains = sed_sediment_env_n_types();
      gint     n;

      eh_require( ind>=0 );
      eh_require( sed_cube_is_in_domain( p , 0 , ind ) );

      if ( depth > 0 )
      {
         Sed_cell avg = sed_cell_new_env( );
/*
         // The grid used by sakura will be smaller than that used by sedflux.
         // As such, we reduce the erosion depth but remove from the entire width
         // of the cell in such a way that mass is conserved.
         //depth *= dx/sed_cube_y_res( p );
         depth /= sed_cube_y_res( p );
*/
         eh_upper_bound( depth , sed_cube_thickness(p,0,ind) );
   
         // Remove different grain sizes equally.  We can change this so that
         // sands are more easily eroded than clays -- or whatever we want,
         // really.
         sed_column_extract_top( sed_cube_col(p,ind) , depth , avg );

         phe = sed_cell_copy_fraction( phe , avg );

/*
         for ( n=0 , volume=0. ; n<n_grains ; n++ )
            volume += depth
                    * phe[n]
                    * sed_type_rho_sat( sed_sediment_type( NULL , n ) )
                    / SAKURA_DENSITY_OF_SEDIMENT_GRAINS;
*/

         // We want to return the amount of sediment plus water that is available
         volume = sed_cell_size(avg)*sed_cube_x_res(p)*sed_cube_y_res(p);

         sed_cell_destroy( avg );
      }
      else
         phe = eh_dbl_array_set( phe , n_grains , 0. );


   // save the volume that was actually eroded.
//   data->erode_depth = volume;
      phe_data->val = volume;
   }

   return;
}

double
sakura_sed_remove_sediment( Sed_cube p , double y , Sakura_cell_st* s )
{
   double vol_rem = 0.;

   eh_require( p );
   eh_require( s );

   if ( p && s && s->t>0 )
   {
      Sed_column col      = sed_cube_col_pos( p , 0 , y );
      gint       n_grains = sed_sediment_env_n_types();
      double*    f        = eh_new0( double , n_grains );
      Sed_cell   cell     = NULL;
      double     dz       = s->t / ( sed_cube_y_res(p) * sed_cube_x_res(p) );

      f[s->id] = 1.;
      cell     = sed_column_separate_top( col , dz , f , NULL );
      vol_rem  = sed_cell_size( cell ) * sed_cube_y_res(p) * sed_cube_x_res(p);

      sed_cell_destroy( cell );
      eh_free( f );
   }

   return vol_rem;
}

double
sakura_sed_add_sediment( Sed_cube p , double y , Sakura_cell_st* s )
{
   double vol_add = 0;

   eh_require( p );
   eh_require( s );

   if ( p && s && s->t>0 )
   {
      Sed_column col      = sed_cube_col_pos( p , 0 , y );
      gint       n_grains = sed_sediment_env_n_types();
      double*    amount   = eh_new0( double , n_grains );
//      double     dz       = s->t / sed_cube_y_res(p);
      double     dz       = s->t / ( sed_cube_y_res(p) * sed_cube_x_res(p) );
      double     depth    = sed_column_water_depth( col );

      if ( dz > depth ) dz = depth;

      amount[s->id]  = dz;

      sed_column_add_vec( col , amount );

      vol_add = dz * sed_cube_x_res(p) * sed_cube_y_res(p);

      eh_free( amount );
   }

   return vol_add;
}

double
sakura_sed_get_depth( Sed_cube p , double y )
{
   double depth = 0.;

   eh_require( p );

   if ( p )
   {
      gint ind = sed_cube_column_id( p , 0 , y );

      if ( ind>=0 ) depth = sed_cube_water_depth( p , 0 , ind );
   }

   return depth;
}

/** Calculate the equivalent grain size.

Sediment grains falling through water will settle a rates that are greater than
those predicted because of flocculation.  That is, a grain will settle at a rate predicted
for a larger grain.  This function calculates this "equivalent" grain size, given the real
grain size.

\param real_diameter The real grain size in meters.

\return The equivalent grain size in meters.
*/

double sakura_get_equivalent_diameter(double real_diameter)
{
/* double m=0.2,b=220e-6;
   double m=0.2,b=100e-6;
   double m=0.2,b=200e-6;
   return m*real_diameter+b;
*/
   double a=39.8e-3, b=.6;
   return a*pow(real_diameter,b);
}

