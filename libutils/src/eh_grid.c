#include "eh_grid.h"
#include "utils.h"

#if !defined( OLD_NDGRID )

CLASS ( Eh_grid )
{
   void **data;
   double *x;
   double *y;
   gssize low_x;
   gssize low_y;
   gssize n_x;
   gssize n_y;
   gssize el_size;
};

//DERIVED_CLASS( Eh_grid , Eh_dbl_grid );
//DERIVED_CLASS( Eh_grid , Eh_int_grid );

Eh_ind_2 eh_ind_2_create( int i , int j )
{
   Eh_ind_2 ind;
   ind.i = i;
   ind.j = j;
   return ind;
}

gboolean eh_ind_2_cmp( Eh_ind_2 a , Eh_ind_2 b )
{
   return a.i==b.i && a.j==b.j;
}

Eh_ind_2 *eh_ind_2_dup( Eh_ind_2 *src , Eh_ind_2 *dest )
{
   if ( !dest )
      dest = eh_new( Eh_ind_2 , 1 );
   dest->i = src->i;
   dest->j = src->j;
   return dest;
}

gssize eh_grid_n_x( Eh_grid g )
{
   return g->n_x;
}

gssize eh_grid_n_y( Eh_grid g )
{
   return g->n_y;
}

gssize eh_grid_n_el( Eh_grid g )
{
   return g->n_x*g->n_y;
}

gssize eh_grid_el_size( Eh_grid g )
{
   return g->el_size;
}

gssize eh_grid_low_x( Eh_grid g )
{
   return g->low_x;
}

gssize eh_grid_low_y( Eh_grid g )
{
   return g->low_y;
}

double* eh_grid_x( Eh_grid g )
{
   return g->x;
}

double* eh_grid_y( Eh_grid g )
{
   return g->y;
}

void* eh_grid_row( Eh_grid g , gssize row )
{
   return g->data[row];
}

void** eh_grid_data( Eh_grid g )
{
   return g->data;
}

double** eh_dbl_grid_data( Eh_grid g )
{
   return (double**)(g->data);
}

void* eh_grid_data_start( Eh_grid g )
{
   return (gchar*)(g->data[g->low_x]) + g->low_y*g->el_size;
}

Eh_grid eh_grid_set_data( Eh_grid g , void** new_data )
{
   g->data = new_data;
   return g;
}

Eh_grid eh_grid_set_x_lin( Eh_grid g , double x_0 , double dx )
{
   eh_require( g );

   if ( g->x )
   {
      eh_dbl_array_linspace( g->x , g->n_x , x_0 , dx );
   }
   return g;
}

Eh_grid eh_grid_set_y_lin( Eh_grid g , double y_0 , double dy )
{
   eh_require( g );

   if ( g->y )
   {
      eh_dbl_array_linspace( g->y , g->n_y , y_0 , dy );
   }
   return g;
}

Eh_grid eh_grid_malloc( gssize n_x , gssize n_y , gssize size )
{
   Eh_grid g;

   NEW_OBJECT( Eh_grid , g );
   
   eh_return_val_if_fail( n_x>=0 , NULL );
   eh_return_val_if_fail( n_y>=0 , NULL );
   eh_return_val_if_fail( size>0 , NULL );

   g->data    = NULL;
   g->x       = NULL;
   g->y       = NULL;
   g->n_x     = 0;
   g->n_y     = 0;
   g->low_x   = 0;
   g->low_y   = 0;
   g->el_size = size;

   eh_grid_resize( g , n_x , n_y );

   return g;
}

Eh_grid eh_grid_malloc_uniform( gssize n_x , gssize n_y , gssize size , double dx , double dy )
{
   Eh_grid g = eh_grid_malloc( n_x , n_y , size );

   eh_grid_set_y_lin( g , 0 , dy );
   eh_grid_set_x_lin( g , 0 , dx );

   return g;
}

Eh_grid eh_grid_resize( Eh_grid g , gssize n_x , gssize n_y )
{
   if ( g )
   {
      gssize i;

      if ( n_x==0 || n_y==0 )
      {
         if ( g->data )
            eh_free( g->data[0] );
         eh_free( g->data );

         g->data = NULL;
      }
      else
      {
         if ( g->data )
         {
            g->data    = eh_renew( void* , g->data , n_x );
//            g->data[0] = eh_realloc( g->data[0] , n_x*n_y*g->el_size , __FILE__ , __LINE__ );
            g->data[0] = eh_renew( gchar , g->data[0] , n_x*n_y*g->el_size );
         }
         else
         {
            g->data    = eh_new( void* , n_x );
//            g->data[0] = eh_malloc( n_x*n_y*g->el_size , NULL , __FILE__ , __LINE__ );
            g->data[0] = eh_new( gchar , n_x*n_y*g->el_size );
         }
         for ( i=1 ; i<n_x ; i++ )
            g->data[i] = (gint8*)(g->data[i-1]) + n_y*g->el_size;
      }

      if ( n_x==0 )
         eh_free( g->x );
      else
         g->x = eh_renew( double , g->x , n_x );

      if ( n_y==0 )
         eh_free( g->y );
      else
         g->y = eh_renew( double , g->y , n_y );

      g->n_x = n_x;
      g->n_y = n_y;
   }
   return g;
}

