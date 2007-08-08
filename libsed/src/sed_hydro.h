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

#ifndef _HYDRO_INCLUDED_
# define _HYDRO_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "eh_types.h"

new_handle( Sed_hydro );
new_handle( Sed_hydro_file );

typedef enum
{
   SED_HYDRO_ERROR_MISSING_LABEL ,
   SED_HYDRO_ERROR_BAD_PARAMETER
}
Sed_hydro_error;

#define SED_HYDRO_ERROR sed_hydro_error_quark()

#include "datadir_path.h"

#if !defined( DATADIR )
#define DATADIR "/usr/local/share"
#endif

#if G_BYTE_ORDER == G_BIG_ENDIAN
# define SED_HYDRO_TEST_FILE        DATADIR "/sedflux/po_daily-be.river"
# define SED_HYDRO_PO_FILE          DATADIR "/sedflux/po_daily-be.river"
# define SED_HYDRO_EEL_FILE         DATADIR "/sedflux/eel_daily-be.river"
# define SED_HYDRO_TEST_INLINE_FILE DATADIR "/sedflux/po_river.kvf"
#else
# define SED_HYDRO_TEST_FILE        DATADIR "/sedflux/po_daily-le.river"
# define SED_HYDRO_PO_FILE          DATADIR "/sedflux/po_daily-le.river"
# define SED_HYDRO_EEL_FILE         DATADIR "/sedflux/eel_daily-le.river"
# define SED_HYDRO_TEST_INLINE_FILE DATADIR "/sedflux/po_river.kvf"
#endif

//# define SED_HYDRO_TEST_INLINE_FILE "/home/plum/huttone/local/ew114/share"##"/ew/po.inline"

#include "sed_sediment.h"
#include "sed_cell.h"
#include "sed_hydrotrend.h"

//#define HYDRO_INLINE        (1<<0)
//#define HYDRO_HYDROTREND    (1<<1)
//#define HYDRO_USE_BUFFER    (1<<2)
#define HYDRO_BUFFER_LEN    (365)
#define HYDRO_N_SIG_VALUES  (10)

typedef enum
{
   SED_HYDRO_INLINE        ,
   SED_HYDRO_HYDROTREND    ,
   SED_HYDRO_HYDROTREND_BE ,
   SED_HYDRO_HYDROTREND_LE ,
   SED_HYDRO_UNKNOWN
}
Sed_hydro_file_type;

/*
typedef struct
{
   int n_grains;
   int n_seasons;
   int n_samples;
   unsigned char *comment;
}
Hydro_header G_GNUC_INTERNAL;
*/

typedef Sed_hydrotrend_header* (*Hydro_read_header_func)(Sed_hydro_file);
typedef Sed_hydro (*Hydro_read_record_func)(Sed_hydro_file);
typedef double (*Hydro_get_val_func)(Sed_hydro);

Sed_hydrotrend_header* sed_hydro_read_header                ( FILE *fp );
Sed_hydrotrend_header* sed_hydro_read_header_from_byte_order( FILE *fp , gint order );
Sed_hydro     sed_hydro_read_record                ( FILE *fp , int n_grains );
Sed_hydro     sed_hydro_read_record_from_byte_order( FILE *fp , int n_grains , gint order );

gssize        sed_hydro_write_record              ( FILE *fp , Sed_hydro rec , gint order );
gssize        sed_hydro_write_record_to_byte_order( FILE *fp , Sed_hydro rec , gint order );

void          sed_hydro_fprint_default_inline_file( FILE *fp );
gssize        sed_hydro_array_fprint( FILE* fp , Sed_hydro* rec_a );
gssize        sed_hydro_fprint      ( FILE* fp , Sed_hydro  rec   );
//Sed_hydro     sed_hydro_init( char *file );
Sed_hydro*    sed_hydro_scan          ( const gchar* file , GError** error );
Sed_hydro*    sed_hydro_scan_n_records( const gchar* file , gint n_recs , GError** error );

Sed_hydrotrend_header* sed_hydro_scan_inline_header( FILE *fp );
gssize        sed_hydro_read_n_records( FILE* fp , Sed_hydro* rec , int n_grains , int n_recs );

Sed_hydro     sed_hydro_new             ( gssize n_grains );
Sed_hydro     sed_hydro_new_from_table  ( Eh_symbol_table t , GError** error );
gboolean      sed_hydro_check           ( Sed_hydro a , GError** err );
const gchar*  sed_hydro_type_to_s( Sed_hydro_file_type t );
Sed_hydro_file_type sed_hydro_str_to_type( const gchar* type_s );
Sed_hydro_file_type sed_hydro_file_guess_type( const gchar* file , GError** error );
Sed_hydro     sed_hydro_copy            ( Sed_hydro dest , Sed_hydro src );
Sed_hydro     sed_hydro_dup             ( Sed_hydro src );
gboolean      sed_hydro_is_same         ( Sed_hydro a    , Sed_hydro b );
Sed_hydro     sed_hydro_resize          ( Sed_hydro a    , gssize n );
gssize        sed_hydro_size            ( Sed_hydro a   );
Sed_hydro     sed_hydro_destroy         ( Sed_hydro rec );
Sed_hydro*    sed_hydro_array_destroy   ( Sed_hydro* arr );

