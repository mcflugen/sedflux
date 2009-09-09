#include <eh_utils.h>

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

/** Number of rows of a grid

\param   g   An Eh_grid

\return The number of rows in the grid
*/
gint
eh_grid_n_x( Eh_grid g )
{
   return g->n_x;
}

/** Number of columns of a grid

\param   g   An Eh_grid

\return The number of columns in the grid
*/
gint
eh_grid_n_y( Eh_grid g )
{
   return g->n_y;
}

/** Total number of elements in a grid

\param   g   En Eh_grid

\return The total number of elements in the grid
*/
gint
eh_grid_n_el( Eh_grid g )
{
   return g->n_x*g->n_y;
}

/** Size of each element of a grid

\param   g   An Eh_grid

\return Size of a grid's elements in bytes
*/
gssize
eh_grid_el_size( Eh_grid g )
{
   return g->el_size;
}

/** Index to the first row of a grid

\param   g   An Eh_grid

\return The index to the first row
*/
gint
eh_grid_low_x( Eh_grid g )
{
   return g->low_x;
}

/** Index to the first column of a grid

\param   g   An Eh_grid

\return The index to the first column
*/
gint
eh_grid_low_y( Eh_grid g )
{
   return g->low_y;
}

/** Index to the last row of a grid

\param   g   An Eh_grid

\return The index to the last row
*/
gint
eh_grid_top_x( Eh_grid g )
{
   return g->low_x + g->n_x - 1;
}

/** Index to the last column of a grid

\param   g   An Eh_grid

\return The index to the last column
*/
gint
eh_grid_top_y( Eh_grid g )
{
   return g->low_y + g->n_y - 1;
}

/** x-values of an Eh_grid

Pointer to an array of x-values for a grid.  The array has the
same number of elements as the grid has rows.  The indices of
the array are the same as that of the grid's rows.

\note Be careful, this is a pointer to the data used by the grid and not
a copy of the data.

\param   g   An Eh_grid

\return A pointer to the x values
*/
double*
eh_grid_x( Eh_grid g )
{
   return g->x;
}

/** x-values of an Eh_grid

Pointer to the start of an array of x-values for a grid.  The array has the
same number of elements as the grid has rows.  However, the indices of
the array start at 0.

\note Be careful, this is a pointer to the data used by the grid and not
a copy of the data.

\param   g   An Eh_grid

\return A pointer to the x values
*/
double*
eh_grid_x_start( Eh_grid g )
{
   return g->x + g->low_x;
}

/** y-values of an Eh_grid

Pointer to an array of y-values for a grid.  The array has the
same number of elements as the grid has columns.  The indices of
the array are the same as that of the grid's columns.

\note Be careful, this is a pointer to the data used by the grid and not not
a copy of the data.

\param   g   An Eh_grid

\return A pointer to the y values
*/
double*
eh_grid_y( Eh_grid g )
{
   return g->y;
}

/** y-values of an Eh_grid

Pointer to the start of an array of y-values for a grid.  The array has the
same number of elements as the grid has columns.  However, the indices of
the array start at 0.

\note Be careful, this is a pointer to the data used by the grid and not
a copy of the data.

\param   g   An Eh_grid

\return A pointer to the y values
*/
double*
eh_grid_y_start( Eh_grid g )
{
   return g->y + g->low_y;
}

/** The n-th row of a grid

\param   g   An Eh_grid
\param   n   Index of the row

\return A pointer to the zero-th index of the row
*/
void*
eh_grid_row( Eh_grid g , gssize n )
{
   return g->data[n];
}

/** The data portion of a grid

\param   g   An Eh_grid

\return A pointer to the data portion of a grid
*/
void**
eh_grid_data( Eh_grid g )
{
   return g->data;
}
 
/** The data portion of a double grid

Retrieve a 2D array of doubles that is the data portion
of an Eh_dbl_array.  The array is indexed in the same
way as the grid.

\param   g   An Eh_dbl_grid

\return A pointer to the data portion of a double grid
*/
double**
eh_dbl_grid_data( Eh_dbl_grid g )
{
   return (double**)(g->data);
}