Eh_grid eh_grid_add_row( Eh_grid g , void* new_row )
{
   eh_require( g );
   eh_grid_resize( g , g->n_x+1 , g->n_y );
   if ( new_row )
      g_memmove( g->data[g->n_x-1] , new_row , g->n_y*g->el_size );
   return g;
}

Eh_grid eh_grid_add_column( Eh_grid g , void* new_column )
{
   eh_grid_resize( g , g->n_x , g->n_y+1 );
   if ( new_column )
   {
      gssize i;
      gssize offset = (g->n_y-1)*g->el_size;
      for ( i=0 ; i<g->n_x ; i++ )
         g_memmove( (gint8*)(g->data[i])+offset , new_column , g->el_size );
   }
   return g;
}

void eh_grid_free_data( Eh_grid g , gboolean free_data )
{
   if ( g )
   {
      if ( free_data && g->data )
      {
         eh_grid_reindex( g , 0 , 0 );
         eh_free( g->data[0] );
      }
      eh_free( g->data    );
      eh_free( g->x       );
      eh_free( g->y       );
   }
}

Eh_grid eh_grid_destroy( Eh_grid g , gboolean free_data )
{
   if ( g )
   {
      eh_grid_free_data( g , free_data );
      eh_free( g );
   }
   return NULL;
}

void eh_grid_dump( FILE *fp , Eh_grid g )
{
   fwrite( &(g->n_x)     , sizeof(gssize) , 1             , fp );
   fwrite( &(g->n_y)     , sizeof(gssize) , 1             , fp );
   fwrite( &(g->el_size) , sizeof(gssize) , 1             , fp );
   fwrite( &(g->low_x)   , sizeof(gssize) , 1             , fp );
   fwrite( &(g->low_y)   , sizeof(gssize) , 1             , fp );
   fwrite( g->x          , sizeof(double) , g->n_x        , fp );
   fwrite( g->y          , sizeof(double) , g->n_y        , fp );
   fwrite( g->data[0]    , g->el_size     , g->n_x*g->n_y , fp );
}

Eh_grid eh_grid_load( FILE *fp )
{
   gssize n_x, n_y, el_size;
   gssize low_x, low_y;
   Eh_grid g;

   fread( &n_x       , sizeof(gssize) , 1       , fp );
   fread( &n_y       , sizeof(gssize) , 1       , fp );
   fread( &el_size   , sizeof(gssize) , 1       , fp );

   g = eh_grid_malloc( n_x , n_y , el_size );

   fread( &low_x     , sizeof(gssize) , 1       , fp );
   fread( &low_y     , sizeof(gssize) , 1       , fp );

   fread( g->x       , el_size        , n_x     , fp );
   fread( g->y       , el_size        , n_y     , fp );
   fread( g->data[0] , el_size        , n_x*n_y , fp );

   eh_grid_reindex( g , low_x , low_y );

   return g;
}

gboolean eh_grid_cmp_data( Eh_grid g_1 , Eh_grid g_2 )
{
   gboolean is_same = FALSE;

   if ( !eh_grid_is_same_size( g_1 , g_2 ) )
      is_same = FALSE;
   else
   {
      gssize n_bytes = g_1->n_x*g_1->n_y*g_1->el_size;
      is_same = (memcmp( g_1->data[0] , g_2->data[0] , n_bytes )==0)?TRUE:FALSE;
   }

   return is_same;
}

gboolean eh_grid_cmp_x_data( Eh_grid g_1 , Eh_grid g_2 )
{
   gboolean is_same = FALSE;

   if ( g_1->n_x != g_2->n_x )
      is_same = FALSE;
   else
      is_same = (memcmp( g_1->x , g_2->x , g_1->n_x )==0)?TRUE:FALSE;

   return is_same;
}

gboolean eh_grid_cmp_y_data( Eh_grid g_1 , Eh_grid g_2 )
{
   gboolean is_same = FALSE;

   if ( g_1->n_y != g_2->n_y )
      is_same = FALSE;
   else
      is_same = (memcmp( g_1->y , g_2->y , g_1->n_y )==0)?TRUE:FALSE;

   return is_same;
}

