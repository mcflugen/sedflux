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

//#include "sed_sedflux.h"

#include <glib.h>
#include <stdio.h>

#include "utils.h"

#include "sed_column.h"

CLASS( Sed_column )
{
   Sed_cell* cell;    ///< Array of cells making up the column
   double z;          ///< Height from some datum to bottom of column
   double t;          ///< The thickness of the column
   gssize len;        ///< Number of filled cells in the column
   gssize size;       ///< Total amount of cells available in column
   double dz;         ///< Height of a cell of sediment
   double x;          ///< x-position of this column
   double y;          ///< y-position of this column
   double age;        ///< age of this column
   double sl;         ///< sea level
};

//@Include: sed_column.h

/** Create a column of sediment

@param n_bins the number of Sed_cell's in the column.

@return A newly created Sed_column.  NULL is returned if there was a problem
        allocating memory.

@see sed_column_destroy
*/
Sed_column sed_column_new( gssize n_bins )
{
   Sed_column s = NULL;
   
   if ( n_bins>0 )
   {
      NEW_OBJECT( Sed_column , s );
   
      // use resize to allocate memory for the column in blocks.
      s->size = 0;
      s->cell = NULL;
      sed_column_resize( s , n_bins );

      s->len = 0;
      s->dz  = 1.;
      s->t   = 0.;
      s->z   = 0.;
      s->x   = 0.;
      s->y   = 0.;
      s->age = 0.;
      s->sl  = 0.;
   }
      
   return s;
}

Sed_column sed_column_new_filled( double t , Sed_size_class size )
{
   Sed_column c  = sed_column_new( 1 );
   Sed_cell cell = sed_cell_new_classed( NULL , t , size );

   sed_column_add_cell( c , cell );

   sed_cell_destroy( cell );

   return c;
}

/** Destroy a column of sediment.

@param s        The Sed_column to be destroyed.

@see sed_column_new
*/
Sed_column sed_column_destroy( Sed_column s )
{
   if ( s )
   {
      gssize i;
   
      for ( i=0;i<s->size;i++)
         sed_cell_destroy(s->cell[i]);
      eh_free(s->cell);

      eh_free(s);
   }
   
   return NULL;
}

/** Remove all of the sediment from a column.

@param s The column to remove the sediment from.
*/
Sed_column sed_column_clear( Sed_column s )
{
   if ( s )
   {
      gssize i;

      for ( i=0 ; i<s->len ; i++ )
         sed_cell_clear( s->cell[i] );
      s->len = 0;
      s->t   = 0.;
   }
   return s;
}

/** Copy one column to another.

Copy the contents of one Sed_column into another.  Memory must already be
allocated for the destination column.  Any sediment information that is 
stored in the destination column will be destroyed and replaced with the
information from the source column.

@param dest    The destination column.
@param src     The source column.

@return The destination column is returned.

@see sed_dup_column
*/
Sed_column sed_column_copy( Sed_column dest , const Sed_column src )
{
   eh_require( src );

   if ( src )
   {
      gssize i;

      if ( !dest )
         dest = sed_column_new( src->size );
   
      sed_column_resize( dest , src->size );

      dest->z   = src->z;
      dest->t   = src->t;
      dest->len = src->len;
      dest->dz  = src->dz;
      dest->x   = src->x;
      dest->y   = src->y;
      dest->age = src->age;
      dest->sl  = src->sl;

      for ( i=0 ; i<src->size ; i++ )
         sed_cell_copy( dest->cell[i] , src->cell[i] );
   }
   else
      dest = NULL;
   
   return dest;
}

gboolean sed_column_is_same( const Sed_column c_1 , const Sed_column c_2 )
{
   gboolean same = TRUE;

   if ( c_1!=c_2 )
   {
      if ( sed_column_is_same_data(c_1,c_2) )
      {
         gssize i;
         gssize len = sed_column_len( c_1 );
         for ( i=0 ; same && i<len ; i++ )
            same = sed_cell_is_same( c_1->cell[i] , c_2->cell[i] );
      }
      else
         same = FALSE;
   }

   return same;
}

gboolean sed_column_is_same_data( const Sed_column c_1 , const Sed_column c_2 )
{
   gboolean same = TRUE;

   if ( c_1!=c_2 )
   {
      same =  eh_compare_dbl( c_1->z   , c_2->z   , 1e-12 )
           && eh_compare_dbl( c_1->t   , c_2->t   , 1e-12 )
           && eh_compare_dbl( c_1->dz  , c_2->dz  , 1e-12 )
           && eh_compare_dbl( c_1->x   , c_2->x   , 1e-12 )
           && eh_compare_dbl( c_1->y   , c_2->y   , 1e-12 )
           && eh_compare_dbl( c_1->age , c_2->age , 1e-12 ) 
           && eh_compare_dbl( c_1->sl  , c_2->sl  , 1e-12 ) 
           && c_1->len == c_2->len;
   }

   return same;
}

Sed_column sed_column_copy_data( Sed_column dest , const Sed_column src )
{
   eh_require( src )
   {
      dest->z    = src->z;
      dest->t    = src->t;
      dest->len  = src->len;
      dest->size = src->size;
      dest->dz   = src->dz;
      dest->x    = src->x;
      dest->y    = src->y;
      dest->age  = src->age;
      dest->sl   = src->sl;
   }

   return dest;
}

Sed_column sed_column_copy_public_data( Sed_column dest , const Sed_column src )
{
   eh_require( src )
   {
      dest->z    = src->z;
      dest->dz   = src->dz;
      dest->x    = src->x;
      dest->y    = src->y;
      dest->age  = src->age;
      dest->sl   = src->sl;
   }

   return dest;
}

/** Create a column as a copy of another.

This is the copy constructor for a Sed_column.

@param src The source column.

@return The newly created column is returned.

@see sed_dup_column
*/
Sed_column sed_column_dup( const Sed_column src )
{
   return sed_column_copy( NULL , src );
}

/** Get fraction information from a Sed_cell of a Sed_column.

Get an array of fractions for each grain type contained within the specified
Sed_cell.  The index is counted from the bottom of the Sed_column to the top.
That is, i=0 refers to the lowest cell within the column.

@param col A pointer to a Sed_column.
@param i   Index to a Sed_cell of the column.

@return A pointer to the fraction information for the requested Sed_cell.
*/
double* sed_column_cell_fraction( const Sed_column col , gssize i )
{
   return sed_cell_fraction_ptr( col->cell[i] );
}

/** Get the height of a Sed_column

@param col A pointer to a Sed_column.

@return The height to the bottom of a Sed_column.
*/
double sed_column_base_height( const Sed_column col)
{
   eh_return_val_if_fail( col , 0 );
   return col->z;
}

/** Get the position of a Sed_column

Get the horizontal position of a Sed_column.  Currently this is only designed
for 2d and so the position only contains one horizontal coordinate.

@param col A pointer to a Sed_column.

@return The position of the Sed_column.

@see sed_get_column_x_position , sed_get_column_y_position .
*/
double sed_column_position( const Sed_column col )
{
   return col->x;
}

/** Get the x-position of a Sed_column

Get the x-coordinate of the horizontal position of a Sed_column.  This is now
the favored version to get the position of a column.  sed_get_column_position
should no longer be used.

@param col A pointer to a Sed_column.

@return The x-position of the Sed_column.

@see sed_get_column_y_position .
*/
double sed_column_x_position( const Sed_column col )
{
   return col->x;
}

/** Get the y-position of a Sed_column

Get the y-coordinate of the horizontal position of a Sed_column.  This is now
the favored version to get the position of a column.  sed_get_column_position
should no longer be used.

@param col A pointer to a Sed_column.

@return The y-position of the Sed_column.

@see sed_get_column_x_position .
*/
double sed_column_y_position( const Sed_column col )
{
   return col->y;
}

double sed_column_age( const Sed_column col )
{
   return col->age;
}

double sed_column_sea_level( const Sed_column col )
{
   return col->sl;
}

/** Set the position of a Sed_column

Set the horizontal position of a Sed_column.  Currently this is only designed
for 2d and so the position only contains one horizontal coordinate.

NOTE: This function is now deprecated.  Use sed_set_column_x_position instead.

@param col      A pointer to a Sed_column.
@param x        The new horizontal location of the Sed_column.

@return A pointer to the input Sed_column.

@see sed_set_column_x_position , sed_set_column_y_position .
*/
Sed_column sed_column_set_position( Sed_column col , double x )
{
   col->x = x;
   return col;
}

