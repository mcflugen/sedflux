#include "utils/utils.h"
#include <check.h>

#include "sed_column.h"

START_TEST ( test_sed_column_new )
{
   {
      Sed_column c = sed_column_new( 5 );

      fail_if    ( c==NULL                             , "NULL is not a valid column" );
      fail_unless( sed_column_len(c)==0                , "A new column is empty" );
      fail_unless( fabs(sed_column_thickness(c))<1e-12 , "A new column has no thickness" );
      fail_unless( fabs(sed_column_base_height(c))<=0  , "Initial elevation is 0" );
      fail_unless( fabs(sed_column_z_res(c)-1)<=0      , "Default resolution is 1" );
      fail_unless( fabs(sed_column_x_position(c))<=0   , "Initial x-position is 1" );
      fail_unless( fabs(sed_column_y_position(c))<=0   , "Initial y-position is 1" );

      sed_column_destroy( c );
   }
}
END_TEST

START_TEST ( test_sed_column_new_neg )
{
   Sed_column c = sed_column_new( -1 );

   fail_unless( c==NULL , "A column of negative length is not allowed" );
}
END_TEST

START_TEST ( test_sed_column_new_zero )
{
   Sed_column c = sed_column_new( 0 );

   fail_unless( c==NULL , "A column of zero length is not allowed" );

}
END_TEST

START_TEST ( test_sed_column_destroy )
{
   Sed_column c = sed_column_new( 5 );

   c = sed_column_destroy(c);

   fail_unless( c==NULL , "Destroyed column should be NULL" );
}
END_TEST

START_TEST ( test_sed_column_copy )
{
   Sed_column c_1 = sed_column_new( 5 );
   Sed_column c_2 = sed_column_new( 55 );
   Sed_column c_3;

   sed_column_set_base_height( c_1 , 16 );
   sed_column_set_z_res      ( c_1 , 42 );

   c_3 = sed_column_copy( c_2 , c_1 );

   fail_if    ( c_1==c_2                         , "A copy of the column should be made" );
   fail_unless( c_2==c_3                         , "The copy should not be a new column" );
   fail_unless( sed_column_is_same_data(c_1,c_2) , "Column data not copied properly"     );
   fail_unless( sed_column_is_same(c_1,c_2)      , "Column cells not copied properly"    );
   
   sed_column_destroy( c_1 );
   sed_column_destroy( c_2 );
}
END_TEST

START_TEST ( test_sed_column_copy_null )
{
   Sed_column c_1 = sed_column_new( 5 );
   Sed_column c_2;

   sed_column_set_base_height( c_1 , 16 );
   sed_column_set_z_res      ( c_1 , 42 );

   c_2 = sed_column_copy( NULL , c_1 );

   fail_if    ( c_1==NULL                        , "NULL destination should duplicate" );
   fail_if    ( c_1==c_2                         , "A copy of the column should be made" );
   fail_unless( sed_column_is_same_data(c_1,c_2) , "Column data not copied properly" );
   fail_unless( sed_column_is_same(c_1,c_2)      , "Column cells not copied properly" );
   
   sed_column_destroy( c_1 );
   sed_column_destroy( c_2 );
}
END_TEST

START_TEST ( test_sed_column_clear )
{
   Sed_column c_0;
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 27.2 , S_SED_TYPE_MUD );

   sed_column_set_z_res      ( c , 5 );
   sed_column_set_x_position ( c , 5 );
   sed_column_set_y_position ( c , 5 );
   sed_column_set_base_height( c , 5 );

   sed_column_add_cell( c , cell );

   c_0 = sed_column_clear( c );

   fail_unless( c_0==c , "Cleared column should be returned" );
   fail_unless( sed_column_is_empty(c) , "Cleared column should be empty" );
   fail_unless( sed_column_mass_is(c,0) , "Cleared column should have no mass" );
   fail_unless( sed_column_len(c)==0    , "Cleared column length should be zero" );

   if (    fabs( sed_column_z_res(c)      -5 ) > 1e-12
        || fabs( sed_column_x_position(c) -5 ) > 1e-12
        || fabs( sed_column_y_position(c) -5 ) > 1e-12
        || fabs( sed_column_base_height(c)-5 ) > 1e-12 )
      fail( "Cleared column data should not change" );

   sed_column_destroy( c );
   sed_cell_destroy( cell );
}
END_TEST

START_TEST ( test_sed_column_rebin )
{
   Sed_column c  = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 150.5 , S_SED_TYPE_SAND );
   Sed_column c_0;
   double mass_0, mass_1;

   sed_column_stack_cell( c , cell );

   mass_0 = sed_column_mass ( c );
   c_0    = sed_column_rebin( c );
   mass_1 = sed_column_mass ( c );

   fail_unless( c_0==c                    , "Rebin returns the rebinned column" );
   fail_unless( fabs(mass_0-mass_1)<1e-12 , "Rebinned column should have same mass" );
   fail_unless( sed_column_len(c)==151    , "Rebinned column has wrong length" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_destroy_null )
{
   Sed_column c = sed_column_destroy( NULL );

   fail_unless( c==NULL , "Destroyed column should be NULL" );
}
END_TEST

START_TEST ( test_sed_column_set_height )
{
   Sed_column c = sed_column_new( 5 );
   Sed_column c_0;

   c_0 = sed_column_set_base_height( c , 33 );

   fail_unless( c==c_0                                       , "Source column not returned in set function" );
   fail_unless( fabs( sed_column_base_height(c)-33 ) < 1e-12 , "Column height not set properly" );

   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_height )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell s = sed_cell_new_classed( NULL , 10. , S_SED_TYPE_SAND );
   double top, base;

   sed_column_set_base_height( c , 142 );
   sed_column_add_cell( c , s );

   top  = sed_column_top_height (c);
   base = sed_column_base_height(c);
   fail_unless( fabs( top-152  ) < 1e-12 , "Top height not returned correctly" );
   fail_unless( fabs( base-142 ) < 1e-12 , "Base height not returned correctly" );

   sed_cell_destroy( s );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_height_empty )
{
   Sed_column c = sed_column_new( 5 );
   double top, base;

   sed_column_set_base_height( c , 15 );

   top  = sed_column_top_height ( c );
   base = sed_column_base_height( c );

   fail_unless( fabs( base - 15. ) < 1e-12 , "Incorrect base height for empty column returned" );
   fail_unless( fabs( base - top ) < 1e-12 , "For empty column, top height is the same as base height" );
}
END_TEST

START_TEST ( test_sed_column_height_null )
{
   double top  = sed_column_top_height( NULL );
   double base = sed_column_base_height( NULL );

   fail_unless( fabs( top  ) < 1e-12 , "Height to the top of a NULL column is 0" );
   fail_unless( fabs( base ) < 1e-12 , "Height to the base of a NULL column is 0" );
}
END_TEST