gboolean eh_grid_cmp( Eh_grid g_1 , Eh_grid g_2 )
{
   gboolean is_same = FALSE;

   if ( !eh_grid_is_same_size( g_1 , g_2 ) )
      is_same = FALSE;
   else
   {
      is_same =    eh_grid_cmp_data( g_1 , g_2 )
                && eh_grid_cmp_x_data( g_1 , g_2 )
                && eh_grid_cmp_y_data( g_1 , g_2 );
   }

   return is_same;
}

gboolean eh_dbl_grid_cmp( Eh_dbl_grid g_1 , Eh_dbl_grid g_2 , double eps )
{
   gboolean is_same;

   if ( !eh_grid_is_same_size( g_1 , g_2 ) )
      is_same = FALSE;
   else if ( eps<=0 )
      is_same = eh_grid_cmp( g_1 , g_2 );
   else
   {
      gssize i, n_elem = eh_grid_n_el(g_1);
      double* data_1 = eh_grid_data_start( g_1 );
      double* data_2 = eh_grid_data_start( g_2 );
      for ( i=0,is_same=TRUE ; i<n_elem && is_same ; i++ )
         if ( fabs( data_1[i] - data_2[i] ) > eps )
            is_same = FALSE;
   }
   return is_same;
}

Eh_grid eh_grid_dup( Eh_grid g )
{
   Eh_grid new_grid = eh_grid_malloc( g->n_x , g->n_y , g->el_size );
   eh_grid_copy( new_grid , g );

   return new_grid;
}

Eh_grid eh_grid_copy( Eh_grid dest , Eh_grid src )
{
   gssize low_x = src->low_x;
   gssize low_y = src->low_y;

   eh_require( src->n_x==dest->n_x         );
   eh_require( src->n_y==dest->n_y         );
   eh_require( src->el_size==dest->el_size );

   eh_grid_reindex( src , 0 , 0 );

   memcpy( dest->data[0] , src->data[0] , src->n_x*src->n_y*src->el_size );
   memcpy( dest->x       , src->x       , src->n_x*sizeof(double)        );
   memcpy( dest->y       , src->y       , src->n_y*sizeof(double)        );

   eh_grid_reindex( src  , low_x , low_y );
   eh_grid_reindex( dest , low_x , low_y );

   return dest;
}

Eh_grid eh_grid_copy_data( Eh_grid dest , Eh_grid src )
{
   gssize low_x = src->low_x;
   gssize low_y = src->low_y;

   eh_require( src->n_x==dest->n_x         );
   eh_require( src->n_y==dest->n_y         );
   eh_require( src->el_size==dest->el_size );

   eh_grid_reindex( src , 0 , 0 );

   memcpy( dest->data[0] , src->data[0] , src->n_x*src->n_y*src->el_size );

   eh_grid_reindex( src  , low_x , low_y );
   eh_grid_reindex( dest , low_x , low_y );

   return dest;
}

Eh_grid eh_grid_reindex( Eh_grid g , gssize low_x , gssize low_y )
{
   gssize i;
   gssize change_low_x = low_x - g->low_x;
   gssize change_low_y = low_y - g->low_y;

   if ( change_low_x == 0 && change_low_y == 0 )
      return g;

   for ( i=g->low_x ; i<g->n_x+g->low_x ; i++ )
      g->data[i] = (gint8*)(g->data[i]) - change_low_y*g->el_size;
   g->data -= change_low_x;

   g->x -= change_low_x;
   g->y -= change_low_y;

   g->low_x = low_x;
   g->low_y = low_y;

   return g;
}

gboolean eh_grid_is_in_domain( Eh_grid g , gssize i , gssize j )
{
   return i>=g->low_x && j>=g->low_y && i<g->n_x+g->low_x && j<g->n_y+g->low_y;
}

gboolean eh_grid_is_same_size( Eh_grid g_1 , Eh_grid g_2 )
{
   return    g_1->n_x     == g_2->n_x
          && g_1->n_y     == g_2->n_y
          && g_1->el_size == g_2->el_size;
}

Eh_grid_id eh_grid_sub_to_id( gssize n_j , gssize i , gssize j )
{
   return i*n_j + j;
}

Eh_ind_2 eh_grid_id_to_sub( gssize n_i , Eh_grid_id id )
{
   Eh_ind_2 sub;

   sub.i = id/n_i;
   sub.j = id%n_i;

   return sub;
}

void eh_dbl_grid_set_val( Eh_dbl_grid g , gssize i , gssize j , double val )
{
   ((double*)(g->data[i]))[j] = val;
}

void eh_int_grid_set_val( Eh_dbl_grid g , gssize i , gssize j , int val )
{
   ((int*)(g->data[i]))[j] = val;
}