/** Set the x-position of a Sed_column

Set the x-coordinate of the horizontal position of a Sed_column.  This is the
favored version to set a columns position.  sed_set_column_position should
no longer be used.

@param col A pointer to a Sed_column.
@param x   The new x-coordinate of the horizontal location of the Sed_column.

@return A pointer to the input Sed_column.

@see sed_set_column_y_position .
*/
Sed_column sed_column_set_x_position( Sed_column col , double x )
{
   col->x = x;
   return col;
}

/** Set the y-position of a Sed_column

Set the y-coordinate of the horizontal position of a Sed_column.  This is the
favored version to set a columns position.  sed_set_column_position should
no longer be used.

@param col A pointer to a Sed_column.
@param y   The new y-coordinate of the horizontal location of the Sed_column.

@return A pointer to the input Sed_column.

@see sed_set_column_x_position .
*/
Sed_column sed_column_set_y_position( Sed_column col , double y )
{
   col->y = y;
   return col;
}

Sed_column sed_column_set_age( Sed_column c , double age )
{
   c->age = age;
   return c;
}

Sed_column sed_column_set_sea_level( Sed_column c , double sl )
{
   c->sl = sl;
   return c;
}

Sed_column sed_column_set_base_height( Sed_column c , double z )
{
   c->z = z;
   return c;
}

Sed_column sed_column_adjust_base_height( Sed_column c , double dz )
{
   c->z += dz;
   return c;
}

/** Get the vertical resolution of a Sed_column

The vertical resolution of a Sed_column is the initial thickness of the 
Sed_cell's that make it up.  Sediment can be added to the cell until it 
reached this thickness.  After it is full, sediment is then added to the 
cell above.  Also, it can later be squeezed to a new thickness.

@param col A pointer to a Sed_column.

@return The vertical resolution of the Sed_column.
*/
double sed_column_cell_height( const Sed_column col )
{
   return col->dz;
}

double sed_column_z_res( const Sed_column col )
{
   return col->dz;
}

Sed_column sed_column_set_z_res( Sed_column col , double new_dz )
{
   col->dz = new_dz;
   return col;
}


/** Get the elevation to the top of Sed_column.

@param col A pointer to a Sed_column.

@return The elevation to the last filled Sed_cell in a Sed_column.
*/
double sed_column_top_height( const Sed_column col )
{
   eh_return_val_if_fail( col , 0 );
   return col->z + sed_column_thickness(col);
}

/** Test if the top of a column is below some elevation.

@param col     A pointer to a Sed_column.
@param z       The reference elevation.

@return TRUE if the top of the column is below the elevation.

@see sed_column_above .
*/
gboolean sed_column_is_below( Sed_column col , double z )
{
   return sed_column_top_height( col ) < z;
}

/** Test if the top of a column is above some elevation.

@param col     A pointer to a Sed_column.
@param z       The reference elevation.

@return        TRUE if the top of the column is above the elevation.

@see sed_column_below .
*/
gboolean sed_column_is_above( Sed_column col , double z )
{
   return sed_column_top_height( col ) > z;
}

/** Get the mass of sediment within a column.

@param s A pointer to a Sed_column.

@return The total mass of sediment contained within a Sed_column.
*/
double sed_column_mass(const Sed_column s)
{
   double sum = 0;

   eh_require( s );
   {
      gssize i;
      gssize n_bins = sed_column_len(s);

      for ( i=0 ; i<n_bins ; i++ )
         sum += sed_cell_mass( s->cell[i] );
   }

   return sum;
}

double sed_column_sediment_mass(const Sed_column s)
{
   double sum = 0;

   eh_require( s );
   {
      gssize i;
      gssize n_bins = sed_column_len(s);

      for ( i=0 ; i<n_bins ; i++ )
         sum += sed_cell_sediment_mass( s->cell[i] );
   }

   return sum;
}

/** Get the total load felt by each cell of sediment in a Sed_column.

Get the load felt by each cell of sediment due to its overlying cells.
Returns an array containing the load felt by each cell (in units of Pa).
The returned array should be freed using free.  If a value of NULL is given
for the load pointer, a newly allocated array will be used.  The zero-th
element of the array will be the load on the start-th bin from the bottom
of the column.  If n_bins <= 0, we go from the start-th bin to the top. 
If a value of NULL is given for the load pointer, a newly allocated array
will be used.

@param s              A pointer to a Sed_column.
@param start          Index to the Sed_cell to begin at.
@param n_bins         The number of Sed_cell's to consider.
@param overlying_load The load overlying the Sed_column.
@param load           A pointer to an array that will hold the load values.

@return A pointer to an array of loads.
*/
double *sed_column_total_load( const Sed_column s    ,
                               gssize start          ,
                               gssize n_bins         ,
                               double overlying_load ,
                               double *load )
{

   eh_require( s );

   if ( s )
   {
      gssize i;
      gssize col_len = s->len;
      double load0 = 0;

      eh_lower_bound( start , 0 );

      if ( n_bins <= 0 || start+n_bins>col_len )
         n_bins = col_len - start;

      // calculate the load above the top cell.  the load on cell includes the
      // weight of itself.
      if ( !load )
         load = eh_new0( double , n_bins );

      for ( i=col_len-1 ; i>=start+n_bins-1 ; i-- )
         load0 += sed_cell_sediment_load( s->cell[i] );

      load0 += overlying_load;

      // calculate the overlying load on each of the cells.
      load[n_bins-1]=load0;
      for ( i=n_bins-2 ; i>=0 ; i-- )
         load[i] = load[i+1] + sed_cell_sediment_load( s->cell[i+start] );
   }
   else
      load = NULL;

   return load;
}

double* sed_column_load( const Sed_column s ,
                         gssize start       ,
                         gssize n_bins      ,
                         double* load )
{
   return sed_column_total_load( s , start , n_bins , 0. , load );
}

double* sed_column_load_with_water( const Sed_column s ,
                                    gssize start       ,
                                    gssize n_bins      ,
                                    double* load )
{
   double water_load = sed_column_water_pressure( s );

   return sed_column_total_load( s , start , n_bins , water_load , load );
}

double sed_column_water_pressure( const Sed_column s )
{
   double p = 0;

   if ( sed_column_water_depth(s)>0 )
      p = sed_column_water_depth(s)*sed_rho_sea_water()*sed_gravity();

   return p;
}

/** Get the total of a specified property for a series of Sed_cell's

This function will total a specifed property of a series of Sed_cell's within
a Sed_column.  The property will be obtained using the Sed_property_func, f.
The total will begin at the start'th cell and work up the Sed_column.  The 
total for each cell will be the sum of the property values of the cells
preceding it as well as its own.  If a value of NULL is passed for the val
parameter, an array will be created.  If n_bins<=0, the total will run to
the top of the sediment column.  If non-NULL value is passed for val, care
should be taken to assure that there is enough memory to hold all of the total
values.

@param f      A function to get a property from a Sed_cell.
@param c      A pointer to a Sed_column.
@param start  Index to the Sed_cell to begin at.
@param n_bins The number of Sed_cell's to consider.
@param val    A pointer to an array that will hold the total values.

@return A pointer to a possibly newly created array of total values.

@see sed_get_column_load , sed_get_column_avg_property_with_load ,
     sed_get_column_avg_property .
*/
double* sed_column_total_property( Sed_property f ,
                                   Sed_column c   ,
                                   gssize start   ,
                                   gssize n_bins  ,
                                   double* val )
{
   eh_require( c );

   if ( c )
   {
      gssize i;
      gssize low_i;
      double val0 = 0.;
      gssize col_len = c->len;

      if ( n_bins <= 0 || start+n_bins>col_len )
         n_bins = col_len-start;

      low_i = start+n_bins-1;

      if ( !val )
         val = eh_new0( double , n_bins );

      for ( i=col_len-1 ; i>=low_i ; i-- )
         val0 += sed_property_measure( f , c->cell[i] );

      val[n_bins-1] = val0;
      for ( i=n_bins-2 ; i>=0 ; i-- )
         val[i] = val[i+1] + sed_property_measure( f , c->cell[i] );
   }
   else
      val = NULL;

   return val;
}