START_TEST ( test_sed_column_set_x_position )
{
   Sed_column c = sed_column_new( 5 );
   Sed_column c_0;

   c_0 = sed_column_set_x_position( c , 3.14 );

   fail_unless( c==c_0                                        , "Source column not returned in set function" );
   fail_unless( fabs( sed_column_x_position(c)-3.14 ) < 1e-12 , "Column x-position not set properly" );

   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_set_y_position )
{
   Sed_column c = sed_column_new( 5 );
   Sed_column c_0;

   c_0 = sed_column_set_y_position( c , 2.78 );

   fail_unless( c==c_0                                        , "Source column not returned in set function" );
   fail_unless( fabs( sed_column_y_position(c)-2.78 ) < 1e-12 , "Column y-position not set properly" );

   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_set_z_res )
{
   Sed_column c = sed_column_new( 5 );
   Sed_column c_0;

   c_0 = sed_column_set_z_res( c , .707 );

   fail_unless( c==c_0                                   , "Source column not returned in set function" );
   fail_unless( fabs( sed_column_z_res(c)-.707 ) < 1e-12 , "Column z resolution not set properly" );

   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_stack_cells_loc )
{
   Sed_cell*  c_arr   = NULL;
   Sed_column c       = sed_column_new( 5 );
   double     mass_in = 0;
   gint       i;

   for ( i=0 ; i<10 ; i++ )
   {
      eh_strv_append( &c_arr , sed_cell_new_classed( NULL , 1. , S_SED_TYPE_SAND ) );
      mass_in += sed_cell_mass( c_arr[i] );
   }

   sed_column_stack_cells_loc( c , c_arr );

   fail_unless( sed_column_len(c)==g_strv_length(c_arr)                     , "Column not resized correctly" );
   fail_unless( eh_compare_dbl( sed_column_mass(c)      , mass_in , 1e-12 ) , "Column mass should match cell mass" );
   fail_unless( eh_compare_dbl( sed_column_thickness(c) , 10.     , 1e-12 ) , "Column thickness should match cell thickness" );

   for ( i=0 ; i<10 ; i++ )
      fail_if( sed_column_nth_cell(c,i)!=c_arr[i] , "Sed_cell locations should be set" );

   eh_free( c_arr );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_add_cell )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell s = sed_cell_new_classed( NULL , 1. , S_SED_TYPE_SAND );
   double mass_in = sed_cell_mass( s );
   double t;

   t = sed_column_add_cell( c , s );

   fail_unless( fabs( t - 1. ) < 1e-12                     , "Added thickness should be returned" );
   fail_unless( sed_column_len(c)==1                       , "Column not resized correctly" );
   fail_unless( fabs( sed_cell_mass(s)  -mass_in ) < 1e-12 , "Cell should not change mass" );
   fail_unless( fabs( sed_column_mass(c)-mass_in ) < 1e-12 , "Column mass should match cell mass" );
   fail_unless( fabs( sed_column_thickness(c)-1. ) < 1e-12 , "Column thickness should match cell thickness" );

   sed_cell_resize( s , 128 );
   mass_in += sed_cell_mass( s );
   t = sed_column_add_cell( c , s );
   
   fail_unless( fabs( t - 128. ) < 1e-12                   , "Added thickness should be returned" );
   fail_unless( sed_column_len(c)==129                     , "Column not resized correctly" );
   fail_unless( fabs( sed_column_mass(c)-mass_in ) < 1e-12 , "Column mass should match cell mass" );
   fail_unless( fabs( sed_column_thickness(c)-129) < 1e-12 , "Column thickness should match added thickness" );

   sed_cell_destroy( s );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_add_cell_empty )
{
   double t;
   Sed_column c   = sed_column_new( 5 );
   Sed_cell s     = sed_cell_new_classed( NULL , 2. , S_SED_TYPE_SAND );
   double mass_in = sed_cell_mass( s );

   t = sed_column_add_cell( c , s );
   sed_cell_resize( s , 0 );
   t = sed_column_add_cell( c , s );

   fail_unless( fabs( t ) < 1e-12                          , "Added thickness should be returned" );
   fail_unless( sed_column_len(c)==2                       , "Column not resized correctly" );
   fail_unless( fabs( sed_column_mass(c)-mass_in ) < 1e-12 , "Column mass should match cell mass" );
   fail_unless( sed_column_size_is(c,2.0)                  , "Column thickness should match cell thickness" );

   sed_cell_destroy( s );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_add_cell_small )
{
   double t;
   gssize i;
   Sed_column c   = sed_column_new( 5 );
   Sed_cell s     = sed_cell_new_classed( NULL , .25 , S_SED_TYPE_SAND );
   double mass_in = sed_cell_mass( s );

   t = sed_column_add_cell( c , s );

   fail_unless( fabs( t - .25 ) < 1e-12                     , "Added thickness should be returned" );
   fail_unless( sed_column_len(c)==1                       , "Column not resized correctly" );
   fail_unless( fabs( sed_cell_mass(s)  -mass_in ) < 1e-12 , "Cell should not change mass" );
   fail_unless( fabs( sed_column_mass(c)-mass_in ) < 1e-12 , "Column mass should match cell mass" );
   fail_unless( fabs( sed_column_thickness(c)-.25) < 1e-12 , "Column thickness should match cell thickness" );

   sed_cell_resize( s , .030 );
   for ( i=0,t=0 ; i<1000 ; i++ )
   {
      mass_in += sed_cell_mass( s );
      t       += sed_column_add_cell( c , s );
   }
   
   fail_unless( fabs( t - 30. ) < 1e-12                      , "Added thickness should be returned" );
   fail_unless( sed_column_len(c)==31                        , "Column not resized correctly" );
   fail_unless( eh_compare_dbl( sed_column_mass(c),mass_in,1e-12 ) , "Column mass should match cell mass" );
   fail_unless( fabs( sed_column_thickness(c)-30.25) < 1e-12 , "Column thickness should match added thickness" );

   sed_cell_destroy( s );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_add_cell_large )
{
   double t;
   gssize i;
   Sed_column c   = sed_column_new( 5 );
   Sed_cell s     = sed_cell_new_classed( NULL , 25 , S_SED_TYPE_SAND );
   double mass_in = sed_cell_mass( s );

   t = sed_column_add_cell( c , s );

   fail_unless( fabs( t - 25 ) < 1e-12                     , "Added thickness should be returned" );
   fail_unless( sed_column_len(c)==25                       , "Column not resized correctly" );
   fail_unless( fabs( sed_cell_mass(s)  -mass_in ) < 1e-12 , "Cell should not change mass" );
   fail_unless( fabs( sed_column_mass(c)-mass_in ) < 1e-12 , "Column mass should match cell mass" );
   fail_unless( fabs( sed_column_thickness(c)-25.) < 1e-12 , "Column thickness should match cell thickness" );

   sed_cell_resize( s , 30 );
   for ( i=0,t=0 ; i<1000 ; i++ )
   {
      mass_in += sed_cell_mass( s );
      t       += sed_column_add_cell( c , s );
   }
   
   fail_unless( fabs( t - 30000. ) < 1e-12                   , "Added thickness should be returned" );
   fail_unless( sed_column_len(c)==30025                     , "Column not resized correctly" );
   fail_unless( fabs( sed_column_mass(c)-mass_in ) < 1e-12   , "Column mass should match cell mass" );
   fail_unless( fabs( sed_column_thickness(c)-30025) < 1e-12 , "Column thickness should match added thickness" );

   sed_cell_destroy( s );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_stack_cell )
{
   Sed_column c   = sed_column_new( 5 );
   Sed_cell s     = sed_cell_new_classed( NULL , .5 , S_SED_TYPE_SAND );
   double mass_in = sed_cell_mass( s );
   double t;

   t = sed_column_stack_cell( c , s );

   fail_unless( fabs( t - .5 ) < 1e-12                     , "Added thickness should be returned" );
   fail_unless( sed_column_len(c)==1                       , "Column not resized correctly" );
   fail_unless( fabs( sed_cell_mass(s)  -mass_in ) < 1e-12 , "Cell should not change mass" );
   fail_unless( fabs( sed_column_mass(c)-mass_in ) < 1e-12 , "Column mass should match cell mass" );
   fail_unless( fabs( sed_column_thickness(c)-.5 ) < 1e-12 , "Column thickness should match cell thickness" );

   sed_cell_resize( s , 128 );
   mass_in += sed_cell_mass( s );
   t = sed_column_stack_cell( c , s );
   
   fail_unless( fabs( t - 128. ) < 1e-12                     , "Added thickness should be returned" );
   fail_unless( sed_column_len(c)==2                         , "Column not resized correctly" );
   fail_unless( eh_compare_dbl( sed_column_mass(c),mass_in,1e-12 ) , "Column mass should match cell mass" );
   fail_unless( fabs( sed_column_thickness(c)-128.5) < 1e-12 , "Column thickness should match added thickness" );

   sed_cell_destroy( s );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_compact_cell )
{
   Sed_column c_0 ;
   Sed_column c = sed_column_new( 5 );
   Sed_cell s   = sed_cell_new_classed( NULL , 1.5 , S_SED_TYPE_SAND );

   sed_column_add_cell( c , s );

   c_0 = sed_column_compact_cell( c , 0 , .5 );

   fail_unless( c_0==c                   , "Column should be returned" );
   fail_unless( sed_cell_is_size(s,1.5)  , "Original cell should not change size" );
   fail_unless( sed_column_size_is(c,1.) , "Column thickness not changed properly" );
   fail_unless( sed_cell_is_size(sed_column_nth_cell(c,0),.5) , "Cell size not changed properly" );

   sed_column_destroy( c );
   sed_cell_destroy  ( s );
}
END_TEST