double eh_dbl_grid_val( Eh_dbl_grid g , gssize i , gssize j )
{
   return ((double*)(g->data[i]))[j];
}

int eh_int_grid_val( Eh_int_grid g , gssize i , gssize j )
{
   return ((int*)(g->data[i]))[j];
}

void* eh_grid_loc( Eh_grid g , gssize i , gssize j )
{
   return (gchar*)(g->data[i])+j*g->el_size;
}

int eh_dbl_grid_write( FILE *fp , Eh_dbl_grid g )
{
   size_t s=0;
   int n_i     = g->n_x*g->n_y;
   int el_size = g->el_size;
   int n, i, i_0;
   int one = 1, size;
   double this_val;
   double* data = eh_grid_data_start(g);

   for ( i_0=0 ; i_0<n_i ; i_0+=n )
   {
//      if ( i_0==n_i-1 || g->data[0][i_0] == g->data[0][i_0+1] )
      if ( i_0==n_i-1 || data[i_0] == data[i_0+1] )
      {
         this_val = data[i_0];

//         for ( i=i_0,n=0 ; i<n_i && g->data[0][i]==this_val ; i++,n++ );
         for ( i=i_0,n=0 ; i<n_i && data[i]==this_val ; i++,n++ );

         s += fwrite( &el_size  , sizeof(int) , 1 , fp )*sizeof(int);
         s += fwrite( &n        , sizeof(int) , 1 , fp )*sizeof(int);
         s += fwrite( &this_val , el_size     , 1 , fp )*el_size;
      }
      else
      {
//         for ( i=i_0+1,n=1 ; i<n_i && g->data[0][i-1]!=g->data[0][i] ; i++,n++ );
         for ( i=i_0+1,n=1 ; i<n_i && data[i-1]!=data[i] ; i++,n++ );

         if ( i<n_i )
            n--;

         size = n*el_size;

         s += fwrite( &size     , sizeof(int) , 1 , fp )*sizeof(int);
         s += fwrite( &one      , sizeof(int) , 1 , fp )*sizeof(int);
         s += fwrite( data+i_0  , size        , 1 , fp )*size;
//         s += fwrite( &(g->data[0][i_0])  , size        , 1 , fp )*size;
      }
   }

   return s;
}

gboolean eh_grid_is_compatible( Eh_grid g_1 , Eh_grid g_2 )
{
   gboolean ans=FALSE;

   if (    g_1
        && g_2
        && g_1->n_x     == g_2->n_x
        && g_1->n_y     == g_2->n_y 
        && g_1->el_size == g_2->el_size )
      ans = TRUE;

   return ans;
}


Eh_dbl_grid eh_dbl_grid_add( Eh_dbl_grid g_1 , Eh_dbl_grid g_2 )
{
   gssize i;
   double* g_1_data = eh_grid_data_start(g_1);
   double* g_2_data = eh_grid_data_start(g_2);
   gssize n_i = g_1->n_x*g_1->n_y;

   eh_require( eh_grid_is_compatible( g_1 , g_2 ) );

   for ( i=0 ; i<n_i ; i++ )
      g_1_data[i] += g_2_data[i];

   return g_1;
}

double eh_dbl_grid_sum( Eh_dbl_grid g )
{
   return eh_dbl_grid_sum_bad_val( g , eh_nan() );
}

double eh_dbl_grid_sum_bad_val( Eh_dbl_grid g , double bad_val )
{
   double sum=0;

   eh_require( g )
   {
      gssize i;
      gssize n_i   = eh_grid_n_el(g);
      double* data = eh_grid_data_start( g );

      if ( eh_isnan( bad_val ) )
      {
         for ( i=0 ; i<n_i ; i++ )
            if ( !eh_isnan( data[i] ) )
               sum += data[i];
      }
      else
         for ( i=0 ; i<n_i ; i++ )
            if ( fabs( data[i] - bad_val )>1e-12 )
               sum += data[i];
   }

   return sum;
}

Eh_dbl_grid eh_dbl_grid_set( Eh_dbl_grid g , double val )
{
   gssize n_i = eh_grid_n_el( g );
   gssize i;
   double* data = eh_grid_data_start( g );
   for ( i=0 ; i<n_i ; i++ )
      data[i] = val;
   return g;
}

Eh_dbl_grid eh_dbl_grid_randomize( Eh_dbl_grid g )
{
   gssize n_i = eh_grid_n_el( g );
   gssize i;
   double* data = eh_grid_data_start( g );
   for ( i=0 ; i<n_i ; i++ )
      data[i] = g_random_double();
   return g;
}