gssize        sed_hydro_write                ( FILE *fp , Sed_hydro a );
Sed_hydro     sed_hydro_read                 ( FILE *fp );

double*       sed_hydro_copy_concentration   ( double* dest , Sed_hydro a );
double        sed_hydro_nth_concentration    ( Sed_hydro a , gssize n );
Sed_hydro     sed_hydro_adjust_mass            ( Sed_hydro a , double f   );
double        sed_hydro_flow_density           ( Sed_hydro a , double rho );
gboolean      sed_hydro_is_hyperpycnal         ( Sed_hydro a );
double*       sed_hydro_fraction               ( Sed_hydro a );
double        sed_hydro_suspended_concentration( Sed_hydro a );
double        sed_hydro_suspended_flux       ( Sed_hydro a );
double        sed_hydro_suspended_volume_flux( Sed_hydro a );
double        sed_hydro_water_flux           ( Sed_hydro a );
double        sed_hydro_suspended_load       ( Sed_hydro a );
double        sed_hydro_total_load           ( Sed_hydro a );
double        sed_hydro_array_suspended_load ( Sed_hydro* arr );
double        sed_hydro_array_total_load     ( Sed_hydro* arr );
Sed_hydro     sed_hydro_set_nth_concentration( Sed_hydro a , gssize n , double val );
Sed_hydro     sed_hydro_set_velocity         ( Sed_hydro a , double val );
Sed_hydro     sed_hydro_set_width            ( Sed_hydro a , double val );
Sed_hydro     sed_hydro_set_depth            ( Sed_hydro a , double val );
Sed_hydro     sed_hydro_set_bedload          ( Sed_hydro a , double val );
Sed_hydro     sed_hydro_set_duration         ( Sed_hydro a , double val );
Sed_hydro     sed_hydro_set_time             ( Sed_hydro a , double t_0 );
Sed_hydro*    sed_hydro_array_set_time( Sed_hydro* arr , double t_0 );
Sed_hydro     sed_hydro_set_time             ( Sed_hydro a , double val );
Sed_hydro*    sed_hydro_array_set_time       ( Sed_hydro* arr , double t_0 );
double        sed_hydro_duration_in_seconds  ( Sed_hydro a );
double        sed_hydro_velocity             ( Sed_hydro a );
double        sed_hydro_width                ( Sed_hydro a );
double        sed_hydro_depth                ( Sed_hydro a );
double        sed_hydro_bedload              ( Sed_hydro a );
double        sed_hydro_duration             ( Sed_hydro a );

Sed_hydro     sed_hydro_add_cell             ( Sed_hydro a    , const Sed_cell s );
Sed_hydro     sed_hydro_subtract_cell        ( Sed_hydro a    , const Sed_cell s );
Sed_hydro     sed_hydro_average_records      ( Sed_hydro* rec , gssize n_recs );
double        sed_hydro_sum_durations        ( Sed_hydro* rec , gssize n_recs );

Sed_hydro*    sed_hydro_array_eventize_number  ( Sed_hydro* rec_a , gssize n_events );
Sed_hydro*    sed_hydro_array_eventize_fraction( Sed_hydro* rec_a , double f        );
Sed_hydro*    sed_hydro_array_eventize         ( Sed_hydro* rec_a , double f        , gboolean insert );
Sed_hydro*    sed_hydro_process_records        ( Sed_hydro* rec_a        ,
                                                 gssize     n_recs       ,
                                                 gssize     n_sig_values ,
                                                 gboolean   insert_mean_values );

Sed_hydro*    sed_hydro_array_sort              ( Sed_hydro* rec_a , GCompareFunc f );
Sed_hydro*    sed_hydro_array_sort_by_time      ( Sed_hydro* arr );
Sed_hydro*    sed_hydro_array_sort_by_suspended_load( Sed_hydro* arr );
Sed_hydro*    sed_hydro_array_sort_by_total_load( Sed_hydro* arr );

Sed_hydro_file sed_hydro_file_set_wrap( Sed_hydro_file fp , gboolean wrap_is_on );
Sed_hydro_file sed_hydro_file_set_buffer_length( Sed_hydro_file fp , gssize len );
Sed_hydro_file sed_hydro_file_set_sig_values( Sed_hydro_file fp , int n_sig_values );

Sed_hydrotrend_header*  sed_hydro_file_header( Sed_hydro_file fp );
Sed_hydro      sed_hydro_file_read_record( Sed_hydro_file fp );
Sed_hydro_file sed_hydro_file_new( const char*         filename     ,
                                   Sed_hydro_file_type type         ,
                                   gboolean            buffer_is_on ,
                                   gboolean            wrap_is_on   ,
                                   GError** error );
Sed_hydro_file sed_hydro_file_destroy( Sed_hydro_file fp );
Sed_hydro*     sed_hydro_file_fill_buffer( Sed_hydro_file fp );

#endif /* hydro.h is included */