/** Data elements of a double grid

Retrieve a pointer to an array of doubles that are the elements
of a Eh_dbl_grid.  The data are listed row by row.  Indices of
the array start at zero.

\param   g   An Eh_dbl_grid

\return Pointer to the start of a grid's data elements
*/
double*
eh_dbl_grid_data_start( Eh_dbl_grid g )
{
   return (double*)(eh_grid_data_start(g));
}

/** Data elements of a grid

Retrieve a pointer to the elements of a Eh_grid.  The data
are listed row by row.  Indices of the array start at zero.

\param   g   An Eh_dbl_grid

\return Pointer to the start of a grid's data elements
*/
void*
eh_grid_data_start( Eh_grid g )
{
   return (gchar*)(g->data[g->low_x]) + g->low_y*g->el_size;
}

/** Set the data portion of a grid

Note that the old data is freed before setting to the new value.

\param   g   A Eh_grid
\param   new_data   Location of the new data

\return The input Eh_grid
*/
Eh_grid
eh_grid_set_data( Eh_grid g , void** new_data )
{
  if (g->data)
  {
    void *data = eh_grid_data_start (g);
    void **data_ptr = g->data + g->low_x;

    eh_free (data);
    eh_free (data_ptr);
    //free (eh_grid_data_start (g));
    //free (g->data + g->low_x);
  }
  g->data = new_data;
  return g;
}

Eh_grid
eh_grid_borrow_data (Eh_grid g, void* data)
{
  if (g->data)
    eh_free (g->data[g->low_x]);
  g->data[g->low_x] = data;
  return g;
}

Eh_grid
eh_grid_set_x_lin( Eh_grid g , double x_0 , double dx )
{
   eh_require( g );

   if ( g->x )
   {
      eh_dbl_array_linspace( g->x+g->low_x , g->n_x , x_0 , dx );
   }
   return g;
}