void eh_dbl_grid_scalar_mult( Eh_dbl_grid g , double scalar )
{
   eh_return_if_fail( g )
   {
      gssize i;
      gssize n_i = g->n_x*g->n_y;
      double* data = eh_grid_data_start(g);
      for ( i=0 ; i<n_i ; i++ )
         data[i] *= scalar;
   }
}

void eh_dbl_grid_rotate( Eh_dbl_grid g , double angle , gssize i_0 , gssize j_0 )
{
   eh_return_if_fail( g && angle!=0 )
   {
      gssize i, j;
      gssize i_rotate, j_rotate;
      double r, alpha, new_angle;
      double d_i, d_j;
      gssize high_x    = g->n_x+g->low_x;
      gssize high_y    = g->n_y+g->low_y;
      double** data, **temp_data;
      Eh_dbl_grid temp = eh_grid_new( double , g->n_x , g->n_y );

      eh_grid_reindex( temp , g->low_x , g->low_y );

      temp_data = (double**)(temp->data);
      data      = (double**)(g->data);

      for ( i=g->low_x ; i<high_x ; i++ )
         for ( j=g->low_y ; j<high_y ; j++ )
         {
            if ( fabs(data[i][j]) > 1e-12 )
            {
               d_i       = i-i_0;
               d_j       = j-j_0;

               r         = sqrt( pow(d_i,2.) + pow(d_j,2.) );
               alpha     = atan2( d_j , d_i );
               new_angle = alpha + angle;

               i_rotate = eh_round( r*cos( new_angle ) , 1 )+i_0;
               j_rotate = eh_round( r*sin( new_angle ) , 1 )+j_0;

               if ( eh_grid_is_in_domain( temp , i_rotate , j_rotate ) )
                  temp_data[i_rotate][j_rotate] = data[i][j];
//                  temp->data[i_rotate][j_rotate] = g->data[i][j];
            }
         }

      eh_grid_copy_data( g , temp );
      eh_grid_destroy( temp , TRUE );
   }
}

Eh_dbl_grid eh_dbl_grid_reduce( Eh_dbl_grid g ,
                                gssize new_n_x , gssize new_n_y )
{
   eh_require( new_n_x<=g->n_x );
   eh_require( new_n_y<=g->n_y );

   return eh_dbl_grid_remesh( g , new_n_x , new_n_y );
}

Eh_dbl_grid eh_dbl_grid_expand( Eh_dbl_grid g ,
                                gssize new_n_x , gssize new_n_y )
{
   eh_require( new_n_x>=g->n_x );
   eh_require( new_n_y>=g->n_y );

   return eh_dbl_grid_remesh( g , new_n_x , new_n_y );
}

Eh_dbl_grid eh_dbl_grid_remesh( Eh_dbl_grid g ,
                                gssize new_n_x , gssize new_n_y )
{
   gssize i, j;
   gssize cur_n_x, cur_n_y;
   Eh_dbl_grid new_grid;
   double *x_ind, *new_x_ind;
   double *y_ind, *new_y_ind;
   double *orig_x, *orig_y;
   double dx, dy;

   eh_require( g );

   if ( new_n_x == g->n_x && new_n_y == g->n_y )
      return eh_grid_dup( g );
/*
   eh_require( g->n_x>=2   );
   eh_require( g->n_y>=2   );
   eh_require( new_n_x > 1 );
   eh_require( new_n_y > 1 );
*/
   cur_n_x = g->n_x;
   cur_n_y = g->n_y;

   if ( new_n_x == cur_n_x && new_n_y == cur_n_y )
      return eh_grid_dup( g );

   new_grid = eh_grid_new( double , new_n_x , new_n_y );
   eh_grid_reindex( new_grid , g->low_x , g->low_y );

   if ( cur_n_x==1 && new_n_x==1 )
      dx = 1;
   else
      dx = (cur_n_x-1.) / (double)(new_n_x-1.);
   if ( cur_n_y==1 && new_n_y==1 )
      dy = 1;
   else
      dy = (cur_n_y-1.) / (double)(new_n_y-1.);

   x_ind     = eh_new( double , g->n_x        );
   new_x_ind = eh_new( double , new_grid->n_x );
   y_ind     = eh_new( double , g->n_y        );
   new_y_ind = eh_new( double , new_grid->n_y );

   for ( i=0 ; i<g->n_x        ; i++ ) x_ind[i]     = i+g->low_x;
   for ( j=0 ; j<g->n_y        ; j++ ) y_ind[j]     = j+g->low_y;

   for ( i=0 ; i<new_grid->n_x ; i++ ) new_x_ind[i] = i*dx+new_grid->low_x;
   for ( j=0 ; j<new_grid->n_y ; j++ ) new_y_ind[j] = j*dy+new_grid->low_y;

   if ( new_x_ind[new_grid->n_x-1] > x_ind[g->n_x-1] )
      new_x_ind[new_grid->n_x-1] = (1.-1e-6)*x_ind[g->n_x-1];
   if ( new_y_ind[new_grid->n_y-1] > y_ind[g->n_y-1] )
      new_y_ind[new_grid->n_y-1] = (1.-1e-6)*y_ind[g->n_y-1];

   interpolate( x_ind         , x_ind       , g->n_x ,
                new_x_ind     , new_grid->x , new_grid->n_x );
   interpolate( y_ind         , y_ind       , g->n_y ,
                new_y_ind     , new_grid->y , new_grid->n_y );

   orig_x = g->x;
   orig_y = g->y;

   g->x = x_ind;
   g->y = y_ind;

   interpolate_2( g , new_grid );

   g->x = orig_x;
   g->y = orig_y;

   eh_free( x_ind     );
   eh_free( y_ind     );
   eh_free( new_x_ind );
   eh_free( new_y_ind );

   return new_grid;
}