/** Get the average of a specified property for a series of Sed_cell's

This function will average a specifed property of a series of Sed_cell's within
a Sed_column.  The property will be obtained using the
Sed_property_with_load_func, f.
The average will begin at the start'th cell and work up the Sed_column.  The 
average for each cell will be the average of the property values of the cells
preceding it as well as its own.  If a value of NULL is passed for the val
parameter, an array will be created.  If n_bins<=0, the total will run to
the top of the sediment column.  If non-NULL value is passed for val, care
should be taken to assure that there is enough memory to hold all of the total
values.

This is the same as the sed_get_avg_property function except that here
the parameter, f is a Sed_property_with_load_func.

@param f      A function to get a property from a Sed_cell.
@param c      A pointer to a Sed_column.
@param start  Index to the Sed_cell to begin at.
@param n_bins The number of Sed_cell's to consider.
@param val    A pointer to an array that will hold the total values.

@return A pointer to a possibly newly created array of average values.

@see sed_get_column_load , sed_get_column_avg_property .
*/
double* sed_column_avg_property_with_load( Sed_property f ,
                                           Sed_column c   ,
                                           gssize start   ,
                                           gssize n_bins  ,
                                           double* val )
{
   eh_require( c );

   if ( c )
   {
      gssize i;
      double *t, *load;
      gssize col_len = c->len;

      if ( n_bins <= 0 || start+n_bins>col_len )
         n_bins = col_len-start;

      if ( !val )
         val = eh_new( double , n_bins );
      t = eh_new( double , n_bins );

      load = sed_column_load( c , start , n_bins , NULL );

      t[n_bins-1] = sed_cell_size( c->cell[n_bins-1] );
      for ( i=n_bins-2 ; i>=0 ; i-- )
         t[i] = t[i+1] + sed_cell_size( c->cell[i] );

      val[n_bins-1] = sed_property_measure( f , c->cell[n_bins-1] , load[n_bins-1] ); // this used to be load[i] (now load[n_bins-1])
      for ( i=n_bins-2 ; i>=0 ; i-- )
         val[i] = ( val[i+1]*t[i+1] + sed_property_measure( f , c->cell[i] , load[i] )*(t[i]-t[i+1]) ) / t[i];

      eh_free( t    );
      eh_free( load );
   }
   else
      val = NULL;

   return val;
}

/** Get the average of a specified property for a series of Sed_cell's

This function will average a specifed property of a series of Sed_cell's within
a Sed_column.  The property will be obtained using the
Sed_property_with_load_func, f.
The average will begin at the start'th cell and work up the Sed_column.  The 
average for each cell will be the average of the property values of the cells
preceding it as well as its own.  If a value of NULL is passed for the val
parameter, an array will be created.  If n_bins<=0, the total will run to
the top of the sediment column.  If non-NULL value is passed for val, care
should be taken to assure that there is enough memory to hold all of the total
values.

This is the same as the sed_get_avg_property_with_load function except that
here the parameter, f is a Sed_property_func.

@param f      A function to get a property from a Sed_cell.
@param c      A pointer to a Sed_column.
@param start  Index to the Sed_cell to begin at.
@param n_bins The number of Sed_cell's to consider.
@param val    A pointer to an array that will hold the average values.

@return A pointer to a possibly newly created array of average values.

@see sed_get_column_load         , sed_get_column_avg_property_with_load .
*/
double* sed_column_avg_property( Sed_property f , 
                                 Sed_column c   ,
                                 gssize start   ,
                                 gssize n_bins  ,
                                 double* val )
{
   eh_require( c );

   if ( c )
   {
      gssize i;
      double *t;
      gssize col_len = c->len;

      if ( n_bins <= 0 || start+n_bins>col_len )
         n_bins = col_len-start;

      if ( !val )
         val = eh_new( double , n_bins );
      t = eh_new( double , n_bins );

      t[n_bins-1] = sed_cell_size( c->cell[n_bins-1] );
      for ( i=n_bins-2 ; i>=0 ; i-- )
         t[i] = t[i+1] + sed_cell_size( c->cell[i] );

      val[n_bins-1] = sed_property_measure( f , c->cell[n_bins-1] );
      for ( i=n_bins-2 ; i>=0 ; i-- )
         val[i] = (   val[i+1]*t[i+1] 
                    + sed_property_measure( f , c->cell[i] )
                    * (t[i]-t[i+1]) ) 
                  / t[i];

      eh_free( t );
   }
   else
      val = NULL;

   return val;
}

/** Get the value of a specified property for a series of Sed_cell's

This function will get specifed property for a series of Sed_cell's within
a Sed_column.  The property will be obtained using the
Sed_property_func, f.
The first query will be at the start'th cell and work up the Sed_column.
If a value of NULL is passed for the val
parameter, an array will be created.  If n_bins<=0, the query will run to
the top of the sediment column.  If non-NULL value is passed for val, care
should be taken to assure that there is enough memory to hold all of the total
values.

@param f      A function to get a property from a Sed_cell.
@param c      A pointer to a Sed_column.
@param start  Index to the Sed_cell to begin at.
@param n_bins The number of Sed_cell's to consider.
@param val    A pointer to an array that will hold the values.

@return A pointer to a possibly newly created array of average values.

@see sed_get_column_load         , sed_get_column_avg_property_with_load .
     sed_get_column_avg_property .
*/
double* sed_column_at_property( Sed_property f ,
                                Sed_column c   ,
                                gssize start   ,
                                gssize n_bins  ,
                                double* val )
{
   eh_require( c );

   if ( c )
   {
      gssize i;
      gssize col_len = c->len;

      if ( n_bins <= 0 || start+n_bins>col_len )
         n_bins = col_len-start;

      if ( !val )
         val = eh_new( double , n_bins );

      for ( i=n_bins-1 ; i>=0 ; i-- )
         val[i] = sed_property_measure( f , c->cell[i] );
   }
   else
      val = NULL;

   return val;
}

/** Get the load felt by the cell n cells from the bottom of the column.

@param s A pointer to a Sed_column.
@param n The index of the cell where the load is felt.

@return The load felt by the n-th cell.
*/
double sed_column_load_at( const Sed_column s , gssize n )
{
   double load_0 = 0;

   eh_require( s );
   eh_require( n>=0 );

   if ( s )
   {
      gssize i, col_len = s->len;

      eh_lower_bound( n , 0 );
      for ( i=col_len-1 ; i>=n; i--)
         load_0 += sed_cell_load( s->cell[i] );
   }

   return load_0;
}

/** Get the average property from over the cells within a column.

Use the Sed_property_func, f, to get the average property value of all the
Sed_cell's in a Sed_column.

This is the same as sed_get_column_property_with_load except here f is of type
Sed_property_func.

@param f A function to get a property from a Sed_cell.
@param c A pointer to a Sed_column.

@return The average property value for the column.

@see sed_get_column_property .
*/
double sed_column_property_0( Sed_property f, const Sed_column c )
{
   double val = 0;

   eh_require( c );

   if ( c )
   {
      gssize i;
      gssize len = sed_column_len( c );

      for ( i=0 ; i<len ; i++ )
      {
         val += sed_property_measure( f , c->cell[i] )
              * sed_cell_size( c->cell[i] );
      }

      val /= sed_column_thickness(c);
   }

   return val;
}

/** Get the average property from over the cells within a column.

Use the Sed_property_with_load_func, f, to get the average property value
of all the Sed_cell's in a Sed_column.

This is the same as sed_get_column_property except here f is of type
Sed_property_with_load_func.

@param f A function to get a property from a Sed_cell.
@param s A pointer to a Sed_column.

@return The average property value for the column.

@see sed_get_column_property .
*/
double sed_column_property( Sed_property f ,
                            const Sed_column s )
{
   double val = 0;

   eh_require( s )

   if ( s )
   {
      gssize i;
      gssize len = sed_column_len( s );
      double* load = eh_new( double , len );

      if ( sed_property_n_args(f)==2 )
      {

         if (    sed_property_is_named( f , "consolidation" )
              || sed_property_is_named( f , "consolidation rate" ) )
         {
            double extra_arg = sed_column_age( s );

            for ( i=0 ; i<len ; i++ )
            {
               val += sed_property_measure( f , s->cell[i] , extra_arg )
                    * sed_cell_size(s->cell[i]);
            }
         }
         else
         {
            double* extra_arg = sed_column_load( s , 0 , sed_column_len(s) , NULL );

            for ( i=0 ; i<len ; i++ )
            {
               val += sed_property_measure( f , s->cell[i] , extra_arg[i] )
                    * sed_cell_size(s->cell[i]);
            }
         }
      }
      else
      {
         for ( i=0 ; i<len ; i++ )
         {
            val += sed_property_measure( f , s->cell[i] )
                 * sed_cell_size(s->cell[i]);
         }
      }

      val /= sed_column_thickness(s);

      eh_free( load );
   }

   return val;
}

