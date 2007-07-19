#include "eh_ndgrid.h"
#include "utils.h"

#if !defined( OLD_NDGRID )

CLASS( Eh_ndgrid )
{
   double *data;
   gssize n_dim;
   gssize *size;
   gssize *low;
   double **x;
   gssize el_size;
};

double* eh_ndgrid_x( Eh_ndgrid g , gssize dim )
{
   eh_require( dim<g->n_dim );
   return g->x[dim];
}

gssize eh_ndgrid_n( Eh_ndgrid g , gssize dim )
{
   eh_require( dim<g->n_dim );
   return g->size[dim];
}

Eh_ndgrid eh_ndgrid_malloc( gssize n_dim , gssize el_size , ... )
{
   gssize n;
   gssize n_el=1;
   Eh_ndgrid g;
   va_list args;

   NEW_OBJECT( Eh_ndgrid , g );

   g->size = eh_new( gssize  , n_dim );
   g->low  = eh_new( gssize  , n_dim );
   g->x    = eh_new( double* , n_dim );

   g->n_dim = n_dim;

   va_start( args , el_size );
   for ( n=0 ; n<n_dim ; n++ )
   {
      g->size[n] = va_arg( args , gssize );
      g->low[n]  = 0;
      g->x[n]    = eh_new( double , g->size[n] );
      n_el      *= g->size[n];
   }
   va_end( args );

//   g->data = (double*)g_malloc0( n_el*el_size );
//   g->data = (double*)eh_malloc( n_el*el_size , NULL , __FILE__ , __LINE__ );
   g->data = (double*)eh_new( gchar , n_el*el_size );

   return g;
}

void eh_free_ndgrid_data( Eh_ndgrid g )
{
   gssize n;
   if ( g )
   {
      for ( n=0 ; n<g->n_dim ; n++ )
         eh_free( g->x[n] );
      eh_free( g->x    );
      eh_free( g->low  );
      eh_free( g->size );
      eh_free( g->data );
   }
}

double eh_ndgrid_ind( Eh_ndgrid g , ... )
{
   gssize n;
   gssize id;
   gssize *sub;
   va_list args;

   sub = eh_new( gssize , g->n_dim );

   va_start( args , g );
   for ( n=0 ; n<g->n_dim ; n++ )
      sub[n] = va_arg( args , gssize );
   va_end( args );

   id = eh_ndgrid_sub_to_id( g->size , sub , g->n_dim );

   eh_free( sub );

   return g->data[id];
}

Eh_ndgrid eh_reshape_ndgrid( Eh_ndgrid g , gssize *new_size , gssize new_n_dim )
{
   gssize k_old, k_new;
   gssize n;

   for ( n=0 ; n<g->n_dim ; n++ )
   {
      k_old *= g->size[n];
      k_new *= new_size[n];
   }
   eh_require( k_old == k_new );

   if ( new_n_dim > g->n_dim )
   {
      g->x = g_renew( double* , g->x , new_n_dim );
      for ( n=0 ; n<g->n_dim ; n++ )
         g->x[n] = g_renew( double , g->x[n] , new_size[n] );
      for ( n=g->n_dim ; n<new_n_dim ; n++ )
         g->x[n] = eh_new( double , new_size[n] );
   }
   else
   {
      for ( n=0 ; n<new_n_dim ; n++ )
         g->x[n] = g_renew( double , g->x[n] , new_size[n] );
      for ( n=new_n_dim ; n<g->n_dim ; n++ )
         eh_free( g->x[n] );
      g->x = g_renew( double* , g->x , new_n_dim );
   }
   
   g->n_dim = new_n_dim;
   g->size  = g_renew( gssize , g->size  , new_n_dim );
   for ( n=0 ; n<g->n_dim ; n++ )
      g->size[n] = new_size[n];

   return g;
}