gboolean eh_grid_path_is_same( gssize* p_1 , gssize* p_2 )
{
   gboolean is_same = TRUE;

   if ( p_1 != p_2 )
   {
      gssize len_1, len_2;

      eh_return_val_if_fail( p_1 , FALSE );
      eh_return_val_if_fail( p_2 , FALSE );

      len_1 = eh_grid_path_len( p_1 );
      len_2 = eh_grid_path_len( p_2 );

      if ( len_1==len_2 )
         is_same = (memcmp( p_1 , p_2 , sizeof(gssize)*len_1 )==0)?TRUE:FALSE;
      else
         is_same = FALSE;
   }

   return is_same;
}

gssize eh_grid_path_len( gssize* p )
{
   gssize len = 0;

   eh_return_val_if_fail( p , 0 );

   for ( len=0 ; p[len]>=0 ; len++ );

   return len;
}

Eh_grid_id* eh_dbl_grid_line_ids( Eh_dbl_grid g , gssize i_0 , gssize j_0 , gssize i_1 , gssize j_1 )
{
   gssize* path = NULL;

   if ( !(i_0==i_1 && j_0==j_1) )
   {
      gssize di = i_1-i_0;
      gssize dj = j_1-j_0;

      if ( di>0 && dj>0 )
      {
         gssize i;
         double to_corner;
         double x, y;
         double m        = dj / (double)di;
         gssize start_id = eh_grid_sub_to_id( eh_grid_n_y(g) , i_0 , j_0 );
         gssize end_id   = eh_grid_sub_to_id( eh_grid_n_y(g) , i_1 , j_1 );
         gssize max_len  = di + dj + 1;

         path = eh_new( gssize , max_len+1 );

         path[0] = start_id;
         x = i_0+.5;
         y = j_0+.5;

         for ( i=1 ; i<max_len && path[i-1]!=end_id ; i++ )
         {
            to_corner = (1.-(x-floor(x)) ) / ( 1.-(y-floor(y)) );
            if ( to_corner<m )
            {
               path[i] = path[i-1] + 1;
               x = floor(x+1);
            }
            else if ( to_corner>m )
            {
               path[i] = path[i-1] + eh_grid_n_y(g);
               y = floor(y+1);
            }
            else
            {
               path[i] = path[i-1] + eh_grid_n_y(g) + 1;
               x = floor(x+1);
               y = floor(y+1);
            }
         }

         path[i] = -1;

      }
      else if ( di<0 )
      {
         if ( dj<0 )
            path = eh_dbl_grid_line_ids( g , i_1 , j_1 , i_0 , j_0  );
         else
            path = eh_dbl_grid_line_ids( g , i_0 , j_0 , i_0-di , j_0  );
      }
      else
         path = eh_dbl_grid_line_ids( g , i_0 , j_0 , i_1 , j_0-dj  );
   }

   return path;
}
/*
gssize* eh_grid_id_transpose( gssize* id , gssize n_x , gssize n_y )
{
   eh_require( n_x>0 );
   eh_require( n_y>0 );

   eh_return_val_if_fail( id , NULL );

   {
      gssize i;
      for ( i=0 ; id[i]>=0 ; i++ )
      {
         id[i] = mod(id[i],n_y)*n_x + floor(id[i],n_y);
      }
   }

   return id;
}

gssize* eh_grid_id_add( gssize* id , )
*/

