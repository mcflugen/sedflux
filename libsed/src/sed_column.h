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

#if !defined(SED_COLUMN_H)
# define SED_COLUMN_H

#include <glib.h>
#include "utils.h"

#ifndef S_ADDBINS
# define S_ADDBINS (16)
#endif

#include "datadir_path.h"

#if !defined( DATADIR )
# define DATADIR "/usr/local/share"
#endif

#define SED_COLUMN_TEST_FILE_BE DATADIR "/ew/sed_column-be.bin"
#define SED_COLUMN_TEST_FILE_LE DATADIR "/ew/sed_column-le.bin"

#include "sed_sediment.h"
#include "sed_cell.h"

/** Sed_column

The Sed_cell struct is used to describe a stack of Sed_cell's.  That is,
a vertical column of sediment that is divided into bins that each contain a
Sed_cell.  

The cell pointer points to an array of Sed_cell's that make up the column.  The
first (zero-th) element is the bottom most cell in the column.

The height member gives the elevation to the bottom of the Sed_column.

The thickness member is the thickness of sediment in the Sed_column.  This is
the sum of Sed_cell thicknesses in cell array.

The filled_bins member gives the number of cells in the cell array that are
completely filled with sediment.  

The size member is the number of Sed_cell's in the cell array - full or
otherwise.

The sed member gives the type of sediment contained within each cell.

The cell_height is the vertical resolution of the Sed_column.  Sediment can
be added to a cell up to this amount.  After that, the next Sed_cell in the
array will begin to be filled.  Cells can have smaller sizes for instance if
a cell is partially filled, or if it has been compacted.

The x member gives the horizontal position of the Sed_column.  Currently, a
Sed_column is assumed to lie on a line and so only one coordinate is needed
to describe is position.  In the future it will be placed on a plane and so
we'll need another coordinate.

@see Sediment , Sed_cell , sed_create_column , sed_destroy_column
*/
new_handle( Sed_column );

Sed_column sed_column_new             ( gssize n );
Sed_column sed_column_new_filled      ( double t , Sed_size_class size );
Sed_column sed_column_destroy         ( Sed_column c );
Sed_column sed_column_clear           ( Sed_column c );
Sed_column sed_column_copy            ( Sed_column d , const Sed_column s );
gboolean   sed_column_is_same         ( const Sed_column a , const Sed_column b );
gboolean   sed_column_is_same_data    ( const Sed_column a , const Sed_column b );
Sed_column sed_column_copy_data       ( Sed_column d , const Sed_column s );
Sed_column sed_column_copy_public_data( Sed_column d , const Sed_column s );
Sed_column sed_column_dup             ( const Sed_column s );

double     sed_column_mass            ( const Sed_column c );
double     sed_column_sediment_mass   ( const Sed_column c );
double*    sed_column_total_load      ( const Sed_column c    ,
                                        gssize start          ,
                                        gssize n_bins         ,
                                        double overlying_load ,
                                        double* load );
double*    sed_column_load            ( const Sed_column c    ,
                                        gssize start          ,
                                        gssize n_bins         ,
                                        double* load );

double* sed_column_load_with_water(const Sed_column,gssize,gssize,double*);
double  sed_column_water_pressure( const Sed_column s );
double* sed_column_total_property( Sed_property f , Sed_column c , gssize start , gssize n_bins , double* val );
double* sed_column_avg_property_with_load( Sed_property f , Sed_column c , gssize start , gssize n_bins , double* val );
double* sed_column_avg_property( Sed_property f , Sed_column c , gssize start , gssize n_bins , double* val );
double* sed_column_at_property( Sed_property f , Sed_column c , gssize start , gssize n_bins , double* val );
double  sed_column_load_at(const Sed_column,gssize);
double  sed_column_property_0( Sed_property f, const Sed_column s );
double  sed_column_property( Sed_property f, const Sed_column s );

double sed_column_add_vec(Sed_column,const double*);

Sed_column sed_column_resize           ( Sed_column c , gssize n );
Sed_column sed_column_resize_cell      ( Sed_column c , gssize i , double t );
Sed_column sed_column_compact_cell     ( Sed_column c , gssize i , double t );
double     sed_column_add_cell         ( Sed_column c , const Sed_cell cell );
double     sed_column_append_cell      ( Sed_column c , const Sed_cell cell );
Sed_cell   sed_column_top_cell         ( const Sed_column c );
Sed_cell   sed_column_nth_cell         ( const Sed_column c , gssize i );


Sed_cell sed_column_extract_top_cell(Sed_column,double,Sed_cell);
Sed_column sed_column_remove_top_cell(Sed_column,double);
Sed_cell sed_column_extract_top(Sed_column,double,Sed_cell);
Sed_cell sed_column_extract_top_fill( Sed_column col  ,
                                      double t ,
                                      Sed_cell fill   ,
                                      Sed_cell dest );
