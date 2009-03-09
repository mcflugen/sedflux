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
 { "Normalized width of coastal current"             , EH_ARG_DBL    , &_p.coastal_current_width      } ,
 { "Velocity of coastal current"                     , EH_ARG_DBL    , &_p.coastal_current            } ,
 { "Number of grid nodes within river mouth"         , EH_ARG_INT    , &_p.river_mouth_nodes      } ,
 { "Ratio of cross- to along-shore grid spacing"     , EH_ARG_DBL    , &_p.aspect_ratio      } ,
// { "Position of left fjord wall"                     , EH_ARG_DBL    , &p.fjord_wall_left   } ,
// { "Position of right fjord wall"                    , EH_ARG_DBL    , &p.fjord_wall_right  } ,
 { "Basin width"                                     , EH_ARG_DBL    , &_p.basin_width  } ,
 { "Basin length"                                    , EH_ARG_DBL    , &_p.basin_len  } ,
 { "Cross-shore output resolution"                   , EH_ARG_DBL    , &_p.dx  } ,
 { "Along-shore output resolution"                   , EH_ARG_DBL    , &_p.dy  } ,
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
plume_print_data( const gchar* file , Eh_dbl_grid* deposit , gint len , gint n_grains )
{
   gint n = 0;

   eh_require( deposit && *deposit );
   eh_require( len>0 )
   eh_require( n_grains>0 );

   if ( file && deposit )
   {
      gint         i;
      //FILE*        fp    = eh_open_file( file , "a" );
      Eh_dbl_grid* d     = NULL;
      GError*      error = NULL;
      gchar*       file_s;

      eh_message( "************************************"              );
      eh_message( "Number of grain sizes: %d" , n_grains              );
      eh_message( "Number of rows       : %d" , eh_grid_n_x(*deposit) );
      eh_message( "Number of cols       : %d" , eh_grid_n_y(*deposit) );
      eh_message( "Units                : %s" , "m/day"               );
      eh_message( "************************************" );

      for ( d=deposit,i=0 ; *d && !error ; d++,i++ )
      {
         file_s = g_strdup_printf( "%s-%d" , file , i );
         //n += eh_dlm_print_dbl_grid( file_s , "," , *d , &error );
         n += eh_dlm_print_dbl_grid( NULL , "," , *d , &error );
         eh_free( file_s );
      }

      //fclose( fp );
   }

   return n;
}

Eh_dbl_grid*
plume_wrapper( Sed_hydro r , Plume_param_st* p , gint* len , gint* n_grains )
{
   Eh_dbl_grid* g = NULL;

   eh_require( p );

   if ( r )
   {
      Plume_enviro  env;
      Plume_grid    grid;
      Plume_options opt;
      gint          len_x, len_y;

      eh_message( "Set plume environment" );
      { /* Set Plume_enviro structure */
         gint       n;

         *n_grains = sed_hydro_size( r );

         env.lat = p->latitude;

         env.n_grains = *n_grains;
         env.sed = eh_new( Plume_sediment , env.n_grains );
         for ( n=0 ; n<*n_grains ; n++ )
         { /* Set grain-size specific parameters */
            env.sed[n].rho    = p->bulk_density[n];
            env.sed[n].lambda = p->lambda[n]*S_DAYS_PER_SECOND;
         }

         env.river             = eh_new( Plume_river , 1 );
         env.river->Q          = sed_hydro_water_flux        ( r );
         env.river->u0         = sed_hydro_velocity          ( r );
         env.river->b0         = sed_hydro_width             ( r );
         env.river->d0         = sed_hydro_depth             ( r );
         env.river->Cs         = sed_hydro_copy_concentration( NULL , r );
         env.river->rdirection = p->r_dir   * S_RADS_PER_DEGREE;
         env.river->rma        = p->r_angle;

         env.ocean             = eh_new( Plume_ocean , 1 );
         env.ocean->Cw         = p->ocean_conc;
         env.ocean->vo         = p->coastal_current;
         env.ocean->vdirection = p->coastal_current_dir;
         env.ocean->cc         = p->coastal_current_width;
         env.ocean->So         = p->river_tracer;
         env.ocean->Sw         = p->ocean_tracer;
      }

      eh_message( "Set plume grid" );
      { /* Set Plume_grid structure */
         grid.ndy   =  p->river_mouth_nodes;
         grid.ndx   =  p->aspect_ratio;
         grid.ymin  = -p->basin_width*.5;
         grid.ymax  =  p->basin_width*.5;
         grid.max_len = p->basin_len;
         grid.x_len = 0;
         grid.y_len = 0;
      }

      eh_message( "Set plume options" );
      { /* Set Plume_opt structure */
         opt.fjrd = (p->n_dim==1)?1:0;
         opt.kwf  = 0;

         opt.o1   = 1;
         opt.o2   = 0;
         opt.o3   = 0;
      }

      eh_message( "Run the plume" );
      plume( &env , &grid , &opt );

      *len = p->basin_len / p->dx * 2;
      len_x = p->basin_len / p->dx;
      len_y = p->basin_len / p->dy;

      //g = plume_grid_to_dbl_grid( &grid , *n_grains );

      g = plume_dbl_grid_new( *n_grains , len_x , len_y , p->dx , p->dy );

      plume_output_3( &env , &grid , g , p->rotate );

   }

   return g;
}