START_TEST ( test_sed_column_resize_cell )
{
   Sed_column c_0 ;
   Sed_column c = sed_column_new( 5 );
   Sed_cell s   = sed_cell_new_classed( NULL , 1.5 , S_SED_TYPE_SAND );

   sed_column_add_cell( c , s );

   c_0 = sed_column_resize_cell( c , 0 , .5 );

   fail_unless( c_0==c                   , "Column should be returned" );
   fail_unless( sed_cell_is_size(s,1.5)  , "Original cell should not change size" );
   fail_unless( sed_column_size_is(c,1.) , "Column thickness not changed properly" );
   fail_unless( sed_cell_is_size(sed_column_nth_cell(c,0),.5) , "Cell size not changed properly" );

   sed_column_destroy( c );
   sed_cell_destroy  ( s );
}
END_TEST

START_TEST ( test_sed_column_resize_cell_neg )
{
   Sed_column c_0 ;
   Sed_column c = sed_column_new( 5 );
   Sed_cell s   = sed_cell_new_classed( NULL , 1.5 , S_SED_TYPE_SAND );

   sed_column_add_cell( c , s );

   c_0 = sed_column_resize_cell( c , 0 , -.5 );

   fail_unless( c_0==c                   , "Column should be returned" );
   fail_unless( sed_column_size_is(c,.5) , "Column thickness not changed properly" );

   sed_column_destroy( c );
   sed_cell_destroy  ( s );
}
END_TEST

START_TEST ( test_sed_column_resize_cell_over )
{
   Sed_column c_0 ;
   Sed_column c = sed_column_new( 5 );
   Sed_cell s   = sed_cell_new_classed( NULL , 1.5 , S_SED_TYPE_SAND );

   sed_column_add_cell( c , s );

   c_0 = sed_column_resize_cell( c , 2 , .5 );

   fail_unless( c_0==c                    , "Column should be returned" );
   fail_unless( sed_column_size_is(c,1.5) , "Invalid index should be ignored" );

   sed_column_destroy( c );
   sed_cell_destroy  ( s );
}
END_TEST

START_TEST ( test_sed_column_resize_cell_under )
{
   Sed_column c_0 ;
   Sed_column c = sed_column_new( 5 );
   Sed_cell s   = sed_cell_new_classed( NULL , 1.5 , S_SED_TYPE_SAND );

   sed_column_add_cell( c , s );

   c_0 = sed_column_resize_cell( c , -1 , .5 );

   fail_unless( c_0==c                    , "Column should be returned" );
   fail_unless( sed_column_size_is(c,1.5) , "Invalid index should be ignored" );

   sed_column_destroy( c );
   sed_cell_destroy  ( s );
}
END_TEST

START_TEST ( test_sed_column_nth_cell )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell s   = sed_cell_new_classed( NULL , 1. , S_SED_TYPE_SAND );
   gssize i;

   for ( i=0 ; i<10 ; i++ )
   {
      sed_cell_set_age( s , i );
      sed_column_add_cell( c , s );
   }

   for ( i=0 ; i<sed_column_len(c) ; i++ )
   {
      sed_cell_set_age( s , i );
      fail_unless( sed_cell_is_same( sed_column_nth_cell( c , i ) , s ) , "nth cell not returned" );
   }

   sed_cell_destroy( s );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_nth_cell_empty )
{
   Sed_column c = sed_column_new( 5 );

   fail_unless( sed_cell_is_clear( sed_column_nth_cell( c , 0 )) ,  "0-th is clear for an empty column" );
   fail_unless( sed_column_nth_cell( c , 1 )==NULL               ,  "nth is NULL for an empty column" );

   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_nth_cell_null )
{
   fail_unless( sed_column_nth_cell( NULL , 0 )==NULL ,  "nth is NULL for a NULL column" );
}
END_TEST

START_TEST ( test_sed_column_top_cell )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell s   = sed_cell_new_classed( NULL , 12.5 , S_SED_TYPE_SAND );
   Sed_cell top;

   sed_column_add_cell( c , s );

   top = sed_column_top_cell( c );

   fail_unless( sed_column_nth_cell(c,12)==top , "Top cell not returned" );

   sed_column_clear( c );
   top = sed_column_top_cell( c );

   fail_unless( top==NULL , "An empty column's top cell is NULL" );

   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_total_load )
{
   Sed_column c  = sed_column_new( 15 );
   Sed_cell cell = sed_cell_new_classed( NULL , 26. , S_SED_TYPE_SILT );
   double cell_load = sed_cell_load( cell );
   double* load = eh_new( double , 26 );
   double load_0 = 2006;
   gssize i;

   sed_column_add_cell( c , cell );

   sed_column_total_load( c , 0 , sed_column_len(c) , load_0 , load );

   for ( i=25 ; i>=0 ; i-- )
      fail_unless( fabs( load[i] - cell_load*(25-i) + load_0 ) > 1e-12 , "Load calculated incorrectly" );

   eh_free( load );
   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_load_at )
{
   Sed_column c  = sed_column_new( 15 );
   Sed_cell cell = sed_cell_new_classed( NULL , 26. , S_SED_TYPE_SILT );
   double cell_load = sed_cell_load( cell );
   double load;

   sed_column_add_cell( c , cell );

   load = sed_column_load_at( c , 0 );
   fail_unless( fabs( load - cell_load*25. ) > 1e-12 , "Load calculated incorrectly" );

   load = sed_column_load_at( c , sed_column_top_index(c) );
   fail_unless( fabs( load ) > 1e-12 , "Load calculated incorrectly" );

   load = sed_column_load_at( c , sed_column_top_index(c)-5 );
   fail_unless( fabs( load -cell_load*5 ) > 1e-12 , "Load calculated incorrectly" );

   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_load )
{
   Sed_column c  = sed_column_new( 15 );
   Sed_cell cell = sed_cell_new_classed( NULL , 26. , S_SED_TYPE_SILT );
   double cell_load = sed_cell_load( cell );
   double* load = eh_new( double , 26 );
   double* load_0;
   gssize i;

   sed_column_add_cell( c , cell );

   load_0 = sed_column_load( c , 0 , sed_column_len(c) , load );

   fail_unless( load_0==load , "Load array should be returned" );

   for ( i=25 ; i>=0 ; i-- )
      fail_unless( fabs( load[i] - cell_load*(25-i) ) > 1e-12 , "Load calculated incorrectly" );

   eh_free( load );
   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_load_neg_start )
{
   Sed_column c  = sed_column_new( 15 );
   Sed_cell cell = sed_cell_new_classed( NULL , 26. , S_SED_TYPE_SILT );
   double cell_load = sed_cell_load( cell );
   double* load = eh_new( double , 26 );
   gssize i;

   sed_column_add_cell( c , cell );

   sed_column_load( c , -16 , sed_column_len(c) , load );

   for ( i=25 ; i>=0 ; i-- )
      fail_unless( fabs( load[i] - cell_load*(25-i) ) > 1e-12 , "Load calculated incorrectly" );

   eh_free( load );
   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_load_neg_bins )
{
   Sed_column c  = sed_column_new( 15 );
   Sed_cell cell = sed_cell_new_classed( NULL , 26. , S_SED_TYPE_SILT );
   double cell_load = sed_cell_load( cell );
   double* load = eh_new( double , 26 );
   gssize i;

   sed_column_add_cell( c , cell );

   sed_column_load( c , -16 , -1 , load );

   for ( i=25 ; i>=0 ; i-- )
      fail_unless( fabs( load[i] - cell_load*(25-i) ) > 1e-12 , "Load calculated incorrectly" );

   eh_free( load );
   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_load_partial )
{
   Sed_column c  = sed_column_new( 15 );
   Sed_cell cell = sed_cell_new_classed( NULL , 26. , S_SED_TYPE_SILT );
   double cell_load = sed_cell_load( cell );
   double* load = eh_new( double , 5 );
   gssize i;

   sed_column_add_cell( c , cell );

   sed_column_load( c , 0 , 5 , load );

   for ( i=4 ; i>=0 ; i-- )
      fail_unless( fabs( load[i] - cell_load*(25-i) ) > 1e-12 , "Load calculated incorrectly" );

   eh_free( load );
   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_load_null )
{
   Sed_column c  = sed_column_new( 15 );
   Sed_cell cell = sed_cell_new_classed( NULL , 26. , S_SED_TYPE_SILT );
   double cell_load = sed_cell_load( cell );
   double* load;
   gssize i;

   sed_column_add_cell( c , cell );

   load = sed_column_load( c , 0 , sed_column_len(c) , NULL );

   fail_if( load==NULL , "New load array not allocated" );

   for ( i=25 ; i>=0 ; i-- )
      fail_unless( fabs( load[i] - cell_load*(25-i) ) > 1e-12 , "Load calculated incorrectly" );

   eh_free( load );
   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_top_index )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 3.14 , S_SED_TYPE_SAND|S_SED_TYPE_SILT );
   gssize top_ind;

   sed_column_add_cell( c , cell );
   
   top_ind = sed_column_top_index( c );

   fail_unless( top_ind==3 , "Top index not correct" );

   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_top_index_null )
{
   gssize top_ind;
   
   top_ind = sed_column_top_index( NULL );

   fail_unless( top_ind==-1 , "Top index should be -1 for NULL column" );
}
END_TEST