Eh_grid
eh_grid_set_y_lin( Eh_grid g , double y_0 , double dy )
{
   eh_require( g );

   if ( g->y )
   {
      eh_dbl_array_linspace( g->y+g->low_y , g->n_y , y_0 , dy );
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

Eh_dbl_grid
eh_dbl_grid_new_set( gint n_x , gint n_y , double** d )
{
   Eh_dbl_grid g;

   NEW_OBJECT( Eh_dbl_grid , g );

   eh_return_val_if_fail( n_x>=0  , NULL );
   eh_return_val_if_fail( n_y>=0  , NULL );
   eh_return_val_if_fail( d!=NULL , NULL );

   g->data    = (void**)d;
   g->x       = eh_new( double , n_x );
   g->y       = eh_new( double , n_y );
   g->n_x     = n_x;
   g->n_y     = n_y;
   g->low_x   = 0;
   g->low_y   = 0;
   g->el_size = sizeof(double);

   return g;
}

Eh_grid
eh_grid_malloc_uniform( gssize n_x , gssize n_y , gssize size , double dx , double dy )
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

Eh_grid
eh_grid_dup( Eh_grid g )
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

Eh_grid
eh_grid_reindex( Eh_grid g , gssize low_x , gssize low_y )
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

/** Subtract one grid from another

Subtract grid \a g_2 from grid \a g_1.  \a g_2 is
unaltered and the result stored in \a g_1.

\param   g_1   A Eh_dbl_grid
\param   g_2   A Eh_dbl_grid to subtract

\return The LHS grid
*/
Eh_dbl_grid
eh_dbl_grid_subtract( Eh_dbl_grid g_1 , Eh_dbl_grid g_2 )
{
   eh_require( g_1 );

   if ( g_1 && g_2 )
   {
      gint       i;
      const gint n_i      = g_1->n_x*g_1->n_y;
      double*    g_1_data = eh_grid_data_start(g_1);
      double*    g_2_data = eh_grid_data_start(g_2);

      eh_require( eh_grid_is_compatible( g_1 , g_2 ) );

      for ( i=0 ; i<n_i ; i++ )
         g_1_data[i] -= g_2_data[i];
   }

   return g_1;
}

/** Add one grid to another

Add grid \a g_2 to grid \a g_1.  \a g_2 is
unaltered and the result stored in \a g_1.

\param   g_1   A Eh_dbl_grid
\param   g_2   A Eh_dbl_grid to add

\return The LHS grid
*/
Eh_dbl_grid
eh_dbl_grid_add( Eh_dbl_grid g_1 , Eh_dbl_grid g_2 )
{
   eh_require( g_1 );

   if ( g_1 && g_2 )
   {
      gint       i;
      const gint n_i      = g_1->n_x*g_1->n_y;
      double*    g_1_data = eh_grid_data_start(g_1);
      double*    g_2_data = eh_grid_data_start(g_2);

      eh_require( eh_grid_is_compatible( g_1 , g_2 ) );

      for ( i=0 ; i<n_i ; i++ )
         g_1_data[i] += g_2_data[i];
   }

   return g_1;
}

/** Find the sum of a grid

Sum all of the elements of a Eh_dbl_grid, ignoring nans.

\param   g   A Eh_dbl_grid

\return The sum of the grid's elements
*/
double
eh_dbl_grid_sum( Eh_dbl_grid g )
{
   return eh_dbl_grid_sum_bad_val( g , eh_nan() );
}

/** Find the sum of a grid, ignoring bad values

Sum all of the elements of a Eh_dbl_grid, ignoring values
equal to \a bad_val.

\param   g         A Eh_dbl_grid
\param   bad_val   Value to ignore when calculating the sum

\return The sum of the grid's elements
*/
double
eh_dbl_grid_sum_bad_val( Eh_dbl_grid g , double bad_val )
{
   double sum=0;

   eh_require( g );

   if ( g )
   {
      gint          i;
      const gint    n_i  = eh_grid_n_el(g);
      double*       data = eh_grid_data_start( g );

      if ( eh_isnan( bad_val ) )
      {
         for ( i=0 ; i<n_i ; i++ )
            if ( !eh_isnan( data[i] ) )
               sum += data[i];
      }
      else
         for ( i=0 ; i<n_i ; i++ )
            if ( !eh_compare_dbl( data[i] , bad_val , 1e-12 ) )
               sum += data[i];
   }

   return sum;
}

/** Set all values of a grid

Set all of the elements of \a g to \a val.

\param   g     A Eh_dbl_grid
\param   val   The value to set the elements to

\return The input Eh_dbl_grid
*/
Eh_dbl_grid
eh_dbl_grid_set( Eh_dbl_grid g , const double val )
{
   if ( g )
   {
      gint       i;
      const gint n_i  = eh_grid_n_el( g );
      double*    data = eh_grid_data_start( g );
      for ( i=0 ; i<n_i ; i++ )
         data[i] = val;
   }

   return g;
}

/** Set values of a grid to a random number between 0 and 1

Set each value of a grid to a random number taken from a
uniform distribution between 0 and 1.

\param   g   A Eh_dbl_grid

\return The grid with its elements set
*/
Eh_dbl_grid
eh_dbl_grid_randomize( Eh_dbl_grid g )
{
   if ( g )
   {
      gint       i;
      const gint n_i  = eh_grid_n_el( g );
      double*    data = eh_grid_data_start( g );
      for ( i=0 ; i<n_i ; i++ )
         data[i] = g_random_double();
   }
   return g;
}

/** Multiply a grid by a scalar

Multiply each element of grid \a g by the scalar value \a val.

\param   g     A Eh_dbl_grid
\param   val   The value to multiply the grid by

\return The input grid
*/
Eh_dbl_grid
eh_dbl_grid_scalar_mult( Eh_dbl_grid g , double val )
{
   eh_require( g )

   if ( g )
   {
      gint       i;
      const gint n_i  = eh_grid_n_el(g);
      double*    data = eh_grid_data_start(g);

      for ( i=0 ; i<n_i ; i++ )
         data[i] *= val;
   }

   return g;
}

/** Rotate a grid

Rotate the Eh_dbl_grid about one of its elements (\a i_0,\a j_0) by an
amount \a angle.  The rotation angle is specified in radians.  Because the
grid is square, some of the elements will be rotated outside of the grid.
If \a lost is non-NULL, it's value is set to the sum of all such elements.

\param   g       A Eh_dbl_grid
\param   angle   The angle (in radians) to the grid by
\param   i_0     The row index of the rotation point
\param   j_0     The column index of the rotation point
\param   lost    The sum of the values that are rotated out of the grid

\return  The (rotated) input grid
*/
Eh_dbl_grid
eh_dbl_grid_rotate( Eh_dbl_grid g , double angle , gssize i_0 , gssize j_0 , double* lost )
{
   if ( g && !eh_compare_dbl(angle,0.,1e-12) )
   {
      double mass_error = 0;
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
                  temp_data[i_rotate][j_rotate] += data[i][j];
//                  temp->data[i_rotate][j_rotate] = g->data[i][j];
               else
                  mass_error += data[i][j];
            }
         }

      if ( lost )
         *lost = mass_error;

      eh_grid_copy_data( g , temp );
      eh_grid_destroy( temp , TRUE );
   }
   else
   {
      if ( lost )
         *lost = 0.;
   }

   return g;
}

