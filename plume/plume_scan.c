#include <glib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "plume_local.h"
#include "plume_types.h"
#include "plumeinput.h"

GQuark
plume_error_quark( void )
{
   return g_quark_from_static_string( "plume-error-quark" );
}

static Plume_param_st _p;

static Eh_key_file_entry template[] =
{
 { "Bulk density"                                    , EH_ARG_DARRAY , &_p.bulk_density    , &_p.n_grains } ,
 { "Removal rate constant"                           , EH_ARG_DARRAY , &_p.lambda          , &_p.n_grains } ,
 { "Coast normal"                                    , EH_ARG_DBL    , &_p.r_dir           } ,
 { "River angle wrt coast normal"                    , EH_ARG_DBL    , &_p.r_angle         } ,
 { "Latitude"                                        , EH_ARG_DBL    , &_p.latitude        } ,
 { "Ocean sediment concentration"                    , EH_ARG_DBL    , &_p.ocean_conc      } ,
 { "Normalized with of coastal current"              , EH_ARG_DBL    , &_p.coastal_current_width      } ,
 { "Number of grid nodes within river mouth"         , EH_ARG_INT    , &_p.river_mouth_nodes      } ,
 { "Ratio of cross- to along-shore grid spacing"     , EH_ARG_DBL    , &_p.aspect_ratio      } ,
// { "Position of left fjord wall"                     , EH_ARG_DBL    , &p.fjord_wall_left   } ,
// { "Position of right fjord wall"                    , EH_ARG_DBL    , &p.fjord_wall_right  } ,
 { "Basin width"                                     , EH_ARG_DBL    , &_p.basin_width  } ,
 { "Basin length"                                    , EH_ARG_DBL    , &_p.basin_len  } ,
 { "Cross-shore output resolution"                   , EH_ARG_DBL    , &_p.dx  } ,
 { NULL }
};

Plume_param_st*
plume_scan_parameter_file( const gchar* file , GError** error )
{
   Plume_param_st* p_new     = NULL;
   GError*         tmp_error = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   eh_require( file );

   eh_key_file_scan_from_template( file , "PLUME" , template , &tmp_error );

   if ( !tmp_error )
   {
      p_new = eh_new( Plume_param_st , 1 );

      *p_new = _p;

      plume_check_params( p_new , &tmp_error );
   }

   if ( tmp_error )
   {
      eh_free( p_new );
      p_new = NULL;
      g_propagate_error( error , tmp_error );
   }

   return p_new;
}

Plume_param_st*
plume_check_params( Plume_param_st* p , GError** error )
{
   eh_require( p );
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( p )
   {
      gchar** err_s = NULL;

      eh_check_to_s( p->latitude<= 90. , "Latitude must be between -90 and 90" , &err_s );
      eh_check_to_s( p->latitude>=-90. , "Latitude must be between -90 and 90" , &err_s );
   }

   return p;
}

gint
plume_print_data( const gchar* file , double** deposit , gint len , gint n_grains )
{
   gint n = 0;

   eh_require( deposit && *deposit );
   eh_require( len>0 )
   eh_require( n_grains>0 );

   if ( file && deposit )
   {
      gint  i, j;
      FILE* fp = eh_open_file( file , "a" );

      for ( j=0 ; j<n_grains ; j++ )
      {
         n += fprintf( fp , "%f" , deposit[0][j] );
         for ( i=1 ; i<len ; i++ )
            n += fprintf( fp , "; %f" , deposit[i][j] );
         n += fprintf( fp , "\n" );
      }

      fclose( fp );
   }

   return n;
}