START_TEST ( test_sed_column_top_index_empty )
{
   Sed_column c = sed_column_new( 5 );
   gssize top_ind;
   
   top_ind = sed_column_top_index( c );

   fail_unless( top_ind==-1 , "Top index should be -1 for empty column" );

   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_is_above )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 33 , S_SED_TYPE_SAND );

   sed_column_set_base_height( c , 58 );
   sed_column_add_cell( c , cell );

   fail_unless(  sed_column_is_above(c,90) , "Column should be above" );
   fail_unless( !sed_column_is_above(c,92) , "Column should not be above" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_is_below )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 33 , S_SED_TYPE_SAND );

   sed_column_set_base_height( c , 58 );
   sed_column_add_cell( c , cell );

   fail_unless( !sed_column_is_below(c,90) , "Column should not be below" );
   fail_unless(  sed_column_is_below(c,92) , "Column should be below" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_is_valid_index )
{
   Sed_column c = sed_column_new( 5 );

   fail_unless(  sed_column_is_valid_index(c,0)  , "0 is a valid index for this column" );
   fail_unless( !sed_column_is_valid_index(c,S_ADDBINS)  , "Index will result in seg fault" );
   fail_unless( !sed_column_is_valid_index(c,-1) , "Index will result in seg fault" );

   fail_unless( !sed_column_is_valid_index(NULL,0) , "NULL has no valid indices" );

   sed_column_clear( c );

   fail_unless(  sed_column_is_valid_index(c,0)  , "0 is a valid index for an empty column" );

   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_is_get_index )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 6 , S_SED_TYPE_SAND );
   gssize len;

   sed_column_add_cell( c , cell );
   len = sed_column_len( c );

   fail_unless(  sed_column_is_get_index(c,0)     , "0 is a valid get-index for this column" );
   fail_unless( !sed_column_is_get_index(c,len)   , "Column len is not a valid get-index for a column" );
   fail_unless(  sed_column_is_get_index(c,len-1) , "Column len-1 is a valid get-index for a column" );
   fail_unless( !sed_column_is_get_index(c,-1)    , "-1 is never a valid get-index for a column" );

   fail_unless( !sed_column_is_get_index(NULL,0) , "NULL has no valid get-indices" );

   sed_column_clear( c );

   fail_unless( !sed_column_is_get_index(c,0)  , "An empty column has no valid get indices" );

   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_is_set_index )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 6 , S_SED_TYPE_SAND );
   gssize len;

   sed_column_add_cell( c , cell );
   len = sed_column_len( c );

   fail_unless(  sed_column_is_set_index(c,0)     , "0 is always a valid set-index for a non-NULL column" );
   fail_unless(  sed_column_is_set_index(c,len)   , "Column len is a valid set-index for a column" );
   fail_unless(  sed_column_is_set_index(c,len-1) , "Column len-1 is a valid set-index for a column" );
   fail_unless( !sed_column_is_set_index(c,-1)    , "-1 is never a valid set-index for a column" );

   fail_unless( !sed_column_is_get_index(NULL,0) , "NULL has no valid set-indices" );

   sed_column_clear( c );

   fail_unless(  sed_column_is_set_index(c,0)  , "0 is the only valid set-index for an empty column" );
   fail_unless( !sed_column_is_set_index(c,1)  , "1 is not a valid set-index for an empty column" );

   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_top_nbins )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 50. , S_SED_TYPE_SAND );
   gssize n;

   sed_column_set_base_height( c , 100 );
   sed_column_add_cell( c , cell );

   n = sed_column_top_nbins( c , 150 );
   fail_unless( n==1 , "The top bin" );

   n = sed_column_top_nbins( c , 148.5 );
   fail_unless( n==2 , "Partial bins count" );

   n = sed_column_top_nbins( c , 100 );
   fail_unless( n==sed_column_len(c) , "All of the bins" );

   n = sed_column_top_nbins( c , 99 );
   fail_unless( n==sed_column_len(c) , "Depth to below column uses all bins" );

   n = sed_column_top_nbins( c , 151 );
   fail_unless( n==1 , "Depth above top of column uses only the top bin" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    ); 

}
END_TEST