/** Reduce the size of a grid

Decrease the size of the grid \a g.  The new grid will have
\a new_n_x rows and \a new_n_y columns.  The values of the
elements of the grid are interpolated onto the new grid.

\param   g         An Eh_dbl_grid
\param   new_n_x   The number of rows of the reduced grid
\param   new_n_y   The number of columns of the reduced grid

\return A newly-created Eh_dbl_grid with the specified size
*/
Eh_dbl_grid
eh_dbl_grid_reduce( Eh_dbl_grid g , gint new_n_x , gint new_n_y )
{
   eh_require( new_n_x<=g->n_x );
   eh_require( new_n_y<=g->n_y );

   return eh_dbl_grid_remesh( g , new_n_x , new_n_y );
}

/** Expand the size of a grid

Increase the size of the grid \a g.  The new grid will have
\a new_n_x rows and \a new_n_y columns.  The values of the
elements of the grid are interpolated onto the new grid.

\param   g         An Eh_dbl_grid
\param   new_n_x   The number of rows of the expanded grid
\param   new_n_y   The number of columns of the expanded grid

\return A newly-created Eh_dbl_grid with the specified size
*/
Eh_dbl_grid
eh_dbl_grid_expand( Eh_dbl_grid g , gint new_n_x , gint new_n_y )
{
   eh_require( new_n_x>=g->n_x );
   eh_require( new_n_y>=g->n_y );

   return eh_dbl_grid_remesh( g , new_n_x , new_n_y );
}