/** Change the thickness of a cell within a column.

Change the thickness of a cell within a column.  The thickness of the specifed
cell is changed as well as the thickness of the sediment column.  The cell
thickness is changed in such a way so as to retain its degree of compactedness.

@param s             A pointer to a Sed_column.
@param i             Index to a Sed_cell within a Sed_column. 
@param new_t         The new thickness of the Sed_cell.

@return A pointer to the input Sed_column.

@see sed_compact_cell_in_column .
*/
Sed_column sed_column_resize_cell( Sed_column s ,
                                   gssize i ,
                                   double new_t )
{
   eh_require( s );

   if ( s && sed_column_is_get_index(s,i) )
   {
      double old_t = sed_cell_size(s->cell[i]);

      eh_lower_bound( new_t , 0 );

      sed_cell_resize( s->cell[i] , new_t );
      sed_column_set_thickness( s , sed_column_thickness(s) + new_t-old_t );
   }

   return s;
}

/** Compact a cell within a column.

Change the thickness of a cell within a column.  The thickness of the specifed
cell is changed as well as the thickness of the sediment column.  The current
thickness of the cell is changed but its original thickness not changed.  This
will therefore cause the sediment to become more dense.

@param s        A pointer to a Sed_column.
@param i        Index to a Sed_cell within a Sed_column. 
@param new_t    The new thickness of the Sed_cell.

@return         A pointer to the input Sed_column.

@see sed_compact_cell_in_column .
*/
Sed_column sed_column_compact_cell( Sed_column s , gssize i , double new_t )
{
   eh_require( s )

   if ( s && sed_column_is_get_index(s,i) )
   {
      double old_t = sed_cell_size( s->cell[i] );
      sed_cell_compact( s->cell[i] , new_t );
      sed_column_set_thickness( s , sed_column_thickness(s) + new_t - old_t );
   }

   return s;
}

/** Add a Sed_cell to the top of a Sed_column.

This interface should not be used.  Rather, the user should use the short cuts
sed_add_cell_to_column or sed_add_cell_to_column_avg_pressure instead,
depending weather the pressures should be updated or just averaged.

@param col  A pointer to a Sed_column.
@param cell A pointer to a Sed_cell.
@param update_pressure Should the cell pressures be updated or averaged.

@return The amount of sediment that is to be added to the column.  This is
        just the thickness of the input cell.

@see sed_add_cell_to_column , sed_add_cell_to_column_avg_pressure .
*/
double sed_column_add_cell_real( Sed_column col ,
                                 Sed_cell cell  ,
                                 gboolean update_pressure )
{
   double amount_to_add = 0;

   eh_require( col  );
   eh_require( cell );

   if ( col && cell && !sed_cell_is_empty(cell) )
   {
      Sed_cell copy = sed_cell_dup( cell );
      Sed_cell top_cell;
      double free_space, left_to_add;
      double cell_load;

      amount_to_add = sed_cell_size( cell );
      left_to_add   = sed_cell_size( cell );

      if ( update_pressure )
      {
         gssize i;
         gssize len = sed_column_len( col );

         cell_load = sed_cell_load( cell );

         for ( i=0 ; i<len ; i++ )
            sed_cell_set_pressure( col->cell[i] ,
                                   sed_cell_pressure( col->cell[i] )
                                   + cell_load );
      }

      if ( sed_column_is_empty(col) )
      {
         top_cell = sed_column_nth_cell(col,0);
         sed_column_resize( col , 1 );
         col->len++;
      }
      else
         top_cell = sed_column_top_cell(col);

      while ( left_to_add > 0 )
      {
         // Determine how much sediment we need to fill up the next cell.
         free_space = sed_column_z_res( col ) - sed_cell_size( top_cell );
         if ( free_space <= 1e-12 )
         {
            // Add another cell.
            sed_column_resize( col , col->len+1 );
            col->len++;
         }
         else
         {
            if ( free_space >= left_to_add )
               free_space = left_to_add;
            sed_cell_resize(cell,free_space);
            sed_cell_add( top_cell , cell );
            sed_column_set_thickness(col,sed_column_thickness(col)+free_space);

            left_to_add -= free_space;

            if ( update_pressure )
            {
               sed_cell_resize( cell , left_to_add );
               cell_load = sed_cell_load( cell );
               sed_cell_set_pressure( top_cell ,
                                      cell_load
                                      + sed_column_water_pressure( col ) );
            }
         }

         top_cell = sed_column_top_cell(col);
      }
      sed_cell_copy( cell , copy );
      sed_cell_destroy( copy );
   }

   return amount_to_add;
}

double sed_column_append_cell_real( Sed_column col ,
                                    Sed_cell cell  ,
                                    gboolean update_pressure )
{
   double amount_to_add = 0;

   eh_require( col  );
   eh_require( cell );

   if ( col && cell )
   {
      amount_to_add = sed_cell_size( cell );

      sed_column_resize( col , col->len + 1 );
/*
      if ( sed_column_is_empty(col) )
         sed_cell_copy( col->cell[0] , cell );
      else
      {
         sed_cell_copy( col->cell[col->len] , cell );
         col->len += 1;
      }
*/
      sed_cell_copy( col->cell[col->len] , cell );
      col->len += 1;

      sed_column_set_thickness( col , sed_column_thickness(col)+sed_cell_size(cell) );

      if ( update_pressure )
      {
         gssize i;
         gssize len = sed_column_len( col );
         double cell_load = sed_cell_load( cell );

         for ( i=0 ; i<len ; i++ )
            sed_cell_set_pressure( col->cell[i] ,
                                   sed_cell_pressure( col->cell[i] )
                                   + cell_load );
      }

   }

   return amount_to_add;
}

/** Add a Sed_cell to the top of a Sed_column.

The contents of a Sed_cell is added to the top of a Sed_column.  This is the
default mode.  In this case the pressures of the cells of the column are 
updated to reflect the increase in overlying load due to the new sediment.

@param col  A pointer to a Sed_column.
@param cell A pointer to a Sed_cell.

@return The amount of sediment that is to be added to the column.  This is
        just the thickness of the input cell.

@see sed_add_cell_to_column_avg_pressure .
*/
double sed_column_add_cell( Sed_column col , Sed_cell cell  )
{
   return sed_column_add_cell_real( col , cell , FALSE );
}

double sed_column_append_cell( Sed_column col , Sed_cell cell )
{
   return sed_column_append_cell_real( col , cell , FALSE );
}

/** Add a Sed_cell to the top of a Sed_column.

The contents of a Sed_cell is added to the top of a Sed_column.  This is
different from sed_add_cell_to_column: In this case, the pressure of the
top cell is averaged with the new cell.

@param col  A pointer to a Sed_column.
@param cell A pointer to a Sed_cell.

@return The amount of sediment that is to be added to the column.  This is
        just the thickness of the input cell.

@see sed_add_cell_to_column .
*/
double sed_column_add_cell_avg_pressure( Sed_column col , Sed_cell cell  )
{
   return sed_column_add_cell_real( col , cell , FALSE );
}

/** Add a vector of thicknesses to a Sed_column.

Add sediment to a column based on the amounts in an array.  The input array
should be the same length as the number of sediment types used in the sediment
column.  The order of the sediment types is also the same.

@param c   A pointer to a Sed_column.
@param t   An array of sediment thicknesses.

@return The total amount of sediment that was added to the column.
*/
double sed_column_add_vec( Sed_column c , const double* t )
{
   double rtn = 0;

   eh_require( c );
   eh_require( t );

   if ( c && t )
   {
      Sed_cell cell = sed_cell_new( sed_sediment_env_size() );

      sed_cell_add_amount( cell , t );
      rtn = sed_column_add_cell( c , cell );

      sed_cell_destroy( cell );
   }
   return rtn;
}