START_TEST ( test_sed_column_top )
{
   Sed_column c = sed_column_new_filled( 20 , S_SED_TYPE_SAND );
   Sed_cell cell = sed_cell_new_classed(NULL,13,S_SED_TYPE_CLAY);
   Sed_cell cell_0;

   cell_0 = sed_column_top( c , 1.5 , cell );
   
   fail_unless( cell_0==cell               , "Destination cell is returned" );
   fail_unless( sed_cell_is_size(cell,1.5) , "Size of destination cell is amount gotten" );
   fail_unless( sed_cell_is_size_class(cell,S_SED_TYPE_SAND) , "Destination contains column sediment" );
   fail_unless( sed_column_size_is(c,20)   , "Source column should not change" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_top_above )
{
   Sed_column c = sed_column_new_filled( 20 , S_SED_TYPE_SAND );
   Sed_cell cell = sed_cell_new_classed(NULL,13,S_SED_TYPE_CLAY);
   Sed_cell cell_0;

   cell_0 = sed_column_top( c , -1.5 , cell );
   
   fail_unless( cell_0==cell               , "Destination cell is returned" );
   fail_unless( sed_cell_is_clear(cell)    , "Negative thickness returns a clear cell" );
   fail_unless( sed_column_size_is(c,20)   , "Source column should not change" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_top_below )
{
   Sed_column c = sed_column_new_filled( 20 , S_SED_TYPE_SAND );
   Sed_cell cell = sed_cell_new_classed(NULL,13,S_SED_TYPE_CLAY);
   Sed_cell cell_0;

   cell_0 = sed_column_top( c , 21.5 , cell );
   
   fail_unless( cell_0==cell              , "Destination cell is returned" );
   fail_unless( sed_cell_is_size(cell,20) , "Size of destination cell is amount gotten" );
   fail_unless( sed_column_size_is(c,20)  , "Source column should not change" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_top_null )
{
   Sed_column c = sed_column_new_filled( 20 , S_SED_TYPE_SAND );
   Sed_cell cell;

   cell = sed_column_top( c , 1.5 , NULL );
   
   fail_unless( sed_cell_is_valid(cell) , "Valid destination cell is created" );
   fail_unless( sed_cell_is_size(cell,1.5) , "Size of destination cell is amount gotten" );
   fail_unless( sed_cell_is_size_class(cell,S_SED_TYPE_SAND) , "Destination contains column sediment" );
   fail_unless( sed_column_size_is(c,20)  , "Source column should not change" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_top_empty )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed(NULL,13,S_SED_TYPE_CLAY);
   Sed_cell cell_0;

   cell_0 = sed_column_top( c , 1.5 , cell );
   
   fail_unless( cell_0==cell            , "Destination cell is returned" );
   fail_unless( sed_cell_is_clear(cell) , "Empty column returns a clear cell" );
   fail_unless( sed_column_is_empty(c)  , "Source column should not change" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_top_rho )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed(NULL,13,S_SED_TYPE_CLAY|S_SED_TYPE_SAND);
   double rho, rho_0;

   sed_column_add_cell( c , cell );

   rho_0 = sed_cell_density( cell );
   rho = sed_column_top_rho( c , 1.5 );
   
   fail_unless( eh_compare_dbl( rho , rho_0 , 1e-12 ) , "Column density doesn't match cell density" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_top_age )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed(NULL,1,S_SED_TYPE_CLAY|S_SED_TYPE_SAND);
   double age;
   gssize i;

   for ( i=1 ; i<=10 ; i++ )
   {
      sed_cell_set_age( cell , i );
      sed_column_add_cell( c , cell );
   }

   age = sed_column_top_age( c , 1.5 );

   fail_unless( eh_compare_dbl( age , 29./3., 1e-12 ) , "Column age not calculated correctly" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_top_property )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed(NULL,10,S_SED_TYPE_CLAY|S_SED_TYPE_SAND);
   Sed_property p = sed_property_new( "grain" );
   double gz, gz_0;

   sed_column_add_cell( c , cell );

   gz_0 = sed_cell_grain_size_in_phi( cell );
   gz   = sed_column_top_property( p , c , 1.5 );

   fail_unless( eh_compare_dbl( gz , gz_0 , 1e-12 ) , "Column grain size not calculated correctly" );

   sed_property_destroy( p );
   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_top_property_with_load )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed(NULL,10,S_SED_TYPE_CLAY|S_SED_TYPE_SAND);
   Sed_property p = sed_property_new( "cohesion" );
   double cohesion, cohesion_0;
   
   sed_column_add_cell_real( c , cell , FALSE );

   cohesion_0 = sed_cell_cohesion( cell , sed_cell_load(cell)*.1 );
   cohesion   = sed_column_top_property( p , c , 1. );

   fail_unless( eh_compare_dbl( cohesion , cohesion_0 , 1e-12 ) , "Column cohesion not calculated correctly" );

   cohesion_0 = sed_cell_cohesion( cell , sed_cell_load(cell)*.65 );
   cohesion   = sed_column_top_property( p , c , 6.5 );
   
   fail_unless( eh_compare_dbl( cohesion , cohesion_0 , 1e-12 ) , "Column cohesion not calculated correctly" );

   sed_property_destroy( p    );
   sed_cell_destroy    ( cell );
   sed_column_destroy  ( c    );
}
END_TEST

START_TEST ( test_sed_column_index_depth )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 21. , S_SED_TYPE_SAND );
   gssize ind;

   sed_column_add_cell( c , cell );

   ind = sed_column_index_depth( c , 1.5 );
   fail_unless( ind==19 , "Index to middle of cell incorrect" );

   ind = sed_column_index_depth( c , 2 );
   fail_unless( ind==18 , "Index to top of cell incorrect" );

   ind = sed_column_index_depth( c , 0 );
   fail_unless( ind==20 , "Index to column top incorrect" );

   ind = sed_column_index_depth( c , 21 );
   fail_unless( ind==-1 , "Index to column bottom incorrect" );

   ind = sed_column_index_depth( c , 21./2. );
   fail_unless( ind==10 , "Index to column middle incorrect" );

   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_depth_age )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 1. , S_SED_TYPE_SAND );
   double d;
   gssize i;

   for ( i=1 ; i<=3 ; i++ )
   {
      sed_cell_set_age( cell , i/10. );
      sed_column_add_cell( c , cell );
   }

   d = sed_column_depth_age( c , .1 );
   fail_unless( eh_compare_dbl(d,2,1e-12) , "Age of bottom cell" );

   d = sed_column_depth_age( c , .3 );
   fail_unless( eh_compare_dbl(d,0,1e-12) , "Age of top cell" );

   d = sed_column_depth_age( c , .25 );
   fail_unless( eh_compare_dbl(d,1.,1e-12) , "Age of a middle cell" );

   d = sed_column_depth_age( c , 0 );
   fail_unless( eh_compare_dbl(d,3.,1e-12) , "Age below column" );

   d = sed_column_depth_age( c , .4 );
   fail_unless( eh_compare_dbl(d,0,1e-12) , "Age above column" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_thickness_index )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 21. , S_SED_TYPE_SAND );
   double t;

   sed_column_add_cell( c , cell );

   t = sed_column_thickness_index( c , 0 );
   fail_unless( fabs(t-1.) < 1e-12 , "Thickness to first cell" );

   t = sed_column_thickness_index( c , 20 );
   fail_unless( fabs(t-21.) < 1e-12 , "Thickness to last cell" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_thickness_index_neg )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 21. , S_SED_TYPE_SAND );
   double t;

   sed_column_add_cell( c , cell );

   t = sed_column_thickness_index( c , -1 );
   fail_unless( fabs(t) < 1e-12 , "Thickness is zero to negative indices" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_thickness_index_above )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 21. , S_SED_TYPE_SAND );
   double t;

   sed_column_add_cell( c , cell );

   t = sed_column_thickness_index( c , 47 );
   fail_unless( fabs(t-21) < 1e-12 , "Thickness to above column is column thickness" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_thickness_index_null )
{
   Sed_column c = sed_column_new( 5 );
   double t = sed_column_thickness_index( NULL , 0 );

   fail_unless( fabs(t) < 1e-12 , "Thickness of empty column is zero" );

   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_thickness_index_empty )
{
   double t = sed_column_thickness_index( NULL , 47 );
   fail_unless( fabs(t) < 1e-12 , "Thickness of NULL column is zero" );
}
END_TEST

START_TEST ( test_sed_column_index_thickness )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 21. , S_SED_TYPE_SAND );
   gssize ind;

   sed_column_add_cell( c , cell );

   ind = sed_column_index_thickness( c , 3.5 );
   fail_unless( ind==3 , "Index to middle of cell incorrect" );

   ind = sed_column_index_thickness( c , 3 );
   fail_unless( ind==2 , "Index to top of cell incorrect" );

   ind = sed_column_index_thickness( c , 21 );
   fail_unless( ind==20 , "Index to column top incorrect" );

   ind = sed_column_index_thickness( c , 0 );
   fail_unless( ind==-1 , "Index to column bottom incorrect" );

   ind = sed_column_index_thickness( c , 21./2. );
   fail_unless( ind==10 , "Index to column middle incorrect" );

   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_index_at )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 21. , S_SED_TYPE_SAND );
   gssize ind;

   sed_column_set_base_height( c , 142 );
   sed_column_add_cell( c , cell );

   ind = sed_column_index_at( c , 145.5 );
   fail_unless( ind==3 , "Index to middle of cell incorrect" );

   ind = sed_column_index_at( c , 145 );
   fail_unless( ind==2 , "Index to top of cell incorrect" );

   ind = sed_column_index_at( c , 163 );
   fail_unless( ind==20 , "Index to column top incorrect" );

   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_index_at_base )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 21. , S_SED_TYPE_SAND );
   gssize ind;

   sed_column_set_base_height( c , 142 );
   sed_column_add_cell( c , cell );

   ind = sed_column_index_at( c , 142 );
   fail_unless( ind==-1 , "Index to base is incorrect" );

   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_index_at_above )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 21. , S_SED_TYPE_SAND );
   gssize ind;

   sed_column_set_base_height( c , 142 );
   sed_column_add_cell( c , cell );

   ind = sed_column_index_at( c , 175 );
   fail_unless( ind==20 , "Index above top is incorrect" );

   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

START_TEST ( test_sed_column_index_at_below )
{
   Sed_column c = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 21. , S_SED_TYPE_SAND );
   gssize ind;

   sed_column_set_base_height( c , 142 );
   sed_column_add_cell( c , cell );

   ind = sed_column_index_at( c , 100 );
   fail_unless( ind==-1 , "Index above top is incorrect" );

   sed_cell_destroy( cell );
   sed_column_destroy( c );
}
END_TEST

#include <glib/gstdio.h>

START_TEST ( test_sed_column_write )
{
   char* name_used;
   gssize n_bytes;

   {
      Sed_column c = sed_column_new( 5 );
      Sed_cell cell = sed_cell_new_classed( NULL , 23.1 , S_SED_TYPE_SAND|S_SED_TYPE_MUD );
      FILE* fp_tmp = eh_open_temp_file( "sed_column_test.binXXXXXX" , &name_used );

      sed_column_add_cell( c , cell );

      sed_column_set_z_res      ( c , 2.718 );
      sed_column_set_x_position ( c , 3.14  );
      sed_column_set_y_position ( c , 9.81  );
      sed_column_set_base_height( c , 1.414 );
      sed_column_set_age        ( c , 33.   );

      n_bytes = sed_column_write( fp_tmp , c );

      sed_cell_destroy  ( cell );
      sed_column_destroy( c    );

      fclose( fp_tmp );
   }

   {
      FILE* fp;
      FILE* fp_tmp = fopen( name_used , "rb" );
      char* data_0 = eh_new( char , n_bytes );
      char* data_1 = eh_new( char , n_bytes );

      fp = fopen( SED_COLUMN_TEST_FILE , "rb" );

      fread( data_0 , sizeof(char) , n_bytes , fp     );
      fread( data_1 , sizeof(char) , n_bytes , fp_tmp );

      fail_unless( strncmp( data_0 , data_1 , n_bytes )==0 , "Binary file does not compare to test file" );

      eh_free( data_0 );
      eh_free( data_1 );

      fclose( fp     );
      fclose( fp_tmp );
   }

   g_remove( name_used );
}
END_TEST