gssize eh_ndgrid_sub_to_id( gssize *size , gssize *sub , gssize n_dim  )
{
   gssize n;
   gssize *k;
   gssize id;

   k = eh_new( gssize , n_dim );
   for ( n=1,k[0]=1 ; n<n_dim ; n++ )
      k[n] = k[n-1]*size[n-1];

   for ( n=0 ; n<n_dim ; n++ )
      id += sub[n]*k[n];

   eh_free( k );

   return id;
}

gssize *eh_ndgrid_id_to_sub( gssize *size , gssize id , gssize n_dim )
{
   gssize n;
   gssize *k;
   gssize *sub;

   sub = eh_new( gssize , n_dim );
   k   = eh_new( gssize , n_dim );

   for ( n=1,k[0]=1 ; n<n_dim ; n++ )
      k[n] = k[n-1]*size[n-1];

   for ( n=n_dim-1 ; n>=0 ; n-- )
   {
      sub[n] = id/k[n];
      id     = id%k[n];
   }

   eh_free( k );

   return sub;
}

void eh_ndgrid_destroy( Eh_ndgrid g )
{
   eh_free_ndgrid_data( g );
   eh_free( g );
}

Eh_dbl_grid eh_ndgrid_to_grid( Eh_ndgrid g )
{
   Eh_dbl_grid dest;
   gssize i, n, n_x, n_y;

   // find the first non-singleton dimension
   for ( n=0 ; n<g->n_dim && g->size[n]==1 ; n++ );

   if ( n!=g->n_dim )
   {
//      n_x = g->size[n];
//      for ( n+=1,n_y=1 ; n<g->n_dim ; n++ )
//         n_y *= g->size[n];

      n_y = g->size[g->n_dim-1];
      for ( n_x=1 ; n<g->n_dim-1 ; n++ )
         n_x *= g->size[n];
   }
   else
   {
      n_x = 1;
      n_y = 1;
   }

   dest          = eh_grid_new( double , n_x , n_y );
/*
   NEW_OBJECT( Eh_dbl_grid , dest );
   dest->x       = eh_new( double , n_x );
   dest->y       = eh_new( double , n_y );
   dest->el_size = g->el_size;
   dest->low_x   = 0;
   dest->low_y   = 0;
   dest->n_x     = n_x;
   dest->n_y     = n_y;

   dest->data    = eh_new( double* , n_x );
   dest->data[0] = g->data;
   for ( i=1 ; i<n_x ; i++ )
      dest->data[i] = dest->data[i-1]+n_y;

   memcpy( dest->x , g->x[0] , sizeof(double)*g->size[0] );
*/
   {
      double** dest_data = eh_dbl_grid_data( dest );
      eh_free( dest_data[0] );
      dest_data[0] = g->data;
      for ( i=1 ; i<n_x ; i++ )
         dest_data[i] = dest_data[i-1]+n_y;
   }

   memcpy( eh_grid_x(dest) , g->x[0] , sizeof(double)*g->size[0] );

   return dest;
}

Eh_ndgrid eh_grid_to_ndgrid( Eh_grid g )
{
   Eh_ndgrid dest;

   NEW_OBJECT( Eh_ndgrid , dest );

   dest->n_dim = 2;

   dest->x    = eh_new( double* , 2 );
   dest->x[0] = eh_new( double , eh_grid_n_x(g) );
   dest->x[1] = eh_new( double , eh_grid_n_y(g) );
   memcpy( dest->x[0] , eh_grid_x(g) , sizeof(double)*eh_grid_n_x(g) );
   memcpy( dest->x[1] , eh_grid_y(g) , sizeof(double)*eh_grid_n_x(g) );

   dest->data = (double*)eh_grid_data_start(g);

   dest->size    = eh_new( gssize , 2 );
   dest->size[0] = eh_grid_n_x(g);
   dest->size[1] = eh_grid_n_y(g);
   
   return dest;
}

gssize eh_ndgrid_write( FILE *fp , Eh_ndgrid g )
{
   gssize n, n_x;
   for ( n=0,n_x=1 ; n<g->n_dim ; n++ )
      n_x *= g->size[n];
   return eh_dbl_array_write( fp , g->data , n_x );
}

#endif