Eh_grid sed_grid_sub( Eh_grid g , int i_0 , int j_0 , int n_x , int n_y )
{
   gssize i;
   Eh_grid sub;

   if ( i_0+n_x > g->n_x )
      n_x = g->n_x-i_0;
   if ( j_0+n_y > g->n_y )
      n_y = g->n_y-j_0;

   sub = eh_grid_malloc( n_x , n_y , g->el_size );

   for ( i=0 ; i<sub->n_x ; i++ )
      memcpy( sub->data[i] ,
              (gchar*)(g->data[i_0+i])+j_0*g->el_size ,
              sub->n_y*g->el_size );
   memcpy( sub->x , g->x+i_0 , sub->n_x*sizeof(double) );
   memcpy( sub->y , g->y+j_0 , sub->n_y*sizeof(double) );

/*
   for ( i=0 ; i<sub->n_x ; i++ )
      sub->data[i] = (gint8*)(g->data[i_0])   + j_0*g->el_size;

   sub->n_x = n_x;
   sub->n_y = n_y;
   sub->low_x = 0;
   sub->low_y = 0;

   sub->data = eh_new( void* , sub->n_x );

   for ( i=0 ; i<sub->n_x ; i++ )
      sub->data[i] = (gint8*)(g->data[i_0])   + j_0*g->el_size;
//      sub->data[i] = (gint8*)(g->data[i_0+1]) + j_0*g->el_size;

   if ( g->x )
      sub->x = g->x+i_0;
   else
      sub->x = NULL;

   if ( g->y )
      sub->y = g->y+j_0;
   else
      sub->y = NULL;
*/

   return sub;
}

void eh_dbl_grid_rebin( Eh_dbl_grid source , Eh_dbl_grid dest )
{
   eh_dbl_grid_rebin_bad_val( source , dest , eh_nan() );
}

void eh_dbl_grid_rebin_bad_val( Eh_dbl_grid source , Eh_dbl_grid dest , double bad_val )
{
   gssize i,j;
   double **temp, *temp_source, *temp_dest;
   gssize src_low_x=source->low_x, dest_low_x=dest->low_x;
   gssize src_low_y=source->low_y, dest_low_y=dest->low_y;
   double** dest_data = (double**)(dest->data);

   eh_grid_reindex( source , 0 , 0 );
   eh_grid_reindex( dest   , 0 , 0 );

   temp    = eh_new( double* , source->n_x );
   temp[0] = eh_new( double  , source->n_x*dest->n_y );
   for ( i=1 ; i<source->n_x ; i++ )
      temp[i] = temp[i-1] + dest->n_y;
   temp_source = eh_new( double , source->n_x );
   temp_dest   = eh_new( double , dest->n_x   );

   for ( i=0 ; i<source->n_x ; i++ )
      eh_rebin_dbl_array_bad_val( source->y , source->data[i] , source->n_y ,
                                  dest->y   , temp[i]         , dest->n_y   ,
                                  bad_val );

//   dest_data = dest->data;

   for ( j=0 ; j<dest->n_y ; j++ )
   {
      for ( i=0 ; i<source->n_x ; i++ )
         temp_source[i] = temp[i][j];

      eh_rebin_dbl_array_bad_val( source->x , temp_source , source->n_x ,
                                  dest->x   , temp_dest   , dest->n_x   ,
                                  bad_val );

      for ( i=0 ; i<dest->n_x ; i++ )
         dest_data[i][j] = temp_dest[i];
   }

   eh_free( temp_dest   );
   eh_free( temp_source );
   eh_free( temp[0]     );
   eh_free( temp        );

   eh_grid_reindex( source , src_low_x  , src_low_y  );
   eh_grid_reindex( dest   , dest_low_x , dest_low_y );

   return;
}

void eh_grid_foreach( Eh_grid g , GFunc func , gpointer user_data )
{
   eh_return_if_fail( g && func )
   {
      gssize i;
      gssize n_i = g->n_x*g->n_y;

      for ( i=0 ; i<n_i ; i++ )
         (*func)( (gchar*)(g->data[0])+i , user_data );
   }
}

Eh_dbl_grid eh_dbl_grid_populate( Eh_dbl_grid g  ,
                                  Populate_func f ,
                                  gpointer user_data )
{
   gssize n;
   gssize population_size=100000;
   gssize low_x  = g->low_x;
   gssize low_y  = g->low_y;
   gssize high_x = g->low_x+g->n_x;
   gssize high_y = g->low_y+g->n_y;
   gssize i, j;
   double inc, x, y;

   for ( n=0 ; n<population_size ; n++ )
   {
      x = eh_get_fuzzy_dbl( low_x , high_x );
      y = eh_get_fuzzy_dbl( low_y , high_y );
      if ( (*f)( x , y , user_data ) )
      {
         i   = floor(x);
         j   = floor(y);
         inc = eh_dbl_grid_val(g,i,j) + 1;
         eh_dbl_grid_set_val( g , i , j , inc );
      }
   }

   eh_dbl_grid_scalar_mult( g , ((double)(g->n_x*g->n_y))/population_size );

   return g;
}