/** Change the size of a grid

Change the size of the grid \a g.  The new grid will have
\a new_n_x rows and \a new_n_y columns.  The values of the
elements of the grid are interpolated onto the new grid.

\param   g         An Eh_dbl_grid
\param   new_n_x   The number of rows of the new grid
\param   new_n_y   The number of columns of the new grid

\return A newly-created Eh_dbl_grid with the specified size
*/
Eh_dbl_grid
eh_dbl_grid_remesh( Eh_dbl_grid g , gint new_n_x , gint new_n_y )
{
   Eh_dbl_grid new_grid = NULL;

   eh_require( g );

   if ( g )
   {
      if ( new_n_x == g->n_x && new_n_y == g->n_y )
         return eh_grid_dup( g );

      new_grid = eh_grid_new( double , new_n_x , new_n_y );

      eh_require( new_grid );

      if ( new_grid )
      {
         double*     x_ind     = eh_new( double , g->n_x        );
         double*     y_ind     = eh_new( double , g->n_y        );
         double*     new_x_ind = eh_new( double , new_grid->n_x );
         double*     new_y_ind = eh_new( double , new_grid->n_y );

         eh_grid_reindex( new_grid , g->low_x , g->low_y );

         { /* Set the x and y positions of the grids */
            gint        i,j;
            double      dx = (g->n_x-1.) / (double)(new_n_x-1.);
            double      dy = (g->n_y-1.) / (double)(new_n_y-1.);

            for ( i=0 ; i<g->n_x        ; i++ ) x_ind[i]     = i+g->low_x;
            for ( j=0 ; j<g->n_y        ; j++ ) y_ind[j]     = j+g->low_y;

            if ( g->n_x==1 && new_n_x==1 ) dx = 1;
            if ( g->n_y==1 && new_n_y==1 ) dy = 1;

            for ( i=0 ; i<new_grid->n_x ; i++ ) new_x_ind[i] = i*dx+new_grid->low_x;
            for ( j=0 ; j<new_grid->n_y ; j++ ) new_y_ind[j] = j*dy+new_grid->low_y;

            if ( new_x_ind[i-1] > x_ind[g->n_x-1] ) new_x_ind[i-1] = x_ind[g->n_x-1]*(1.-1e-6);
            if ( new_y_ind[i-1] > y_ind[g->n_y-1] ) new_y_ind[i-1] = y_ind[g->n_y-1]*(1.-1e-6);
         }

         { /* Interpolate the x and y indices */
            interpolate( x_ind         , x_ind                     , g->n_x ,
                         new_x_ind     , eh_grid_x_start(new_grid) , new_grid->n_x );
            interpolate( y_ind         , y_ind                     , g->n_y ,
                         new_y_ind     , eh_grid_y_start(new_grid) , new_grid->n_y );
         }

         { /* Interpolate to the new grid */
            double* orig_x = g->x;
            double* orig_y = g->y;

            g->x = x_ind;
            g->y = y_ind;

            interpolate_2( g , new_grid );

            g->x = orig_x;
            g->y = orig_y;
         }

         eh_free( x_ind     );
         eh_free( y_ind     );
         eh_free( new_x_ind );
         eh_free( new_y_ind );
      }
   }

   return new_grid;
}

void
interpolate_2( Eh_dbl_grid source , Eh_dbl_grid dest )
{
   interpolate_2_bad_val( source , dest , eh_nan() );
}