Eh_dbl_grid*
plume_dbl_grid_new( gint n_grains , gint n_x , gint n_y , double dx , double dy )
{
   Eh_dbl_grid* g = NULL;

   if ( n_grains>0 )
   {
      gint n;

      g = eh_new( Eh_dbl_grid , n_grains+1 );
      g[n_grains] = NULL;

      for ( n=0 ; n<n_grains ; n++ )
      {
         g[n] = eh_grid_new( double , n_x*2 , n_y*2 );

         eh_grid_set_x_lin( g[n] , -n_x*dx + dx*.5 , dx );
         eh_grid_set_y_lin( g[n] , -n_y*dy + dy*.5 , dy );
      }
      
   }

   return g;
}

double*
plume_river_volume( Plume_river r , Plume_sediment* s , const gint n_grains )
{
   double* v = NULL;

   {
      gint n;

      v = eh_new(double,n_grains);

      for ( n=0 ; n<n_grains ; n++ )
         v[n] = r.Cs[n]*r.Q*S_SECONDS_PER_DAY/s[n].rho;
   }

   return v;
}

Eh_dbl_grid*
plume_output_3( Plume_enviro *env , Plume_grid *grid , Eh_dbl_grid *dest , gboolean rotate )
{
   //Eh_dbl_grid* g = NULL;

   eh_require(  dest );
   eh_require( *dest );
   eh_require(   env );
   eh_require(  grid );

   if ( dest )
   {
      const gint   n_grains = env->n_grains;
      Eh_dbl_grid* g        = plume_grid_to_dbl_grid( grid , n_grains );
      //g        = plume_grid_to_dbl_grid( grid , n_grains );

      eh_message( "Mass balance" );
      { /* Ensure mass balance */
         Plume_river     riv = *(env->river);
         Plume_sediment* sed = env->sed;
         double*         v   = plume_river_volume( riv , sed , n_grains );

         plume_dbl_grid_scale ( g , v );

         eh_free( v );
      }

      eh_message( "Rebin" );
      plume_dbl_grid_rebin ( g , dest );

      eh_message( "Set land" );
      plume_dbl_grid_set_land( dest , -1 );

      eh_message( "Rotate plume" );
      if ( rotate )
      { /* Rotate the plume */
         Plume_river riv   = *(env->river);
         gint        i     = eh_grid_n_x(dest[0])/2;
         gint        j     = eh_grid_n_y(dest[0])/2;
         double      alpha = eh_reduce_angle( riv.rdirection ) - M_PI_2;

         eh_message( "Rotating %f degrees" , (alpha+M_PI_2) * S_DEGREES_PER_RAD );

         plume_dbl_grid_rotate( dest , i , j , alpha );
      }

      eh_message( "Destroy grid" );
      { /* Destroy the temporary grid */
         Eh_dbl_grid* p;

         for ( p=g ; *p ; p++ )
            eh_grid_destroy( *p , TRUE );

         eh_free( g );
      }
   }

   return dest;
}

Eh_dbl_grid*
plume_dbl_grid_set_land( Eh_dbl_grid* grid , double val )
{
   eh_require( grid );
   if ( grid )
   {
      gint         i, j;
      const gint   n_x = eh_grid_n_x(*grid);
      const gint   n_y = eh_grid_n_y(*grid)/2;
      Eh_dbl_grid* g;
      double**     d;

      for ( g=grid ; *g ; g++ )
      {
         d = eh_dbl_grid_data( *g );
         for ( i=0 ; i<n_x ; i++ )
            for ( j=0 ; j<n_y ; j++ )
               d[i][j] = val;
      }
   }
   return grid;
}