double**
plume_wrapper( Sed_hydro r , Plume_param_st* p , gint* len , gint* n_grains )
{
   double** deposit = NULL;

   eh_require( p );

   if ( r )
   {
      Plume_enviro  env;
      Plume_grid    grid;
      Plume_options opt;

      { /* Set Plume_enviro structure */
         gint       n;

         *n_grains = sed_hydro_size( r );

         env.lat = p->latitude;

         env.n_grains = *n_grains;
         env.sed = eh_new( Plume_sediment , env.n_grains );
         for ( n=0 ; n<*n_grains ; n++ )
         {
            env.sed[n].rho    = p->bulk_density[n];
            env.sed[n].lambda = p->lambda[n]*S_DAYS_PER_SECOND;
         }

         env.river             = eh_new( Plume_river , 1 );
         env.river->Q          = sed_hydro_water_flux        ( r );
         env.river->u0         = sed_hydro_velocity          ( r );
         env.river->b0         = sed_hydro_width             ( r );
         env.river->d0         = sed_hydro_depth             ( r );
         env.river->Cs         = sed_hydro_copy_concentration( NULL , r );
         env.river->rdirection = p->r_dir;
         env.river->rma        = p->r_angle;

         env.ocean             = eh_new( Plume_ocean , 1 );
         env.ocean->Cw         = p->ocean_conc;
         env.ocean->vo         = p->coastal_current;
         env.ocean->vdirection = p->coastal_current_dir;
         env.ocean->cc         = p->coastal_current_width;
         env.ocean->So         = p->river_tracer;
         env.ocean->Sw         = p->ocean_tracer;
      }

      { /* Set Plume_grid structure */
         grid.ndy   =  p->river_mouth_nodes;
         grid.ndx   =  p->aspect_ratio;
         grid.ymin  = -p->basin_width*.5;
         grid.ymax  =  p->basin_width*.5;
         grid.max_len = p->basin_len;
         grid.x_len = 0;
         grid.y_len = 0;
      }

      { /* Set Plume_opt structure */
         opt.fjrd = 1;
         opt.kwf  = 0;

         opt.o1   = 1;
         opt.o2   = 0;
         opt.o3   = 0;
      }
/*
   double *Cs;        // River Concentration (kg/m^3) [0.001:100]
   double Q;          // discharge (m^3/s) [1:1e6]
   double u0;         // velocity (m/s) [0.01:10]
   double rdirection; // river mouth direction (degN) [0:360]
   double b0;         // river mouth width (m) [1.0:1e5]
   double d0;         // river depth (m) [calculated]
   double rma;        // River mouth angle (degrees), + is in plus y dir.

   double Cw;         // ocean sediment concentration (kg/m^3) [0:min(Cs)]
   double vo;         // alongshore current magnitude (m/s) [-3:3]
   double vdirection; // alongshore current direction (degN) [0:360]
   double cc;         // Coastal Current width = cc*inertial length scale [0.1:1]
   double So;         // Conservative Tracer Property, River concentration
   double Sw;         // Conservative Tracer Property, Ocean concentration
*/

      if ( FALSE )
      {
         gint n;
      for ( n=0 ; n<env.n_grains ; n++ )
         eh_watch_dbl( env.river->Cs[n] );
      eh_watch_dbl( env.river->Q );
      eh_watch_dbl( env.river->u0 );
      eh_watch_dbl( env.river->rdirection );
      eh_watch_dbl( env.river->b0 );
      eh_watch_dbl( env.river->d0 );
      eh_watch_dbl( env.river->rma );

      eh_watch_dbl( env.ocean->Cw );
      eh_watch_dbl( env.ocean->vo );
      eh_watch_dbl( env.ocean->vdirection );
      eh_watch_dbl( env.ocean->cc );
      eh_watch_dbl( env.ocean->So );
      eh_watch_dbl( env.ocean->Sw );

      for ( n=0 ; n<env.n_grains ; n++ )
      {
         eh_watch_dbl( env.sed[n].rho    );
         eh_watch_dbl( env.sed[n].lambda );
         eh_watch_dbl( env.sed[n].lambda*S_SECONDS_PER_DAY );
      }
      }
      plume( &env , &grid , &opt );

      *len = p->basin_len / p->dx;

      deposit = eh_new_2( double , *len , *n_grains );

      plumeout2( &env , &grid , p->dx , deposit , *len , *n_grains , p->basin_width );
   }

   return deposit;
}