void
interpolate_2_bad_val( Eh_dbl_grid source , Eh_dbl_grid dest ,
                       double bad_val )
{
   gssize i,j;
   Eh_dbl_grid temp;
   double *temp_source, *temp_dest;
   gssize src_low_x=eh_grid_low_x(source), dest_low_x=eh_grid_low_x(dest);
   gssize src_low_y=eh_grid_low_y(source), dest_low_y=eh_grid_low_y(dest);

   eh_grid_reindex( source , 0 , 0 );
   eh_grid_reindex( dest   , 0 , 0 );

   temp        = eh_grid_new( double , eh_grid_n_x(source) , eh_grid_n_y(dest) );
   temp_source = eh_new( double , eh_grid_n_x(source) );
   temp_dest   = eh_new( double , eh_grid_n_x(dest)   );

   for ( i=0 ; i<eh_grid_n_x(source) ; i++ )
      interpolate_bad_val( eh_grid_y(source) , eh_grid_row(source,i) , eh_grid_n_y(source) ,
                           eh_grid_y(dest)   , eh_grid_row(temp,i)   , eh_grid_n_y(dest)   ,
                           bad_val );

   for ( j=0 ; j<eh_grid_n_y(dest) ; j++ )
   {
      for ( i=0 ; i<eh_grid_n_x(source) ; i++ )
         temp_source[i] = eh_dbl_grid_val(temp,i,j);

      interpolate_bad_val( eh_grid_x(source) , temp_source , eh_grid_n_x(source) ,
                           eh_grid_x(dest)   , temp_dest   , eh_grid_n_x(dest)   ,
                           bad_val );

      for ( i=0 ; i<eh_grid_n_x(dest) ; i++ )
         eh_dbl_grid_set_val( dest , i , j , temp_dest[i] );
//         dest->data[i][j] = temp_dest[i];
   }

   eh_free( temp_dest   );
   eh_free( temp_source );
   eh_grid_destroy( temp , TRUE );

   eh_grid_reindex( source , src_low_x  , src_low_y  );
   eh_grid_reindex( dest   , dest_low_x , dest_low_y );

   return;
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

Eh_grid
eh_grid_sub( Eh_grid g , gint i_0 , gint j_0 , gint n_x , gint n_y )
{
   Eh_grid sub = NULL;

   eh_require( g );

   if ( g )
   {
      if ( i_0     < g->low_x ) i_0 = g->low_x;
      if ( j_0     < g->low_y ) j_0 = g->low_y;

      if ( i_0+n_x > g->n_x   ) n_x = g->n_x-i_0;
      if ( j_0+n_y > g->n_y   ) n_y = g->n_y-j_0;

      if ( n_x>0 && n_y>0 )
      {
         gint    i;
         gchar** d = (gchar**)(g->data);

         sub = eh_grid_malloc( n_x , n_y , g->el_size );

         for ( i=0 ; i<sub->n_x ; i++ )
            memcpy( sub->data[i] , d[i_0+i]+j_0*g->el_size , sub->n_y*g->el_size );

         memcpy( sub->x , g->x+i_0 , sub->n_x*sizeof(double) );
         memcpy( sub->y , g->y+j_0 , sub->n_y*sizeof(double) );
      }
   }

   return sub;
}

/*
void
eh_dbl_grid_rebin( Eh_dbl_grid source , gint id_ul , gint id_lr , double dx , double dy )
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

   dest_n_x = eh_dbl_array_rebin_len( src_n_x , dx );
   dest_n_y = eh_dbl_array_rebin_len( src_n_y , dy );

   dest = eh_grid_new( double , dest_n_x , dest_n_y );

   for ( i=0 ; i<n_x ; i++ )
   {
      eh_dbl_array_rebin( dest->data[i] , source->data[i]+j_0 , n_y , dy , &temp_len );
      if ( temp_len != dest_n_y )
         eh_require_not_reached();
   }

   for ( j=0 ; j<dest_n_y ; j++ )
   {
      eh_dbl_col_to_array( temp_source , temp[0]+j , dest_n_x , dest_n_y );

      temp_dest = eh_dbl_array_rebin( temp_source , n_x , dx , &temp_len );

      if ( temp_len != dest_n_x )
         eh_require_not_reached();

      eh_dbl_array_to_col( dest_data[0]+j , temp_dest , dest_n_x , dest_n_y );

      for ( i=0 ; i<dest_n_x ; i++ )
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
*/

Eh_dbl_grid
eh_dbl_grid_rebin( Eh_dbl_grid source , Eh_dbl_grid dest )
{
   return eh_dbl_grid_rebin_bad_val( source , dest , eh_nan() );
}

Eh_dbl_grid
eh_dbl_grid_rebin_bad_val( Eh_dbl_grid source , Eh_dbl_grid dest , double bad_val )
{
   eh_require( source );
   eh_require( dest   );

   if ( source && dest )
   {
      gint src_low_x  = source->low_x;
      gint dest_low_x = dest->low_x;
      gint src_low_y  = source->low_y;
      gint dest_low_y = dest->low_y;

      eh_grid_reindex( source , 0 , 0 );
      eh_grid_reindex( dest   , 0 , 0 );

      {
         double** temp = eh_new_2( double , source->n_x , dest->n_y );

         { /* Rebin the rows of the destination grid */
            gint i;
            for ( i=0 ; i<source->n_x ; i++ )
               eh_rebin_dbl_array_bad_val( source->y , source->data[i] , source->n_y ,
                                           dest->y   , temp[i]         , dest->n_y   ,
                                           bad_val );
         }

//   dest_data = dest->data;

         {/* Rebin the columns of the destination grid */
            double*  temp_source = eh_new( double , source->n_x );
            double*  temp_dest   = eh_new( double , dest->n_x   );
            double** dest_data   = eh_dbl_grid_data( dest );
            gint i, j;

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
         }

         eh_free_2( temp );
      }

      eh_grid_reindex( source , src_low_x  , src_low_y  );
      eh_grid_reindex( dest   , dest_low_x , dest_low_y );
   }

   return dest;
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

Eh_dbl_grid
eh_dbl_grid_populate( Eh_dbl_grid g  , Populate_func f , gpointer user_data )
{
   if ( g && f )
   {
      const gint64 population_size=100000;
      const gint64 low_x  = g->low_x;
      const gint64 low_y  = g->low_y;
      const gint64 high_x = g->low_x+g->n_x;
      const gint64 high_y = g->low_y+g->n_y;
      gint64 i, j;
      gint64 n;
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
   }

   return g;
}

gint
eh_dbl_grid_fprintf( FILE* fp , const gchar* format , Eh_dbl_grid g )
{
   gint n = 0;

   eh_require( g      );
   eh_require( fp     );
   eh_require( format );

   if ( g && fp && format )
   {
      gint       i, j;
      double**   d     = eh_dbl_grid_data( g );
      const gint i_0   = eh_grid_low_x( g );
      const gint j_0   = eh_grid_low_y( g );
      const gint top_i = eh_grid_top_x( g );
      const gint top_j = eh_grid_top_y( g );

      for ( i=i_0 ; i<=top_i ; i++ )
      {
         for ( j=j_0 ; j<=top_j ; j++ )
            n += fprintf( fp , format , d[i][j] );
         n += fprintf( fp , "\n" );
      }
   }

   return n;
}

#include <string.h> // included for g_memmove

/** Transpose the row and columns of a grid

\param   g   An Eh_grid

\return The transposed input Eh_grid 
*/
Eh_grid
eh_grid_transpose( Eh_grid g )
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

/** Find the n-th order difference of a grid

Calculate the n-th order difference or derivative of a grid along a
specified dimension.  \a dim is either 1 or 2 to indicate 
differences to be taken along rows or columns, respectively.

\param   g      A double grid
\param   n      Order of the difference to calculate
\param   dim    Dimension of the grid to difference (1 or 2)

\return A newly-allocated Eh_dbl_grid of differences
*/
Eh_dbl_grid
eh_dbl_grid_diff( Eh_dbl_grid g , gssize n , gssize dim )
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

/** Trim rows of the grid that are less than some value

Starting along each edge, trim rows and columns that contain
no elements greater that \a val.

\param   g   A double grid
\param   val Comparison value

\return A newly-allocated Eh_dbl_grid
*/
Eh_dbl_grid
eh_dbl_grid_trim_gt( Eh_dbl_grid g , double val )
{
   Eh_dbl_grid t = NULL;

   eh_require( g );

   if ( g )
   {
      gint* box = eh_dbl_grid_crop_box_gt( g , val );

      t = eh_grid_sub( g , box[0] , box[1] , box[2] , box[3] );

      eh_free( box );
   }

   return t;
}

/** The smallest box that contains values greater than a value

Find a box that excludes rows and columns that have no elements greater
than \a val.  Rows and columns are examined starting from each edge
until a row or column is found that has at least a single element greater
than \a val.

The bounding box is returned as an array that defines the position of its
upper-left corner, and its width and height.  The elements of the array are
{ ROW , COL , N_ROWS , N_COLS } where (ROW,COL) are the indices of the
upper-left corner, and (N_ROWS,N_COLS) are the height and width of the box.

\param   g   A double grid
\param   val Comparison value

\return A newly-allocated array that defines the limits of the bounding box.
*/
gint*
eh_dbl_grid_crop_box_gt( Eh_dbl_grid g , double val )
{
   gint* box = NULL;

   if ( g )
   {
      box[0] = eh_dbl_grid_first_row_gt( g , val );
      box[2] = eh_dbl_grid_last_row_gt ( g , val ) + box[0] + 1;
      
      box[1] = eh_dbl_grid_first_col_gt( g , val );
      box[3] = eh_dbl_grid_last_col_gt ( g , val ) + box[1] + 1;
   }

   return box;
}

/** The first row that contains only elements greater than a value

\param   g   A double grid
\param   val Comparison value

\return Index to the row or -1 if not found.
*/
gint
eh_dbl_grid_first_row_gt( Eh_dbl_grid g , double val )
{
   gint row = -1;

   if ( g )
   {
      const gint n_x = eh_grid_n_x( g );

      for ( row=0 ; row<n_x ; row++ )
         if ( eh_dbl_grid_row_is_gt(g,row,val) )
            break;
   }

   return row;
}

/** The first column that contains only elements greater than a value

\param   g   A double grid
\param   val Comparison value

\return Index to the column or -1 if not found.
*/
gint
eh_dbl_grid_first_col_gt( Eh_dbl_grid g , double val )
{
   gint col = -1;

   if ( g )
   {
      const gint n_y = eh_grid_n_y( g );

      for ( col=0 ; col<n_y ; col++ )
         if ( eh_dbl_grid_col_is_gt(g,col,val) )
            break;
   }

   return col;
}

/** The last row that contains only elements greater than a value

\param   g   A double grid
\param   val Comparison value

\return Index to the row or -1 if not found.
*/
gint
eh_dbl_grid_last_row_gt( Eh_dbl_grid g , double val )
{
   gint row = -1;

   if ( g )
   {
      const gint n_x = eh_grid_n_x( g );

      for ( row=n_x-1 ; row>0 ; row-- )
         if ( eh_dbl_grid_row_is_gt(g,row,val) )
            break;
   }

   return row;
}

/** The last column that contains only elements greater than a value

\param   g   A double grid
\param   val Comparison value

\return Index to the column or -1 if not found.
*/
gint
eh_dbl_grid_last_col_gt( Eh_dbl_grid g , double val )
{
   gint col = -1;

   if ( g )
   {
      const gint n_y = eh_grid_n_y( g );

      for ( col=n_y-1 ; col>=0 ; col++ )
         if ( eh_dbl_grid_col_is_gt(g,col,val) )
            break;
   }

   return col;
}

/** Are elements of a row greater than some value?

Compare all elements of a row in a Eh_dbl_grid to \a val.  

\param g   A double grid
\param row The row to test
\param val The value to compare

\return    TRUE if all elemets are greater than the value
*/
gboolean
eh_dbl_grid_row_is_gt( Eh_dbl_grid g , gint row , double val )
{
   gboolean is_gt = FALSE;

   eh_require( g );

   if ( g )
   {
      double* x = eh_grid_row( g , row );

      if ( x )
      {
         gint       j;
         const gint n_y = eh_grid_n_y( g );

         for ( j=0 ; j<n_y ; j++ )
            if ( x[j] > val )
               break;

         if ( j<n_y ) is_gt = TRUE;
      }
   }

   return is_gt;
}

/** Are elements of a column greater than some value?

Compare all elements of a column in a Eh_dbl_grid to \a val.  

\param g   A double grid
\param col The column to test
\param val The value to compare

\return    TRUE if all elemets are greater than the value
*/
gboolean
eh_dbl_grid_col_is_gt( Eh_dbl_grid g , gint col , double val )
{
   gboolean is_gt = FALSE;

   eh_require( g );

   if ( g )
   {
      //double* x = eh_grid_col( g , col );

      {
         double*    set = eh_dbl_grid_data_start( g ) + col;
         double*    end = eh_dbl_grid_data_start( g ) + eh_grid_n_el(g)*eh_grid_el_size(g);
         const gint off = eh_grid_n_y( g );
         double*    cur;

         for ( cur=set ; cur<end ; cur+=off )
            if ( *cur > val )
               break;

         if ( cur<end ) is_gt = TRUE;
      }
   }

   return is_gt;
}

#endif