Eh_dbl_grid*
plume_dbl_grid_rebin( Eh_dbl_grid* grid , Eh_dbl_grid* dest )
{
   eh_require( grid );

   if ( grid )
   {
      Eh_dbl_grid* g;
      gint         n;

      eh_require( g_strv_length(grid)==g_strv_length(dest) );

      for ( g=grid,n=0 ; *g ; g++,n++ )
         eh_dbl_grid_rebin_bad_val( *g , dest[n] , 0. );
   }

   return grid;
}


Eh_dbl_grid*
plume_dbl_grid_rotate( Eh_dbl_grid* grid , gint i , gint j , double alpha )
{
   eh_require( grid );

   if ( grid )
   {
      Eh_dbl_grid* g;
      double       mass_lost = 0;
      double       total     = 0;

      for ( g=grid ; *g ; g++ )
         eh_require( eh_grid_is_in_domain(*g,i,j) );

      for ( g=grid ; *g ; g++ )
      {
         eh_dbl_grid_rotate( *g , alpha , i , j , &mass_lost );
         total += mass_lost;
      }
   }

   return grid;
}

Eh_dbl_grid*
plume_dbl_grid_scale( Eh_dbl_grid* grid , double* vol_in )
{
   eh_require(  grid );
   eh_require( *grid );

   if ( grid && *grid )
   {
      gint         n;
      Eh_dbl_grid* g;
      double       area = (eh_grid_x(*grid)[1] - eh_grid_x(*grid)[0])
                        * (eh_grid_y(*grid)[1] - eh_grid_y(*grid)[0]);
      double       vol;

      for ( g=grid,n=0 ; *g ; g++,n++ )
      {
         vol = eh_dbl_grid_sum( *g ) * area;

         if ( vol>0 && !eh_compare_dbl( vol_in[n] , vol , 1e-5 ) )
             eh_dbl_grid_scalar_mult( *g , vol_in[n]/vol );
         else if ( vol<0 )
            eh_require_not_reached();
      }
   }

   return grid;
}

Eh_dbl_grid*
plume_dbl_grid_trim( Eh_dbl_grid* grid , double val )
{
   Eh_dbl_grid* t = NULL;

   eh_require( grid );

   if ( grid )
   {
      Eh_dbl_grid* g;
      gint*        box     = NULL;
      gint         lim[4]  = { G_MAXINT , G_MAXINT , 0 , 0 };
      gint         top_row = G_MININT;
      gint         top_col = G_MININT;
      gint         row, col;

      for ( g=grid ; *g ; g++ )
      {
         box = eh_dbl_grid_crop_box_gt( *g , val );

         row = box[0]+box[2];
         col = box[1]+box[3];

         if ( row>top_row ) top_row = row;
         if ( col>top_col ) top_col = col;

         if ( box[0]<lim[0] ) lim[0] = box[0];
         if ( box[1]<lim[1] ) lim[1] = box[1];

         eh_free( box );
      }

      lim[2] = top_row - lim[0] + 1;
      lim[3] = top_col - lim[1] + 1;

      for ( g=grid ; *g ; g++ )
         t = eh_strv_append( &t , eh_grid_sub( *g , lim[0] , lim[1] , lim[2] , lim[3] ) );

      eh_require( g_strv_length(grid)==g_strv_length(t) ); 
   }

   return t;
}

Eh_dbl_grid*
plume_grid_to_dbl_grid( Plume_grid* g , const gint n_grains )
{
   Eh_dbl_grid* d_grid = NULL;

   eh_require( g );

   if ( g )
   {
      gint       i, j, n;
      const gint len_x = g->ly;
      const gint len_y = g->lx;
      double**   data;

      d_grid = eh_new( Eh_dbl_grid , n_grains+1 );
      d_grid[n_grains] = NULL;

      for ( n=0 ; n<n_grains ; n++ )
      {
         d_grid[n] = eh_grid_new( double , len_x , len_y );

         eh_require( d_grid[n] );

         memcpy( eh_grid_x(d_grid[n]) , g->yval , len_x*sizeof(double) );
         memcpy( eh_grid_y(d_grid[n]) , g->xval , len_y*sizeof(double) );

         data = eh_dbl_grid_data( d_grid[n] );

         for ( i=0 ; i<len_y ; i++ )
            for ( j=0 ; j<len_x ; j++ )
               data[j][i] = g->deps[i][j][n];
      }
   }

   return d_grid;
}

