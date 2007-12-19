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

#ifndef _HYDROTREND_INCLUDED_
# define _HYDROTREND_INCLUDED_

#include <stdio.h>
#include <glib.h>

typedef struct
{
   int    n_grains;
   int    n_seasons;
   int    n_samples;
   gchar* comment;
}
Sed_hydrotrend_header;

typedef enum
{
   SED_HYDROTREND_ERROR_BAD_HEADER
}
Sed_hydrotrend_error;

#define SED_HYDROTREND_ERROR sed_hydrotrend_error_quark()

#include <sed/sed_hydro.h>

// Read and write HydroTend header information.
Sed_hydrotrend_header* sed_hydrotrend_read_header( FILE *fp );
Sed_hydrotrend_header* sed_hydrotrend_read_header_from_byte_order( FILE *fp , gint order );
Sed_hydrotrend_header* sed_hydrotrend_header_destroy( Sed_hydrotrend_header* h );
Sed_hydrotrend_header* sed_hydrotrend_join_header_from_byte_order( FILE** fp_list , gint order , GError** err );
gssize                 sed_hydrotrend_write_header_to_byte_order( FILE*  fp        ,
                                                                  gint   n_grains  ,
                                                                  gint   n_seasons ,
                                                                  gint   n_samples ,
                                                                  gchar* comment_s ,
                                                                  gint   order );
gssize                 sed_hydrotrend_write_header( FILE* fp       ,
                                                    gint n_grains  ,
                                                    gint n_seasons ,
                                                    gint n_samples ,
                                                    gchar* comment_s );

gint sed_hydrotrend_byte_order( const gchar* file , GError** error );
gint sed_hydrotrend_guess_byte_order( FILE* fp );

// Read and write HydroTrend records
Sed_hydro          sed_hydrotrend_read_next_rec( FILE* fp , int n_grains );
Sed_hydro          sed_hydrotrend_read_next_rec_from_byte_order( FILE *fp , int n_grains , gint order );
Sed_hydro*         sed_hydrotrend_read_recs( FILE* fp , gint rec_0 , gint n_recs , gint byte_order , GError** error );
gssize             sed_hydrotrend_read_next_n_recs( FILE* fp , Sed_hydro* rec , int n_grains , int n_recs );

Sed_hydro*         sed_hydrotrend_read       ( const gchar* file       ,
                                               gint         byte_order ,
                                               int*         n_seasons  ,
                                               GError**     error );
Sed_hydro*         sed_hydrotrend_read_n_recs( const gchar* file       ,
                                               gint         n_recs     ,
                                               gint         byte_order ,
                                               int*         n_seasons  ,
                                               GError** error );

gssize             sed_hydrotrend_write_record( FILE *fp , Sed_hydro rec );
gssize             sed_hydrotrend_write_record_to_byte_order( FILE *fp , Sed_hydro rec , gint order );
gssize             sed_hydrotrend_write( gchar* file , Sed_hydro* rec_a , gint n_seasons , gchar* comment_s , GError** error );
gssize             sed_hydro_array_write_hydrotrend_records_to_byte_order( FILE* fp , Sed_hydro* rec_a , gint order );
gssize             sed_hydro_array_write_hydrotrend_records( FILE* fp , Sed_hydro* rec_a );

gint               sed_hydrotrend_fseek      ( FILE* fp , gint offset , gint whence , gint byte_order );
gint               sed_hydrotrend_record_size( FILE* fp , gint byte_order , Sed_hydrotrend_header* h );
gint               sed_hydrotrend_n_grains   ( FILE* fp , gint byte_order , Sed_hydrotrend_header* h );
gint               sed_hydrotrend_data_start ( FILE* fp , gint byte_order , Sed_hydrotrend_header* h );

#endif /* hydrotrend.h is included */