/** Get the Sed_cell at the top of a column.

@param col A pointer to a Sed_column.

@return A pointer to the Sed_cell at the top of a Sed_column.
*/
Sed_cell sed_column_top_cell( const Sed_column col )
{
   Sed_cell top = NULL;

   if ( !sed_column_is_empty(col) )
      top = col->cell[col->len-1];

   return top;
}

/** Get the n-th cell from a column.

Get a pointer to the Sed_cell that is n cells from the bottom of a Sed_column.

@param col A pointer to a Sed_column.
@param n   The index of the cell.

@return A pointer to the n-th Sed_cell of a Sed_column.

*/
Sed_cell sed_column_nth_cell( const Sed_column col , gssize n )
{
   Sed_cell cell = NULL;

   eh_return_val_if_fail( col , NULL );

   if ( sed_column_is_set_index(col,n) )
      cell = col->cell[n];

   return cell;
}

/** Change the number of cells within a column.

Change the number of cells that are contained within a column.  If the new
size is greater than the current size, more memory is allocated and new
(cleared) Sed_cell's are added to the top of the Sed_column.  If the new size
is smaller, the cells from the smaller size to the current size are cleared.
In this case, no memory is freed.

@param col A pointer to a Sed_column.
@param n   The new size of the Sed_column.
*/
Sed_column sed_column_resize( Sed_column col , gssize n )
{
   eh_require( col );

   {
      gssize i;

      if ( n > col->size )
      {
         // Add bins in blocks of S_ADDBINS
         gssize add_bins = ((n-col->size)/S_ADDBINS+1)*S_ADDBINS;
         gssize new_size = col->size + add_bins;

         if ( col->cell )
         {
            Sed_cell* new_col;
            col->cell = eh_renew( Sed_cell , col->cell , col->size+add_bins );
         }
         else
            col->cell = eh_new( Sed_cell , col->size+add_bins );

         for ( i=col->size ; i<new_size ; i++ )
            col->cell[i] = sed_cell_new( sed_sediment_env_size() );
         col->size += add_bins;
      }
      else
      {
         for ( i=n ; i<col->size ; i++ )
            sed_cell_clear( col->cell[i] );
      }
   }

   return col;
}

/** Remove part of the top cell of a column and save the sediment.

The top fraction (given by f) is removed from the cell at the top of a sediment
column.  The removed sediment is placed into the destination cell.  The 
destination cell is cleared before the new sediment is added.  If a NULL value
is passed as the destination cell, a new cell is created to hold the sediment.

@param col  A pointer to a Sed_column.
@param f    The fraction of the top cell to remove.
@param dest A pointer to a Sed_cell to hold the removed sediment.

@return A pointer to a Sed_cell that holds the removed sediment.

@see sed_column_remove_cell.
*/
Sed_cell sed_column_extract_top_cell( Sed_column col ,
                                      double f       ,
                                      Sed_cell dest )
{
   eh_require( col );
   eh_require( f<=1 );
   eh_require( f>=0 );

   eh_clamp( f , 0 , 1 );

   if ( col && !sed_column_is_empty(col) )
   {
      Sed_cell top_cell = sed_column_top_cell(col);

      dest = sed_cell_copy( dest , top_cell );

      sed_cell_resize( dest , sed_cell_size(dest)*f );
      sed_column_remove_top_cell( col , f );
   }
   else
      dest = NULL;

   return dest;
}

/** Remove part of the top cell of a column.

The top fraction (given by f) is removed from the cell at the top of a sediment
column.  The removed sediment is discarded.

@param col  A pointer to a Sed_column.
@param f    The fraction of the top cell to remove.

@see sed_extract_cell_from_column .
*/
Sed_column sed_column_remove_top_cell( Sed_column col , double f )
{
   eh_require( col );
   eh_require( f<=1 );
   eh_require( f>=0 );

   eh_clamp( f , 0 , 1 );

   if ( col && !sed_column_is_empty(col) )
   {
      Sed_cell top_cell = sed_column_top_cell(col);
      sed_column_set_thickness( col ,
                                  sed_column_thickness(col)
                                - f*sed_cell_size(top_cell) );
      sed_cell_resize( top_cell , sed_cell_size(top_cell)*(1.-f) );

      if ( sed_cell_size(top_cell) < 1e-12 )
      {
         sed_cell_clear( top_cell );
         (col->len)--;
         if ( col->len<0 )
            eh_require_not_reached();
      }
/*
      if ( sed_cell_is_empty( top_cell ) )
      {
         if ( col->len > 1 )
            (col->len)--;
      }
*/
   }

   return col;
}

/** Remove the top of a column and save the sediment.

The top thickness units is removed from the cells at the top of a sediment
column.  The removed sediment is placed into the destination cell.  The 
destination cell is cleared before the new sediment is added.  If a NULL value
is passed as the destination cell, a new cell is created to hold the sediment.

@param col       A pointer to a Sed_column.
@param t         The amount of sediment to remove from the top of the column.
@param dest      A pointer to a Sed_cell to hold the removed sediment.

@return A pointer to a Sed_cell that holds the removed sediment.

@see sed_remove_top_from_column , sed_get_top_from_column.
*/
Sed_cell
sed_column_extract_top( Sed_column col ,
                        double t       ,
                        Sed_cell dest )
{
   return sed_column_extract_top_fill( col , t , NULL , dest );
}

Sed_cell sed_column_extract_top_fill( Sed_column col ,
                                      double t       ,
                                      Sed_cell fill  ,
                                      Sed_cell dest )
{
   eh_require( col );

   if ( !dest )
      dest = sed_cell_new( sed_sediment_env_size() );
   else
      sed_cell_clear( dest );

   if ( fill )
      sed_cell_resize( fill , G_MINDOUBLE );

   if ( t>0 )
   {
      gssize n_grain = sed_sediment_env_size();
      double left_to_remove, available_sediment;
      double f;
      Sed_cell cell_temp, top_cell;
      gboolean more_to_remove = FALSE;
   
      cell_temp = sed_cell_new_env();
      left_to_remove = t;

      if ( left_to_remove>0 )
         more_to_remove = TRUE;

      while ( !sed_column_is_empty(col) && more_to_remove )
      {
         top_cell = sed_column_top_cell(col);
         available_sediment = sed_cell_size( top_cell );
         if ( available_sediment > left_to_remove )
         {
            f = left_to_remove/available_sediment;
            more_to_remove = FALSE;
         }
         else 
            f = 1.0;
         sed_column_extract_top_cell( col , f , cell_temp );
         sed_cell_add( dest , cell_temp );
         left_to_remove -= sed_cell_size( cell_temp );
      }

      if ( fill && fabs( sed_cell_size(dest) - t ) > 1e-12 )
      {
         double dh = t - sed_cell_size(dest);

         if ( dh > 0 )
         {
            eh_require( col->t < 1e-12 );

            if ( !(col->t < 1e-12) )
            {
               eh_watch_dbl( col->t );
               eh_watch_dbl( t );
               eh_watch_dbl( sed_cell_size(dest) );
            }

            sed_cell_resize( fill , dh );
            sed_column_adjust_base_height( col , -dh );
            col->z -= dh;
            sed_cell_add( dest , fill );
         }
      }

      sed_cell_destroy( cell_temp );
   }

   return dest;
}

/** Remove the top of a column.

The top thickness units is removed from the cells at the top of a sediment
column.  The removed sediment is discarded.

@param col       A pointer to a Sed_column.
@param t         The amount of sediment to remove from the top of the column.

@see sed_extract_top_from_column , sed_get_top_from_column .
*/
Sed_column sed_column_remove_top( Sed_column col , double t )
{
   eh_require( col );

   if ( col && t>0 && !sed_column_is_empty(col) )
   {
      Sed_cell top_cell;
      double left_to_remove, available_sediment;
      double f;
      gboolean more_to_remove = FALSE;
   
      left_to_remove = t;

      if ( left_to_remove>0 )
         more_to_remove = TRUE;

      while ( !sed_column_is_empty(col) && more_to_remove )
      {
         top_cell = sed_column_top_cell(col);
         available_sediment = sed_cell_size( top_cell );
         if ( available_sediment >= left_to_remove )
         {
            f = left_to_remove/available_sediment;
            more_to_remove = FALSE;
         }
         else 
            f = 1.0;
         sed_column_remove_top_cell(col,f);
         left_to_remove -= f*available_sediment;
      }
   }

   return col;
}