START_TEST ( test_sed_column_read )
{
   FILE* fp = eh_fopen( SED_COLUMN_TEST_FILE , "rb" );
   Sed_column c;

   c = sed_column_read( fp );

   fail_if    ( c==NULL                                       , "Read error" );
   fail_unless( fabs(sed_column_z_res(c)-2.718)       < 1e-12 , "Read error for column z res" );
   fail_unless( fabs(sed_column_x_position(c)-3.14)   < 1e-12 , "Read error for column x-position" );
   fail_unless( fabs(sed_column_y_position(c)-9.81)   < 1e-12 , "Read error for column y-position" );
   fail_unless( fabs(sed_column_base_height(c)-1.414) < 1e-12 , "Read error for column base height" );
   fail_unless( fabs(sed_column_mass(c)-40425)        < 1e0   , "Read error for column mass" );
   fail_unless( sed_column_len(c)==24                         , "Read error for column length" );

   sed_column_destroy( c );

   fclose( fp );
}
END_TEST

START_TEST ( test_sed_column_chop )
{
   Sed_column c_0;
   Sed_column c  = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 20 , S_SED_TYPE_SAND );

   sed_column_set_base_height( c , 123 );

   sed_column_add_cell( c , cell );

   c_0 = sed_column_chop( c , 133 );

   fail_unless( c_0==c , "Chopped column should be returned" );
   fail_unless( fabs( sed_column_base_height(c)-123 ) < 1e-23 , "Chop doesn't change base height" );
   fail_unless( fabs( sed_column_top_height(c) -133 ) < 1e-23 , "Top height not changed correctly" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_chomp )
{
   Sed_column c_0;
   Sed_column c  = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 20 , S_SED_TYPE_SAND );

   sed_column_set_base_height( c , 123 );

   sed_column_add_cell( c , cell );

   c_0 = sed_column_chomp( c , 133 );

   fail_unless( c_0==c , "Chompped column should be returned" );
   fail_unless( fabs( sed_column_base_height(c)-133 ) < 1e-23 , "Base height not changed correctly" );
   fail_unless( fabs( sed_column_top_height(c) -143 ) < 1e-23 , "Chomp does not change top height" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_strip )
{
   Sed_column c_0;
   Sed_column c  = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 20 , S_SED_TYPE_SAND );

   sed_column_set_base_height( c , 123 );

   sed_column_add_cell( c , cell );

   c_0 = sed_column_strip( c , 133 , 135 );

   fail_unless( c_0==c , "Stripped column should be returned" );
   fail_unless( fabs( sed_column_base_height(c)-133 ) < 1e-23 , "Base height not changed correctly" );
   fail_unless( fabs( sed_column_top_height(c) -135 ) < 1e-23 , "Top height not changed correctly" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_chop_above )
{
   Sed_column c_0;
   Sed_column c  = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 20 , S_SED_TYPE_SAND );

   sed_column_set_base_height( c , 123 );

   sed_column_add_cell( c , cell );

   c_0 = sed_column_chop( c , 145 );

   fail_unless( c_0==c , "Chopped column should be returned" );
   fail_unless( fabs( sed_column_base_height(c)-123 ) < 1e-23 , "Chop doesn't change base height" );
   fail_unless( fabs( sed_column_top_height(c) -143 ) < 1e-23 , "Chopping above column doesn't change height" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_extract_above_0 )
{
   Sed_cell* c_arr = NULL;
   Sed_column c  = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 20 , S_SED_TYPE_SAND );

   sed_column_set_base_height( c , 123 );

   sed_column_add_cell( c , cell );

   c_arr = sed_column_extract_cells_above( c , 140 );

   fail_unless( c_arr!=NULL                                   , "Returned array is null" );
   fail_unless( g_strv_length(c_arr)==3                       , "Incorrect number of cells extracted" );
   fail_unless( fabs( sed_column_base_height(c)-123 ) < 1e-23 , "Extract doesn't change base height" );
   fail_unless( fabs( sed_column_top_height(c) -140 ) < 1e-23 , "Top height not correct" );

   sed_cell_array_free( c_arr );
   sed_cell_destroy   ( cell );
   sed_column_destroy ( c    );
}
END_TEST

START_TEST ( test_sed_column_extract_above_1 )
{
   Sed_cell* c_arr = NULL;
   Sed_column c  = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 20 , S_SED_TYPE_SAND );

   sed_column_set_base_height( c , 123 );

   sed_column_add_cell( c , cell );

   c_arr = sed_column_extract_cells_above( c , 125.1 );

   fail_unless( c_arr!=NULL                                     , "Returned array is null" );
   fail_unless( g_strv_length(c_arr)==18                        , "Incorrect number of cells extracted" );
   fail_unless( fabs( sed_column_base_height(c)-123   ) < 1e-23 , "Extract doesn't change base height" );
   fail_unless( fabs( sed_column_top_height(c) -125.1 ) < 1e-23 , "Top height not correct" );

   sed_cell_array_free( c_arr );
   sed_cell_destroy   ( cell );
   sed_column_destroy ( c    );
}
END_TEST

START_TEST ( test_sed_column_extract_above_2 )
{
   Sed_cell* c_arr = NULL;
   Sed_column c  = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 20 , S_SED_TYPE_SAND );

   sed_column_set_base_height( c , 123 );

   sed_column_add_cell( c , cell );

   c_arr = sed_column_extract_cells_above( c , 120.1 );

   fail_unless( c_arr!=NULL                                   , "Returned array is null" );
   fail_unless( g_strv_length(c_arr)==20                      , "Incorrect number of cells extracted" );
   fail_unless( sed_column_is_empty(c)                        , "Column should have been emptied" );
   fail_unless( fabs( sed_column_base_height(c)-123 ) < 1e-23 , "Extract doesn't change base height" );
   fail_unless( fabs( sed_column_top_height(c) -123 ) < 1e-23 , "Top height not correct" );

   sed_cell_array_free( c_arr );
   sed_cell_destroy   ( cell );
   sed_column_destroy ( c    );
}
END_TEST

START_TEST ( test_sed_column_chop_below )
{
   Sed_column c_0;
   Sed_column c  = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 20 , S_SED_TYPE_SAND );

   sed_column_set_base_height( c , 123 );

   sed_column_add_cell( c , cell );

   c_0 = sed_column_chop( c , 120 );

   fail_unless( c_0==c , "Chopped column should be returned" );
   fail_unless( fabs( sed_column_base_height(c)-120 ) < 1e-23 , "Chopping below column moves base" );
   fail_unless( fabs( sed_column_top_height(c) -120 ) < 1e-23 , "Chopping below column removes all sediment" );
   fail_unless( sed_column_is_empty(c)                        , "Chopping below column empties column" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_chop_null )
{
   fail_unless( sed_column_chop( NULL , 120 )==NULL , "Chopping a NULL column returns NULL" );
}
END_TEST

START_TEST ( test_sed_column_chomp_above )
{
   Sed_column c_0;
   Sed_column c  = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 20 , S_SED_TYPE_SAND );

   sed_column_set_base_height( c , 123 );

   sed_column_add_cell( c , cell );

   c_0 = sed_column_chomp( c , 153 );

   fail_unless( c_0==c , "Chompped column should be returned" );
   fail_unless( fabs( sed_column_base_height(c)-153 ) < 1e-23 , "Base height not changed correctly" );
   fail_unless( fabs( sed_column_top_height(c) -153 ) < 1e-23 , "Top height not changed correctly" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_chomp_below )
{
   Sed_column c_0;
   Sed_column c  = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 20 , S_SED_TYPE_SAND );

   sed_column_set_base_height( c , 123 );

   sed_column_add_cell( c , cell );

   c_0 = sed_column_chomp( c , 120 );

   fail_unless( c_0==c , "Chompped column should be returned" );
   fail_unless( fabs( sed_column_base_height(c)-123 ) < 1e-23 , "Base height not changed correctly" );
   fail_unless( fabs( sed_column_top_height(c) -143 ) < 1e-23 , "Top height not changed correctly" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( c    );
}
END_TEST