void eh_dbl_grid_fprintf( FILE* fp , const gchar* format , Eh_dbl_grid g )
{
   eh_require( g      );
   eh_require( fp     );
   eh_require( format );

   if ( g && fp && format )
   {
      gssize i, j;
      for ( i=0 ; i<g->n_x ; i++ )
      {
         for ( j=0 ; j<g->n_y ; j++ )
            fprintf( fp , format , ((double*)(g->data[i]))[j] );
         fprintf( fp , "\n" );
      }
   }
}

#include <string.h> // included for g_memmove

Eh_grid eh_grid_transpose( Eh_grid g )
{ 
   gssize low_x = g->low_x;
   gssize low_y = g->low_y;

   eh_grid_reindex( g , 0 , 0 );

   eh_debug( "Swap x and y" );
   {
      double* temp = eh_grid_x(g);
      g->x = g->y;
      g->y = temp;
   }

   eh_debug( "Swap x and y" );
   {
      gssize i, j;

      if ( g->n_x == g->n_y )
      {
         for ( i=0 ; i<g->n_x ; i++ )
            for ( j=i+1 ; j<g->n_y ; j++ )
               eh_memswap( (gint8*)(g->data[i])+j*g->el_size ,
                           (gint8*)(g->data[j])+i*g->el_size ,
                           g->el_size );
      }
      else
      {
         void** temp = eh_new( void* , g->n_y );

//         temp[0] = eh_malloc( g->n_y*g->n_x*g->el_size , NULL , __FILE__ , __LINE__ );
         temp[0] = eh_new( gchar , g->n_y*g->n_x*g->el_size );
         for ( i=1 ; i<g->n_y ; i++ )
            temp[i] = (gint8*)(temp[i-1]) + g->n_x*g->el_size;

         for ( i=0 ; i<g->n_x ; i++ )
            for ( j=0 ; j<g->n_y ; j++ )
               g_memmove( (gint8*)(temp[j])+i*g->el_size    ,
                          (gint8*)(g->data[i])+j*g->el_size ,
                          g->el_size);

         g->data = eh_renew( void* , g->data , g->n_y );
         for ( i=1 ; i<g->n_y ; i++ )
            g->data[i] = (gint8*)(g->data[i-1]) + g->n_x*g->el_size;

         g_memmove( g->data[0] , temp[0] , g->n_x*g->n_y*g->el_size );

         eh_free( temp[0] );
         eh_free( temp );
      }

   }

   eh_debug( "Swap dimension lengths" );
   {
      gssize temp = g->n_x;
      g->n_x = g->n_y;
      g->n_y = temp;
   }

   eh_grid_reindex( g , low_y , low_x );

   return g;
}
/*
Eh_dbl_grid eh_dbl_grid_dx( Eh_dbl_grid g )
{
   double c = .25/h;
   for ( i=0 ; i<g->n_x ; i++ )
      for ( j=0 ; j<g->n_y ; j++ )
         dx->data[i][j] = c*(   g->data[i+1][j+1]
                              - g->data[i-1][j+1]
                              + g->data[i+1][j-1]
                              - g->data[i-1][j-1] ); 
}

Eh_dbl_grid eh_dbl_grid_dxx( Eh_dbl_grid g )
{
   double c = 1./( 3*h*h );
   for ( i=0 ; i<g->n_x ; i++ )
      for ( j=0 ; j<g->n_y ; j++ )
         dx->data[i][j] = c*(     g->data[i+1][j+1]
                              - 2*g->data[i  ][j+1]
                              +   g->data[i-1][j+1]
                              +   g->data[i+1][j  ]
                              - 2*g->data[i  ][j  ]
                              +   g->data[i-1][j  ]
                              +   g->data[i+1][j-1]
                              - 2*g->data[i  ][j-1]
                              +   g->data[i-1][j-1] );
}
*/
Eh_dbl_grid eh_dbl_grid_diff( Eh_dbl_grid g , gssize n , gssize dim )
{
   Eh_dbl_grid diff = eh_grid_new( double , g->n_x , g->n_y );

   if ( dim==1 )
   {
      eh_grid_transpose( g );
      diff = eh_grid_new( double , g->n_y , g->n_x );
   }
   else
      diff = eh_grid_new( double , g->n_x , g->n_y );

   {
      gssize i;

      for ( i=0 ; i<g->n_x ; i++ )
         eh_dbl_array_diff( diff->data[i] , g->data[i] , n , g->n_y );
   }

   if ( dim==1 )
   {
      eh_grid_transpose( diff );
      eh_grid_transpose( g );
   }

   return diff;
}

#endif