Sed_column sed_column_remove_top_cell(Sed_column,double);
Sed_cell sed_column_separate_top(Sed_column,double,double[],Sed_cell);
Sed_cell sed_column_separate_top_amounts( Sed_column col ,
                                          double total_t ,
                                          double t[]     ,
                                          Sed_cell rem_cell );
Sed_cell sed_column_separate_top_amounts_fill( Sed_column col ,
                                               double total_t ,
                                               double t[]     ,
                                               Sed_cell fill  ,
                                               Sed_cell rem_cell );
Sed_cell sed_column_separate_top_thickness( Sed_column col ,
                                            double         ,
                                            double*        ,
                                            Sed_cell );


double     sed_column_top_property_0( Sed_property property , const Sed_column s , double top );
double     sed_column_top_property  ( Sed_property property , const Sed_column s , double top );

double     sed_column_top_rho     ( const Sed_column c , double t );
double     sed_column_top_age     ( const Sed_column c , double t );
Sed_cell   sed_column_top         ( const Sed_column c , double t , Sed_cell d );
gssize     sed_column_top_nbins   ( const Sed_column c , double z );

gssize     sed_column_write   (FILE* fp , const Sed_column  c );
Sed_column sed_column_read    (FILE* fp                       );

Sed_column sed_column_height_copy(const Sed_column,double,Sed_column);

Sed_column sed_column_chomp   ( Sed_column , double );
Sed_column sed_column_chop    ( Sed_column , double );
Sed_column sed_column_strip   ( Sed_column , double , double );

gboolean   sed_column_is_below   ( Sed_column c , double z );
gboolean   sed_column_is_above   ( Sed_column c , double z );

Sed_column sed_column_set_thickness     ( Sed_column c , double t ) G_GNUC_INTERNAL;

double     sed_column_depth_age         ( const Sed_column , double );
gssize     sed_column_index_at          ( const Sed_column c , double h );
double     sed_column_thickness_index   ( const Sed_column c , gssize i );
gssize     sed_column_index_thickness   ( const Sed_column c , double t );
gssize     sed_column_index_depth       ( const Sed_column c , double d );

Sed_column sed_column_add               ( Sed_column d , const Sed_column s );
Sed_column sed_column_remove            ( Sed_column d , const Sed_column s );
Sed_column sed_column_rebin             ( Sed_column c );
Sed_cell   sed_cell_add_column          ( Sed_cell cell , const Sed_column c );

double     sed_column_base_height    ( const Sed_column c );
double     sed_column_top_height     ( const Sed_column c );
double     sed_column_x_position     ( const Sed_column c );
double     sed_column_y_position     ( const Sed_column c );
double     sed_column_age            ( const Sed_column c );
double     sed_column_water_depth    ( const Sed_column c );
double     sed_column_sea_level      ( const Sed_column c );
double     sed_column_z_res          ( const Sed_column c );
double     sed_column_thickness      ( const Sed_column c );
gssize     sed_column_len            ( const Sed_column c );
gboolean   sed_column_is_empty       ( const Sed_column c );
gboolean   sed_column_is_valid_index ( const Sed_column c , gssize n );
gboolean   sed_column_is_get_index   ( const Sed_column c , gssize n );
gboolean   sed_column_is_set_index   ( const Sed_column c , gssize n );
gboolean   sed_column_size_is        ( const Sed_column c , double t );
gboolean   sed_column_mass_is        ( const Sed_column c , double m );
gboolean   sed_column_sediment_mass_is( const Sed_column c , double m );
gboolean   sed_column_base_height_is ( const Sed_column c , double z );
gboolean   sed_column_top_height_is  ( const Sed_column c , double z );
gssize     sed_column_top_index      ( const Sed_column c );

Sed_column sed_column_set_base_height    ( Sed_column c , double z );
Sed_column sed_column_adjust_base_height ( Sed_column s , double dz );
Sed_column sed_column_set_x_position     ( Sed_column c , double );
Sed_column sed_column_set_y_position     ( Sed_column c , double );
Sed_column sed_column_set_age            ( Sed_column c , double );
Sed_column sed_column_set_sea_level      ( Sed_column c , double );
Sed_column sed_column_set_z_res          ( Sed_column c , double new_dz );

double     sed_column_position           ( const Sed_column c  ) G_GNUC_DEPRECATED;
Sed_column sed_column_set_position       ( Sed_column , double ) G_GNUC_DEPRECATED;
double     sed_column_depth              ( const Sed_column  c ) G_GNUC_DEPRECATED;

#endif /* sed_column.h is included */

