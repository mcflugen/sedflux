#include <glib.h>
#include <utils/utils.h>
#include "bio.h"

G_GNUC_INTERNAL void diffuse_col( double* u , gint len , double dz , double k , double total_t );

GQuark
bio_error_quark( void )
{
   return g_quark_from_static_string( "bio-error-quark" );
}

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

void
diffuse_col( double* f , gint len , double dz , double k , double total_t )
{
   if ( len>2 )
   {
      {
         Eh_num_method method = EH_NUM_IMPLICIT;
         gint n;
         double*      f_new  = eh_dbl_array_dup( f , len );
//         const double dt_opt = .9*.5*dz*dz/k;
//         const double dt     = eh_min( dt_opt , total_t );
         const double dt     = total_t;
         const double c      = dt * k / (dz*dz);
         const gint   n_t    = total_t / dt;
         const double dt_rem = fmod( total_t , dt );

         for ( n=0 ; n<n_t ; n++ )
           eh_dbl_array_diffuse ( f , len , c , method );

         if  ( dt_rem > 0. )
           eh_dbl_array_diffuse ( f , len , c * ( dt_rem / dt ) , method );

         eh_free( f_new );
      }
   }

   return;
}

double**
bio_diffuse_layers( double* t , gint n_layers , double dz , double k , double duration )
{
   double** u_out = NULL;

   if ( n_layers>2 )
   {
      gint    i;
      double* u_copy;

      u_out = eh_new( double* , n_layers+1 );

      for ( i=0 ; i<n_layers ; i++ )
      {
         u_copy    = eh_new0( double , n_layers );
         u_copy[i] = t[i];

         diffuse_col( u_copy , n_layers , dz , k , duration );

         u_out[i] = u_copy;
      }
      u_out[n_layers] = NULL;
   }

   return u_out;
}

double**
bio_conveyor_layers( double* t , gint n_layers , double dz , double r , double duration )
{
   double** u_out = NULL;

   eh_require( dz       > 0 );
   eh_require( n_layers > 0 );

   if ( r > 0 && duration > 0 )
   {
      const double h  = eh_dbl_array_sum(t,n_layers);
      const double dh = r*duration;

      if ( dh >= h )
      {
         u_out    = eh_new( double* , 2        );
         u_out[0] = eh_dbl_array_dup( t , n_layers );
         u_out[1] = NULL;
      }
      else if ( dh > 0 )
      {
         double* u_avg = eh_new0( double , n_layers );
         gint    i, i_shift, j;
         gint    new_len;
         double  tot;

         for ( i=0,tot=t[0] ; tot<=dh && i<n_layers ; i++,tot+=t[i] ) u_avg[i] = t[i];
         u_avg[i] = t[i] - (tot-dh);

         i_shift  = i;

         eh_require( tot     >= dh       );
         eh_require( i_shift <  n_layers );

         new_len = n_layers-i_shift+1;

         u_out = eh_new( double* , new_len+1 );

         for ( i=0,j=i_shift ; i<new_len-1 ; i++,j++ )
         {
            u_out[i]    = eh_new0( double , n_layers );
            u_out[i][j] = t[j];
         }
         u_out[0][i_shift] = tot-dh;

         u_out[new_len-1] = u_avg;
         u_out[new_len]   = NULL;
      }
      else
      {
         gint i;
         u_out = eh_new( double* , n_layers+1 );
         for ( i=0 ; i<n_layers ; i++ )
         {
            u_out[i] = eh_new0( double , n_layers );
            u_out[i][i] = t[i];
         }
         u_out[n_layers] = NULL;
      }
   }

   return u_out;
}

void
bio_conveyor( double* u , gint len , double r , double total_t , double** u_out , gint** i_out , gint* len_out )
{
   eh_require( u );

   if ( u )
   {
      const double h     = eh_dbl_array_sum( u , len );
      const double dh    = fmod( r*total_t , h );
      gint*        i_in  = eh_new( gint   , len   );
      gint         i;
      gint         i_shift;
      double       z;

      *u_out   = eh_new( double , len+1 );
      *i_out   = eh_new( gint   , len+1 );
      *len_out = len+1;

      for ( i=0 ; i<len ; i++ ) i_in[i] = i;

      /* Find index to elevation dh */
      for ( i_shift=0 ; i_shift<len && z<dh ; z+=u[i_shift],i_shift++ );

      /* Shift the layers */
      g_memmove( *u_out               , u+i_shift    , sizeof(double)*(len-i_shift) );
      g_memmove( *u_out+(len-i_shift) , u            , sizeof(double)*(    i_shift) );
      g_memmove( i_out                , i_in+i_shift , sizeof(gint)  *(len-i_shift) );
      g_memmove( i_out+(len-i_shift)  , i_in         , sizeof(gint)  *(    i_shift) );

      /* Split the bottom cell.  Don't check if dh lies exactly at a layer boundary */
      (*u_out)[0]   = z - dh;
      (*u_out)[len] = u[i_shift] - (z-dh);

      eh_free( i_in );
   }

   return;
}
/*
void
bio_conveyor( double* u , gint len , double dz , double r , double total_t )
{
   if ( u )
   {
      gint i, n;
      double*      u_new = eh_dbl_array_dup(u,len);
      const gint   top_i = len-1;
      const gint   n_t   = 100;
      const double c     = (r*total_t)*S_DAYS_PER_SECOND / ( dz*n_t );

      {
         const double dh  = r*total_t;
         const double h   = dz*len;

         dh = fmod( dh , h );
      }

      for ( n=0 ; n<n_t ; n++ )
      {
         for ( i=1 ; i<top_i ; i++ )
            u_new[i] = (1.-c) * u[i] + c * u[i-1];
         u_new[0] = (1.-c)*u[0] + c*u[top_i];

         eh_dbl_array_copy( u , u_new , len );
      }

      eh_free( u_new );
   }
}
*/