Sed_column sed_column_remove_top_erode( Sed_column col , double t )
{
   eh_require( col );

   if ( col )
   {
      double erode = t - sed_column_thickness(col);

      sed_column_remove_top( col , t );

      if ( erode>0 )
      {
         col->z -= erode;
      }
   }

   return col;
}

Sed_cell sed_column_separate_top( Sed_column col  ,
                                  double t        ,
                                  double f[]      ,
                                  Sed_cell rem_cell )
{
   Sed_cell lag_cell = sed_cell_new( sed_sediment_env_size() );

   sed_column_extract_top    ( col      , t        , lag_cell );
   sed_cell_separate_fraction( lag_cell , f        , rem_cell );
   sed_column_add_cell       ( col      , lag_cell            );
   sed_cell_destroy          ( lag_cell                       );

   return rem_cell;
}

Sed_cell sed_column_separate_top_amounts( Sed_column col ,
                                          double total_t ,
                                          double t[]     ,
                                          Sed_cell rem_cell )
{
   Sed_cell lag_cell = sed_cell_new( sed_sediment_env_size() );

   sed_column_extract_top  ( col      , total_t  , lag_cell );
   sed_cell_separate_amount( lag_cell , t        , rem_cell );
   sed_column_add_cell     ( col      , lag_cell            );
   sed_cell_destroy        ( lag_cell );

   return rem_cell;
}

Sed_cell sed_column_separate_top_amounts_fill( Sed_column col ,
                                               double total_t ,
                                               double t[]     ,
                                               Sed_cell fill  ,
                                               Sed_cell rem_cell )
{
   Sed_cell lag_cell;
//   double m_0 = sed_column_mass(col);
//   double t_0 = sed_column_thickness(col);
//   gssize len_0 = col->len;
//   double m_1, t_1;

   lag_cell = sed_cell_new_env( );
   sed_column_extract_top_fill( col , total_t , fill , lag_cell );
   sed_cell_separate_amount( lag_cell , t , rem_cell );
   sed_column_add_cell( col , lag_cell );
   sed_cell_destroy( lag_cell );
/*
   m_1 = sed_column_mass(col);
//   if ( fill && fabs(m_1+sed_cell_mass(rem_cell)-(m_0+sed_cell_mass(fill)) )/m_0 > 1e-5 )
   if ( fabs(m_1+sed_cell_mass(rem_cell)-m_0 )/m_0 > 1e-5 )
{
   t_1 = sed_column_thickness(col);
eh_watch_int( len_0 );
eh_watch_int( col->len );
eh_watch_dbl( t_0 );
eh_watch_dbl( t_1 );
eh_watch_dbl( m_0 );
eh_watch_dbl( m_1 );
eh_watch_dbl( sed_cell_mass(rem_cell) );
//eh_watch_dbl( sed_cell_mass(fill) );
eh_watch_dbl( total_t );
eh_dbl_array_fprint( stderr , t , sed_sediment_env_size() );
      exit(0);
}
*/
   return rem_cell;
}

/** Get the top of a column.

The top thickness units is copied from the cells at the top of a sediment
column.  A copy of the sediment is placed into the destination cell.  The 
destination cell is cleared before the new sediment is added.  If a NULL value
is passed as the destination cell, a new cell is created to hold the sediment.
No sediment is removed from the Sed_column.

\param col  A pointer to a Sed_column.
\param t    The amount of sediment to copy from the top of the column.
\param dest A pointer to a Sed_cell to hold the copied sediment.

\return A pointer to a Sed_cell that holds the copied sediment.

\see sed_remove_top_from_column , sed_extract_top_from_column .
*/
Sed_cell sed_column_top( const Sed_column col ,
                         double t             ,
                         Sed_cell dest )
{
   eh_return_val_if_fail( col , NULL );

   if ( !dest )
      dest = sed_cell_new( sed_sediment_env_size() );
   sed_cell_clear( dest );

   if ( !sed_column_is_empty(col) )
   {
      Sed_cell next_cell;
      double left_to_get, available_sediment;
      int next_ind;

      left_to_get = t;

      next_ind = sed_column_len( col )-1;
      while ( left_to_get > 1e-12 && next_ind >= 0 )
      {
         next_cell = sed_column_nth_cell( col , next_ind );
         available_sediment = sed_cell_size( next_cell );

         if ( available_sediment > left_to_get )
         {
            sed_cell_resize( next_cell , left_to_get        );
            sed_cell_add   ( dest      , next_cell          );
            sed_cell_resize( next_cell , available_sediment );

            left_to_get = 0.;
         }
         else
         {
            sed_cell_add( dest , next_cell );
            left_to_get -= available_sediment;
         }
         next_ind--;
      }
   }

   return dest;
}

/** Get a bulk property from the top of a column.

This is the same as sed_get_top_property_with_load except that here we use
Sed_property_func.

@param property A function to get a property from a Sed_cell.
@param s        A pointer to a Sed_column.
@param top      The depth of sediment to query.

@return The bulk property value.

@see sed_get_top_property_with_load .
*/
double sed_column_top_property_0( Sed_property property ,
                                const Sed_column s    ,
                                double top )
{
   double val = 0;

   eh_require( s );

   if ( s )
   {
      Sed_cell avg;

      avg = sed_cell_new( sed_sediment_env_size() );

      avg = sed_column_top( s , top , avg );
      val = sed_property_measure( property , avg );

      sed_cell_destroy( avg );
   }

   return val;
}

/** Get a bulk property from the top of a column.

This is the same as sed_get_top_property except that here we use
Sed_property_func_with_load.  As we examine only the top portion of a column,
in this case the load is that of the sediment we are examining.

@param property A function to get a property from a Sed_cell.
@param s        A pointer to a Sed_column.
@param top      The depth of sediment to query.

@return The bulk property value.

@see sed_get_top_property .
*/
double sed_column_top_property( Sed_property property , const Sed_column s , double top )
{
   double val;

   eh_return_val_if_fail( s        , 0 );
   eh_return_val_if_fail( property , 0 );

   {
      Sed_cell avg = sed_column_top( s , top , NULL );

      if ( sed_property_n_args(property)==2 )
      {
         double extra_arg;
         if (    sed_property_is_named( property , "consolidation" )
              || sed_property_is_named( property , "consolidation rate" ) )
            extra_arg = sed_column_age( s );
         else
            extra_arg = sed_cell_load( avg );
         val  = sed_property_measure( property , avg , extra_arg );
      }
      else
         val  = sed_property_measure( property , avg );


      sed_cell_destroy( avg );
   }

   return val;
}

/** Get the density of the top of a column.

Get the bulk density of the top (units of depth) of a column.  This function
is a convenience function for sed_get_top_property (S_DENSITY,s,top) .

@param s   A pointer to a Sed_column.
@param top The depth of sediment to query.

@return The bulk density of the top of a column.
*/
double sed_column_top_rho( const Sed_column s , double top )
{
   double rho = 0.;

   eh_return_val_if_fail( s , 0. );

   {
      Sed_cell avg;

      avg = sed_cell_new( sed_sediment_env_size() );

      avg = sed_column_top(s,top,avg);
      rho = sed_cell_density( avg );

      sed_cell_destroy(avg);
   }

   return rho;
}

/** Get the age of the top of a column.

Get the age of the top (units of depth) of a column.  This function
is essentially a convenience function for sed_get_top_property (S_AGE,s,top) .

@param s   A pointer to a Sed_column.
@param top The depth of sediment to query.

@return The age of the top of a column.
*/
double sed_column_top_age( const Sed_column s , double top )
{
   double age = 0;

   eh_require( s );

   if ( s )
   {
      Sed_cell avg;

      avg = sed_cell_new( sed_sediment_env_size() );
      sed_column_top(s,top,avg);
      age = sed_cell_age(avg);
      sed_cell_destroy(avg);
   }

   return age;
}