START_TEST ( test_sed_column_chomp_null )
{
   fail_unless( sed_column_chomp( NULL , 120 )==NULL , "Chomping a NULL column returns NULL" );
}
END_TEST

START_TEST ( test_sed_cell_add_column )
{
   Sed_column s = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 10 , S_SED_TYPE_SAND );
   Sed_cell cell_0;
   double mass_in;

   sed_cell_set_age( cell , 33 );
   sed_column_add_cell( s , cell );

   sed_cell_set_age( cell , 66 );
   sed_column_add_cell( s , cell );

   mass_in = sed_column_mass( s );

   sed_cell_clear( cell );
   cell_0 = sed_cell_add_column( cell , s );

   fail_unless( cell_0==cell                   , "Cell should be returned"             );
   fail_unless( sed_cell_is_valid(cell)        , "Cell isn't valid"                    );
   fail_unless( sed_cell_is_mass(cell,mass_in) , "Cell mass doesn't match column mass" );
   fail_unless( sed_cell_is_age(cell,49.5)     , "Cell mass doesn't match column mass" );
   fail_unless( sed_column_mass_is(s,mass_in)  , "Column mass should not change"       );

   sed_cell_add_column( cell , s );

   fail_unless( sed_cell_is_mass(cell,2*mass_in) , "Cell should not be cleared before being added to" );

   sed_cell_destroy  ( cell );
   sed_column_destroy( s    );
}
END_TEST

START_TEST ( test_sed_cell_add_column_dup )
{
   Sed_column s = sed_column_new( 5 );
   Sed_cell cell = sed_cell_new_classed( NULL , 33 , S_SED_TYPE_SAND );
   Sed_cell cell_0;
   double mass_in;

   sed_cell_set_age( cell , 33 );
   sed_column_add_cell( s , cell );

   mass_in = sed_column_mass( s );

   cell_0 = sed_cell_add_column( NULL , s );

   fail_unless( sed_cell_is_valid(cell_0)        , "Cell isn't valid"                    );
   fail_unless( sed_cell_is_mass(cell_0,mass_in) , "Cell mass doesn't match column mass" );
   fail_unless( sed_cell_is_age(cell_0,33)       , "Cell age doesn't match column age"   );

   sed_cell_destroy  ( cell   );
   sed_cell_destroy  ( cell_0 );
   sed_column_destroy( s      );
}
END_TEST

START_TEST ( test_sed_cell_add_column_null )
{
   Sed_cell cell = sed_cell_new_classed( NULL , 33 , S_SED_TYPE_SAND );
   Sed_cell cell_0;

   cell_0 = sed_cell_add_column( cell , NULL );

   fail_unless( cell_0==NULL , "A NULL column should return a NULL cell" );

   sed_cell_destroy  ( cell );
}
END_TEST

START_TEST ( test_sed_column_add )
{
   Sed_column d = sed_column_new( 5 );
   Sed_column s = sed_column_new( 5 );
   Sed_cell   c = sed_cell_new_classed( NULL , 58 , S_SED_TYPE_SAND|S_SED_TYPE_CLAY );
   Sed_column d_0;
   double mass_in;

   sed_column_set_base_height( s , 666 );
   sed_column_set_base_height( d , 868 );

   sed_column_add_cell( s , c );

   mass_in = sed_column_mass( s );

   d_0 = sed_column_add( d , s );

   fail_unless( sed_column_mass_is(d,sed_column_mass(s))    , "Source and destination mass does not match" );
   fail_unless( sed_column_mass_is(s,mass_in)               , "Source column mass should not change" );
   fail_unless( d_0==d                                      , "Should return destination column" );
   fail_unless( fabs(sed_column_base_height(d)-868) < 1e-12 , "Column height should not change" );

   sed_column_add( d , s );

   fail_unless( sed_column_mass_is(d,2.*mass_in)            , "Destination column is not cleared before addition" );

   sed_cell_destroy  ( c );
   sed_column_destroy( d );
   sed_column_destroy( s );
}
END_TEST

START_TEST ( test_sed_column_add_dup )
{
   Sed_column d;
   Sed_column s   = sed_column_new( 5 );
   Sed_column s_0 = sed_column_new( 5 );
   Sed_cell   c = sed_cell_new_classed( NULL , 58 , S_SED_TYPE_SAND|S_SED_TYPE_CLAY );

   sed_column_set_base_height( s , 666 );
   sed_column_set_z_res( s , 1.2 );

   d = sed_column_add( NULL , s );

   fail_if    ( d==NULL                                     , "A non-NULL column should be returned" );
   fail_if    ( d==s                                        , "A new column should be created" );
   fail_unless( sed_column_is_same_data(d,s_0)              , "New column should keep newly-created data" );
   fail_unless( sed_column_is_empty(d)                      , "Columns not added correctly" );

   sed_column_add_cell( s , c );
   sed_column_destroy( d );

   d = sed_column_add( NULL , s );

   fail_unless( sed_column_mass_is(d,sed_column_mass(s))    , "Source and destination mass does not match" );

   sed_cell_destroy  ( c   );
   sed_column_destroy( d   );
   sed_column_destroy( s   );
   sed_column_destroy( s_0 );
}
END_TEST

START_TEST ( test_sed_column_add_null )
{
   Sed_column d = sed_column_new( 5 );

   fail_unless( sed_column_add( d , NULL )==NULL , "A NULL source column returns a NULL column" );

   sed_column_destroy( d );
}
END_TEST

START_TEST ( test_sed_column_remove )
{
   Sed_column d_0;
   Sed_column d = sed_column_new( 5 );
   Sed_column s = sed_column_new( 5 );
   Sed_cell c_1 = sed_cell_new_classed( NULL , 25 , S_SED_TYPE_SAND );

   sed_column_set_base_height( d , 100 );

   sed_column_add_cell( d , c_1 );
   sed_column_add_cell( d , c_1 );
   sed_column_add_cell( d , c_1 );

   sed_column_set_base_height( s , 125 );

   d_0 = sed_column_remove( d , s );

   fail_unless( d_0==d                                        , "Eroded column should be returned" );
   fail_unless( sed_column_base_height_is( d , 100 )          , "Base height should not change" );
   fail_unless( sed_column_top_height_is ( d , 125 )          , "Top height should be source base height" );
   fail_unless( sed_column_mass_is ( d , sed_cell_mass(c_1) ) , "Incorrect mass after erosion" );

   sed_cell_destroy  ( c_1 );
   sed_column_destroy( d   );
   sed_column_destroy( s   );
}
END_TEST

START_TEST ( test_sed_column_remove_above )
{
   Sed_column d = sed_column_new( 5 );
   Sed_column s = sed_column_new( 5 );
   Sed_cell c_1 = sed_cell_new_classed( NULL , 25 , S_SED_TYPE_SAND );

   sed_column_set_base_height( d , 100 );

   sed_column_add_cell( d , c_1 );

   sed_column_set_base_height( s , 135 );

   sed_column_remove( d , s );

   fail_unless( sed_column_base_height_is( d , 100 )          , "Base height should not change" );
   fail_unless( sed_column_top_height_is ( d , 125 )          , "Top height should be source base height" );
   fail_unless( sed_column_mass_is ( d , sed_cell_mass(c_1) ) , "Incorrect mass after erosion" );

   sed_cell_destroy  ( c_1 );
   sed_column_destroy( d   );
   sed_column_destroy( s   );
}
END_TEST

START_TEST ( test_sed_column_remove_below )
{
   Sed_column d = sed_column_new( 5 );
   Sed_column s = sed_column_new( 5 );
   Sed_cell c_1 = sed_cell_new_classed( NULL , 25 , S_SED_TYPE_SAND );

   sed_column_set_base_height( d , 100 );

   sed_column_add_cell( d , c_1 );

   sed_column_set_base_height( s , 75 );

   sed_column_remove( d , s );

   fail_unless( sed_column_base_height_is( d , 75 ) , "Erosion below base changes base height" );
   fail_unless( sed_column_top_height_is ( d , 75 ) , "Erosion below base removes all sediment" );
   fail_unless( sed_column_is_empty(d)              , "Erosion below base empties column" );

   sed_cell_destroy  ( c_1 );
   sed_column_destroy( d   );
   sed_column_destroy( s   );
}
END_TEST

