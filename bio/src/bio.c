#include <glib.h>
#include "utils.h"
#include "bio.h"

void diffuse_col( double* u , gint len , double dz , double k , double total_t );

void
bioturbate( double** col , gint n_grains , gint n_layers , double dz , double k , double total_t )
{
   eh_require( n_grains>0 );
   eh_require( dz>0       );
   eh_require( k>0        );

   if ( col && n_layers>1 )
   {
      gint n;

      for ( n=0 ; n<n_grains ; n++ )
         diffuse_col( col[n] , n_layers , dz , k , total_t );
   }
}

#include <math.h>
#include "utils.h"

void
diffuse_col( double* u , gint len , double dz , double k , double total_t )
{
   if ( u )
   {
      gint i, n;
      double*      u_new  = eh_dbl_array_dup( u , len );
      const gint   top_i  = len-1;
      const double dt_opt = .75*.5*dz*dz/k;
      const double dt     = eh_min( dt_opt , total_t );
      const double c      = dt * k / (dz*dz);
      const gint   n_t    = total_t / dt;
      const double dt_rem = fmod( total_t , dt );

      for ( n=0 ; n<n_t ; n++ )
      {
         for ( i=1 ; i<top_i ; i++ )
            u_new[i] = c * ( u[i-1] - 2.*u[i] + u[i+1] ) + u[i];

         u_new[0]     = c * ( -2.*u[0]     + 2.*u[1]       ) + u[0];
         u_new[top_i] = c * ( -2.*u[top_i] + 2.*u[top_i-1] ) + u[top_i];

         eh_dbl_array_copy( u , u_new , len );
      }

      if ( dt_rem > 0. )
      {
         for ( i=1 ; i<top_i ; i++ )
            u_new[i] = ( dt_rem / dt ) * c * ( u[i-1] - 2.*u[i] + u[i+1] ) + u[i];

         u_new[0]     = ( dt_rem / dt ) * c * ( -2.*u[0]     + 2.*u[1]       ) + u[0];
         u_new[top_i] = ( dt_rem / dt ) * c * ( -2.*u[top_i] + 2.*u[top_i-1] ) + u[top_i];

         eh_dbl_array_copy( u , u_new , len );
      }


      eh_free( u_new );
   }

   return;
}