/** Get the number of cells contained within the top of a column.

Return the number of sediment bins that make up the top height-th units of
a sediment column.  The bins may be of a size that is different from the 
vertical resolution of the column.

@param s      A pointer to a Sed_column.
@param z      The thickness of a Sed_column top.

@return The number of sediment bins within the top of a Sed_column.
*/
gssize sed_column_top_nbins( Sed_column s , double z )
{
   gssize n_bins = 0;

   eh_require( s );

   if ( s && !sed_column_is_empty(s) )
   {
      double t       = z - sed_column_base_height( s );

      if ( t>0 )
      {
         gssize ind_bot = sed_column_index_thickness( s , t );
         gssize ind_top = s->len;
         n_bins = ind_top - ind_bot;
      }
      else
         n_bins = s->len;
   }

   return n_bins;
}

/** Get the index of a cell at a specified elevation within a column.

@param s    A pointer to a Sed_column.
@param z    The elevation of a Sed_cell within a Sed_column.

@return     The index to a cell at a given elevation.
*/
gssize sed_column_index_at( const Sed_column s , double z )
{
   eh_return_val_if_fail( s , -1 );

   return sed_column_index_thickness( s , z - sed_column_base_height(s) );
}

/** Write the contents of Sed_column to a binary file.

@param fp A pointer to an open file.
@param s  A pointer to a Sed_column.

@see sed_load_column .
*/
gssize sed_column_write( FILE* fp , const Sed_column s )
{
   return sed_column_write_to_byte_order( fp , s , G_BYTE_ORDER );
}

gssize sed_column_write_to_byte_order( FILE* fp , const Sed_column s , gint order )
{
   gssize n = 0;

   if ( s && fp )
   {
      if ( order == G_BYTE_ORDER )
      {
         gssize i;

         n += fwrite( &(s->z)    , sizeof(double) , 1 , fp );
         n += fwrite( &(s->t)    , sizeof(double) , 1 , fp );
         n += fwrite( &(s->len)  , sizeof(gssize) , 1 , fp );
         n += fwrite( &(s->size) , sizeof(gssize) , 1 , fp );
         n += fwrite( &(s->dz)   , sizeof(double) , 1 , fp );
         n += fwrite( &(s->x)    , sizeof(double) , 1 , fp );
         n += fwrite( &(s->y)    , sizeof(double) , 1 , fp );
         n += fwrite( &(s->age)  , sizeof(double) , 1 , fp );
         n += fwrite( &(s->sl)   , sizeof(double) , 1 , fp );

         for ( i=0 ; i<s->size ; i++ )
            n += sed_cell_write( fp , s->cell[i] );
      }
      else
      {
         gssize i;

         n += eh_fwrite_dbl_swap  ( &(s->z)    , sizeof(double) , 1 , fp );
         n += eh_fwrite_dbl_swap  ( &(s->t)    , sizeof(double) , 1 , fp );
         n += eh_fwrite_int32_swap( &(s->len)  , sizeof(gssize) , 1 , fp );
         n += eh_fwrite_int32_swap( &(s->size) , sizeof(gssize) , 1 , fp );
         n += eh_fwrite_dbl_swap  ( &(s->dz)   , sizeof(double) , 1 , fp );
         n += eh_fwrite_dbl_swap  ( &(s->x)    , sizeof(double) , 1 , fp );
         n += eh_fwrite_dbl_swap  ( &(s->y)    , sizeof(double) , 1 , fp );
         n += eh_fwrite_dbl_swap  ( &(s->age)  , sizeof(double) , 1 , fp );
         n += eh_fwrite_dbl_swap  ( &(s->sl)   , sizeof(double) , 1 , fp );

         for ( i=0 ; i<s->size ; i++ )
            n += sed_cell_write_to_byte_order( fp , s->cell[i] , order );
      }
   }

   return n;
}

/** Read Sed_column information from a binary file.

@param fp  A pointer to an open file.

@return A pointer to the loaded Sed_column.
*/
Sed_column sed_column_read( FILE* fp )
{
   Sed_column s = NULL;

   eh_require( fp );

   if ( fp )
   {
      gssize i;

      NEW_OBJECT( Sed_column , s );

      fread( &(s->z)    , sizeof(double) , 1 , fp );
      fread( &(s->t)    , sizeof(double) , 1 , fp );
      fread( &(s->len)  , sizeof(gssize) , 1 , fp );
      fread( &(s->size) , sizeof(gssize) , 1 , fp );
      fread( &(s->dz)   , sizeof(double) , 1 , fp );
      fread( &(s->x)    , sizeof(double) , 1 , fp );
      fread( &(s->y)    , sizeof(double) , 1 , fp );
      fread( &(s->age)  , sizeof(double) , 1 , fp );
      fread( &(s->sl)   , sizeof(double) , 1 , fp );

      s->cell = eh_new( Sed_cell , s->size );
      for ( i=0 ; i<s->size ; i++ )
         s->cell[i] = sed_cell_read( fp );
   }

   return s;
}

/** Get a column from a portion of another.

Get a portion of one column from another.  The copy begins at an elevation,
height from the source column.  No sediment is removed from the source column.
Information is only copied.  If NULL is passed for the destination column, a
new column is created.

@param src    A pointer to the source Sed_column.
@param z      The elevation to start the copy.
@param dest   A pointer to the destination Sed_column.

@return A pointer to the destination column.

*/
Sed_column sed_column_height_copy( const Sed_column src ,
                                   double z             ,
                                   Sed_column dest )
{
   eh_return_val_if_fail( src , NULL );

   {
      gssize bins_to_extract, start;
      double t       = z - sed_column_base_height(src);

      start = sed_column_index_thickness( src , t );
      bins_to_extract = sed_column_len(src) - start;

      if ( !dest )
         dest = sed_column_new( 1 );
      sed_column_copy_public_data( dest , src );

      sed_column_set_base_height( dest , z );

      if ( bins_to_extract > 0 )
      {
         gssize i;
         double dh;

         dh = sed_column_thickness_index( src , start )
            - ( z - sed_column_base_height(src) );

         if ( dh>0 )
         {
            sed_column_append_cell( dest , src->cell[start] );
            sed_cell_resize( dest->cell[0] , dh );
         }

         // Add the cells to be extracted.
         for ( i=1 ; i<bins_to_extract ; i++ )
            sed_column_append_cell( dest , src->cell[start+i] );

      }
   }

   return dest;
}

/** Bite off the begining of a column.

Remove the bottom portion of a column starting at an elevation, bottom.

@param col    A pointer to a Sed_column.
@param bottom The elevation where the bite will end.

@see sed_chop_column , sed_strip_column .
*/
Sed_column sed_column_chomp( Sed_column col , double bottom )
{
   eh_return_val_if_fail( col , NULL );

   if ( bottom > sed_column_base_height(col) )
   {
      Sed_column new_col = sed_column_height_copy( col , bottom , NULL );
      sed_column_copy( col , new_col );
      sed_column_destroy( new_col );
   }

   return col;
}

/** Cut off the last portion of a column.

Cut off the top part of a column starting at an elevation, top.

@param col A pointer to a Sed_column.
@param top The elevation where the cut is to be made.

@see sed_chomp_column , sed_strip_column .
*/
Sed_column sed_column_chop( Sed_column col , double top )
{
   eh_return_val_if_fail( col , NULL );

   if ( top < sed_column_top_height(col) )
   {
      double top_t = sed_column_top_height( col ) - top;

      sed_column_remove_top( col , top_t );

      if ( top < sed_column_base_height( col ) )
         sed_column_set_base_height( col , top );
   }

   return col;
}

/** Remove the begining and end of a column.

Remove the cells of a column starting at the bottom and going to an elevation,
bottom.  Also, remove the cells starting at an elevation, top and running to
the top of the column.  That is, keep only the cells between bottom and top.

@param col    A pointer to a Sed_column.
@param bottom Elevation where the chomp will end.
@param top    Elevation where the chop will begin.

@see sed_chomp_column , sed_chop_column .
*/
Sed_column sed_column_strip( Sed_column col , double bottom , double top )
{
   return sed_column_chop( sed_column_chomp( col , bottom ) , top );
}

/** Get the depth to the top of a column.

Get the depth to the top of a Sed_column.

@param col A pointer to a Sed_column

@return The depth to the top of a column.

@see sed_get_depth_from_cube .
*/
double sed_column_water_depth( const Sed_column col )
{
   return sed_column_sea_level(col) - sed_column_top_height(col);
}

/** Get the number of filled (or partially filled) cells within a column.

@param col A pointer to a Sed_column.

@return The number of filled (or partially filled) cells in a Sed_column
*/
int sed_get_column_size(const Sed_column col)
{
   return col->len;
}