START_TEST ( test_sed_column_remove_empty )
{
   Sed_column d = sed_column_new( 5 );
   Sed_column s = sed_column_new( 55 );

   sed_column_set_base_height( d , 100 );
   sed_column_set_base_height( s , 125 );

   sed_column_remove( d , s );

   fail_unless( sed_column_is_empty(d)              , "Erosion of an empty column produces an empty column" );
   fail_unless( sed_column_base_height_is( d , 100 ) , "Erosion above an empty column causes no change in height" );

   sed_column_set_base_height( s , 75 );

   sed_column_remove( d , s );

   fail_unless( sed_column_is_empty(d)              , "Erosion of an empty column produces an empty column" );
   fail_unless( sed_column_base_height_is( d , 75 ) , "Erosion below an empty column changes its base height" );

   sed_column_destroy( d );
   sed_column_destroy( s );
}
END_TEST

Suite *sed_column_suite( void )
{
   Suite *s = suite_create( "Sed_column" );
   TCase *test_case_core   = tcase_create( "Core" );
   TCase *test_case_limits = tcase_create( "Limits" );
   TCase *test_case_set    = tcase_create( "Set" );
   TCase *test_case_io     = tcase_create( "IO" );

   suite_add_tcase( s , test_case_core   );
   suite_add_tcase( s , test_case_limits );
   suite_add_tcase( s , test_case_set    );
   suite_add_tcase( s , test_case_io     );

   tcase_add_test( test_case_core , test_sed_column_new      );
   tcase_add_test( test_case_core , test_sed_column_destroy  );
   tcase_add_test( test_case_core , test_sed_column_copy     );
   tcase_add_test( test_case_core , test_sed_column_clear    );
   tcase_add_test( test_case_core , test_sed_column_stack_cells_loc );
   tcase_add_test( test_case_core , test_sed_column_add_cell );
   tcase_add_test( test_case_core , test_sed_column_add_cell_small );
   tcase_add_test( test_case_core , test_sed_column_add_cell_large );
   tcase_add_test( test_case_core , test_sed_column_stack_cell );
   tcase_add_test( test_case_core , test_sed_column_resize_cell );
   tcase_add_test( test_case_core , test_sed_column_compact_cell );
   tcase_add_test( test_case_core , test_sed_column_height );
   tcase_add_test( test_case_core , test_sed_column_top_cell );
   tcase_add_test( test_case_core , test_sed_column_nth_cell );
   tcase_add_test( test_case_core , test_sed_column_load );
   tcase_add_test( test_case_core , test_sed_column_load_at );
   tcase_add_test( test_case_core , test_sed_column_total_load );
   tcase_add_test( test_case_core , test_sed_column_top_index );
   tcase_add_test( test_case_core , test_sed_column_is_valid_index );
   tcase_add_test( test_case_core , test_sed_column_is_get_index );
   tcase_add_test( test_case_core , test_sed_column_is_set_index );
   tcase_add_test( test_case_core , test_sed_column_index_at );
   tcase_add_test( test_case_core , test_sed_column_index_thickness );
   tcase_add_test( test_case_core , test_sed_column_index_depth );
   tcase_add_test( test_case_core , test_sed_column_depth_age );
   tcase_add_test( test_case_core , test_sed_column_top_nbins );
   tcase_add_test( test_case_core , test_sed_column_rebin );
   tcase_add_test( test_case_core , test_sed_column_thickness_index );
   tcase_add_test( test_case_core , test_sed_column_is_above );
   tcase_add_test( test_case_core , test_sed_column_is_below );
   tcase_add_test( test_case_core , test_sed_column_chop );
   tcase_add_test( test_case_core , test_sed_column_chomp );
   tcase_add_test( test_case_core , test_sed_column_strip );
   tcase_add_test( test_case_core , test_sed_cell_add_column );
   tcase_add_test( test_case_core , test_sed_column_add );
   tcase_add_test( test_case_core , test_sed_column_remove );
   tcase_add_test( test_case_core , test_sed_column_top );
   tcase_add_test( test_case_core , test_sed_column_top_rho );
   tcase_add_test( test_case_core , test_sed_column_top_age );
   tcase_add_test( test_case_core , test_sed_column_top_property );
   tcase_add_test( test_case_core , test_sed_column_top_property_with_load );

   tcase_add_test( test_case_set , test_sed_column_set_height     );
   tcase_add_test( test_case_set , test_sed_column_set_x_position );
   tcase_add_test( test_case_set , test_sed_column_set_y_position );
   tcase_add_test( test_case_set , test_sed_column_set_z_res      );

   tcase_add_test( test_case_limits , test_sed_column_destroy_null );
   tcase_add_test( test_case_limits , test_sed_column_copy_null    );
   tcase_add_test( test_case_limits , test_sed_column_new_neg      );
   tcase_add_test( test_case_limits , test_sed_column_new_zero     );
   tcase_add_test( test_case_limits , test_sed_column_add_cell_empty );
   tcase_add_test( test_case_limits , test_sed_column_resize_cell_neg );
   tcase_add_test( test_case_limits , test_sed_column_resize_cell_over );
   tcase_add_test( test_case_limits , test_sed_column_resize_cell_under );
   tcase_add_test( test_case_limits , test_sed_column_load_partial );
   tcase_add_test( test_case_limits , test_sed_column_load_null );
   tcase_add_test( test_case_limits , test_sed_column_load_neg_start );
   tcase_add_test( test_case_limits , test_sed_column_load_neg_bins );
   tcase_add_test( test_case_limits , test_sed_column_top_index_null );
   tcase_add_test( test_case_limits , test_sed_column_top_index_empty );
   tcase_add_test( test_case_limits , test_sed_column_nth_cell_empty );
   tcase_add_test( test_case_limits , test_sed_column_nth_cell_null );
   tcase_add_test( test_case_limits , test_sed_column_index_at_base );
   tcase_add_test( test_case_limits , test_sed_column_index_at_above );
   tcase_add_test( test_case_limits , test_sed_column_index_at_below );
   tcase_add_test( test_case_limits , test_sed_column_height_empty );
   tcase_add_test( test_case_limits , test_sed_column_height_null );
   tcase_add_test( test_case_limits , test_sed_column_thickness_index_neg );
   tcase_add_test( test_case_limits , test_sed_column_thickness_index_above );
   tcase_add_test( test_case_limits , test_sed_column_thickness_index_null );
   tcase_add_test( test_case_limits , test_sed_column_thickness_index_empty );
   tcase_add_test( test_case_limits , test_sed_column_chomp_above );
   tcase_add_test( test_case_limits , test_sed_column_chomp_below );
   tcase_add_test( test_case_limits , test_sed_column_chomp_null );
   tcase_add_test( test_case_limits , test_sed_column_extract_above_0 );
   tcase_add_test( test_case_limits , test_sed_column_extract_above_1 );
   tcase_add_test( test_case_limits , test_sed_column_extract_above_2 );
   tcase_add_test( test_case_limits , test_sed_column_chop_above );
   tcase_add_test( test_case_limits , test_sed_column_chop_below );
   tcase_add_test( test_case_limits , test_sed_column_chop_null );
   tcase_add_test( test_case_limits , test_sed_cell_add_column_null );
   tcase_add_test( test_case_limits , test_sed_cell_add_column_dup );
   tcase_add_test( test_case_limits , test_sed_column_add_null );
   tcase_add_test( test_case_limits , test_sed_column_add_dup );
   tcase_add_test( test_case_limits , test_sed_column_remove_above );
   tcase_add_test( test_case_limits , test_sed_column_remove_below );
   tcase_add_test( test_case_limits , test_sed_column_remove_empty );
   tcase_add_test( test_case_limits , test_sed_column_top_above );
   tcase_add_test( test_case_limits , test_sed_column_top_below );
   tcase_add_test( test_case_limits , test_sed_column_top_null );
   tcase_add_test( test_case_limits , test_sed_column_top_empty );

   tcase_add_test( test_case_io , test_sed_column_write );
   tcase_add_test( test_case_io , test_sed_column_read  );

   return s;
}

#include "sed_sediment.h"

int test_sed_column( void )
{
   int n;

   {
      Suite *s = sed_column_suite();
      SRunner *sr = srunner_create( s );

      srunner_run_all( sr , CK_NORMAL );
      n = srunner_ntests_failed( sr );
      srunner_free( sr );
   }

   return n;
}