gssize sed_column_len( const Sed_column col )
{
   eh_return_val_if_fail( col , 0 );
   return col->len;
}

gboolean sed_column_is_empty( const Sed_column col )
{
   eh_return_val_if_fail( col , TRUE );
   return col->len==0;
}

/** Is the index in bounds.
*/
gboolean sed_column_is_valid_index( const Sed_column c , gssize n )
{
   eh_return_val_if_fail( c , FALSE );
   return n>=0 && n<c->size;
}

/** Is the index to a cell that can be read.
*/
gboolean sed_column_is_get_index( const Sed_column c , gssize n )
{
   eh_return_val_if_fail( c , FALSE );
   return n>=0 && n<c->len;
}

/** Is the index to a cell that can be added to.
*/
gboolean sed_column_is_set_index( const Sed_column c , gssize n )
{
   eh_return_val_if_fail( c , FALSE );
   return n>=0 && n<=c->len;
}

/** Get the index to the top cell in a Sed_column.

@param col A pointer to a Sed_column.

@return The index of the top cell in a Sed_column.
*/
gssize sed_column_top_index( const Sed_column col )
{
   eh_return_val_if_fail( col , -1 );
   return col->len-1;
}

/** Get the sediment thickness of a Sed_column.

@param col A pointer to a Sed_column.

@return The thickness of sediment housed in a Sed_column.
*/
double sed_column_thickness( const Sed_column col)
{
   eh_return_val_if_fail( col , 0 );
   return col->t;
}

/** Set the thickness of sediment in a Sed_column.

@param col           A pointer to a Sed_column.
@param new_t         The new thickness of a Sed_column.

@return A pointer to the input Sed_column.
*/
Sed_column sed_column_set_thickness( Sed_column col , double new_t )
{
   col->t = new_t;
   return col;
}

gboolean sed_column_size_is( const Sed_column col , double t )

{
   return eh_compare_dbl( col->t , t , 1e-12 );
}

gboolean sed_column_mass_is( const Sed_column c , double m )
{
   return eh_compare_dbl( sed_column_mass(c) , m , 1e-12 );
}

gboolean sed_column_sediment_mass_is( const Sed_column c , double m )
{
   return eh_compare_dbl( sed_column_sediment_mass(c) , m , 1e-12 );
}

gboolean sed_column_base_height_is( const Sed_column c , double z )
{
   return eh_compare_dbl( sed_column_base_height(c) , z , 1e-12 );
}

gboolean sed_column_top_height_is( const Sed_column c , double z )
{
   return eh_compare_dbl( sed_column_top_height(c) , z , 1e-12 );
}

/** Get the thickness of sediment up to (and including) a cell.

@param col A pointer to a Sed_column.
@param ind Index to a cell within a Sed_column.

@return The total sediment thickness from the bottom of a column up to (and
        including) the ind-th cell.
*/
double sed_column_thickness_index( const Sed_column col , gssize ind )
{
   double t=0;

   eh_return_val_if_fail( col , 0 );

   {
      gssize i;
      gssize top_ind = ind+1;

      eh_clamp( top_ind , 0 , sed_column_len(col) );

      for ( i=0 ; i<top_ind ; i++ )
         t += sed_cell_size( col->cell[i] );
   }

   return t;
}

/** Get the burial depth to sediment of a specified age.

@param col A pointer to a Sed_column.
@param age The age of the buried sediment.

@return The depth from the top of a Sed_column to sediment of the specified
        age.
*/
double sed_column_depth_age( const Sed_column col , double age )
{
   double d = 0;

   eh_require( col );

   if ( col )
   {
      gssize i;
      for ( i=col->len-1 ; i>=0 && sed_cell_age(col->cell[i])>age ; i--)
         d += sed_cell_size( col->cell[i] );
   }
   
   return d;
}

/** Get the index to the cell that is a specified height above a column bottom.

@param col      A pointer to a Sed_column.
@param t        Sediment thickness to a Sed_cell within a Sed_column.

@return The index to the Sed_cell that is a specifed distance from the bottom
        of a Sed_column.
*/
gssize sed_column_index_thickness( const Sed_column col , double t )
{
   gssize i = -1;

   eh_return_val_if_fail( col , -1 );

   if ( t>sed_column_thickness(col)*.5 )
      i = sed_column_index_depth( col , sed_column_thickness(col)-t );
   else
   {
      double total_t = 0;

      eh_lower_bound( t , 0 );

      for ( i=0 ; total_t<t && i<col->len ; i++)
         total_t += sed_cell_size( col->cell[i] );

      i -= 1;
   }

   return i;
}

/** Get the index to a cell that is at a specified burial depth.

@param col   A pointer to a Sed_column.
@param d     Burial depth of a Sed_cell.

@return The index to a Sed_cell that is buried at a specified depth.
*/
gssize sed_column_index_depth( const Sed_column col , double d )
{
   gssize i;

   eh_return_val_if_fail( col , -1 );

   if ( d>=sed_column_thickness(col)*.5 )
      i = sed_column_index_thickness( col , sed_column_thickness(col)-d );
   else
   {
      double total_d = 0;

      eh_lower_bound( d , 0 );

      for ( i=col->len-1 ; total_d<=d && i>=0 ; i--)
         total_d += sed_cell_size( col->cell[i] );

      i += 1;
   }

   return i;
}

/** Add the sediment from a portion of a column to a cell.

Sediment is added from the source column to the destination cell.  The
destination cell is not cleared before the addition of the new sediment.

@param dest A pointer to the destination Sed_cell.
@param src  A pointer to the source Sed_column.

@return A pointer the the destination Sed_cell.
*/
Sed_cell sed_cell_add_column( Sed_cell dest , const Sed_column src )
{
   eh_return_val_if_fail(src,NULL);

   {
      gssize i;

      if ( !dest )
         dest = sed_cell_new( sed_sediment_env_size() );

      for ( i=0 ; i<src->len ; i++ )
         sed_cell_add( dest , src->cell[i] );
   }

   return dest;
}

/** Add the contents of one column to the top of another.

The contents of one column is added to the top of another.  No sediment is
removed from the source column.  The sediment is copied and then added to the
destination column.

@param dest A pointer to the destination Sed_column.
@param src  A pointer to the source Sed_column.

@return A pointer to the destination Sed_column.
*/
Sed_column sed_column_add( Sed_column dest , const Sed_column src )
{
   eh_return_val_if_fail( src , NULL );

   {
      gssize i;
      if ( !dest )
         dest = sed_column_new( src->size );
      for ( i=0 ; i<src->len ; i++ )
         sed_column_add_cell( dest , src->cell[i] );
   }

   return dest;
}

Sed_column sed_column_append( Sed_column dest , const Sed_column src )
{
   eh_require( src );

   if ( src )
   {
      gssize i;
      for ( i=0 ; i<src->len ; i++ )
         sed_column_append_cell( dest , src->cell[i] );
   }
   else
      dest = NULL;

   return dest;
}

Sed_column sed_column_remove( Sed_column s1 , const Sed_column s2 )
{
   eh_return_val_if_fail( s1 , NULL );
   eh_return_val_if_fail( s2 , NULL );

   {
      double d = sed_column_top_height(s1) - sed_column_base_height( s2 );

      if ( d > 0 )
      {
         sed_column_remove_top( s1 , d );
         if ( sed_column_is_empty(s1) )
            sed_column_set_base_height(s1,sed_column_base_height(s2));
      }
   }

   return s1;
}

/** Rebin the cells of a column to their original size.

@param col A pointer to a Sed_column.

@return A pointer to the input Sed_column.
*/
Sed_column sed_column_rebin( Sed_column col )
{
   eh_require( col );

   if ( col )
   {
      gssize i;
      Sed_column col_temp;

      // Create a temporary sediment column that is a copy of the old one.
      col_temp = sed_column_new( col->size );
      sed_column_copy( col_temp , col );

      // Remove all of the sediment from the old one.
      sed_column_clear( col );
   
      // Add each cell from the temporary sediment column back to the
      // new one so that the cells will be rebinned with the proper cell
      // heights.
      for ( i=0 ; i<sed_column_len(col_temp) ; i++ )
         sed_column_add_cell_avg_pressure( col , col_temp->cell[i] ); 
   
      // Destroy the temporary sediment column.
      sed_column_destroy( col_temp );
   }

   return col;
}

