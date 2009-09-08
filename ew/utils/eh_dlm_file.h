#ifndef __EH_DLM_FILE_H__
#define __EH_DLM_FILE_H__

#include <glib.h>
#include <utils/eh_symbol_table.h>
#include <utils/eh_grid.h>

double**   eh_dlm_read                   ( const gchar* file ,
                                           gchar* delims     ,
                                           gint* n_rows      ,
                                           gint* n_cols      ,
                                           GError** error );
double**   eh_dlm_read_swap              ( const gchar* file ,
                                           gchar* delims     ,
                                           gint* n_rows      ,
                                           gint* n_cols      ,
                                           GError** error );
double***  eh_dlm_read_full              ( const gchar* file ,
                                           gchar* delims     ,
                                           gint** n_rows     ,
                                           gint** n_cols     ,
                                           gchar*** rec_data ,
                                           gint max_records  ,
                                           GError** err );
double***  eh_dlm_read_full_swap         ( const gchar* file ,
                                           gchar* delims     ,
                                           gint** n_rows     ,
                                           gint** n_cols     ,
                                           gchar*** rec_data ,
                                           gint max_records  ,
                                           GError** err );

gint       eh_str_count_chr              ( gchar* str , gchar* end , gint delim );
gchar*     eh_dlm_remove_empty_lines     ( gchar* content );
gchar*     eh_str_remove_to_eol_comments ( gchar* content , gchar* com_start );
gchar*     eh_str_remove_c_style_comments( gchar* content );
gchar*     eh_str_remove_comments        ( gchar* content         ,
                                           const gchar* start_str ,
                                           const gchar* end_str   ,
                                           gchar*** comments );
gchar**    eh_strv_append                ( gchar*** str_l ,
                                           gchar* new_str );
Eh_symbol_table eh_str_parse_key_value   ( gchar* str ,
                                           gchar* delim_1 ,
                                           gchar* delim_2 );
gchar*     eh_str_replace                ( gchar* str  ,
                                           gchar old_c ,
                                           gchar new_c );
gchar*     eh_str_remove                 ( gchar* str   ,
                                           gchar* start ,
                                           gint n );
gchar*     eh_str_remove_blocks          ( gchar* str   ,
                                           gchar** block_start ,
                                           gchar** block_end );

gint       eh_dlm_print                  ( const gchar*   file   ,
                                           const gchar*   delim  ,
                                           const double** data   ,
                                           const gint     n_rows ,
                                           const gint     n_cols ,
                                           GError** error ) ;
gint       eh_dlm_print_swap             ( const gchar*   file   ,
                                           const gchar*   delim  ,
                                           const double** data   ,
                                           const gint     n_rows ,
                                           const gint     n_cols ,
                                           GError** error ) ;
gint       eh_dlm_print_full             ( const gchar*   file   ,
                                           const gchar*   delim  ,
                                           const double** data   ,
                                           const gint     n_rows ,
                                           const gint     n_cols ,
                                           gboolean       swap   ,
                                           GError** error ) ;
gint       eh_dlm_print_dbl_grid         ( const gchar* file  ,
                                           const gchar* delim ,
                                           Eh_dbl_grid g      ,
                                           GError** error );

void       eh_bov_print                  (const char* prefix,
                                          const double* x,
                                          const char* name,
                                          int len[3],
                                          double size[3],
                                          GError** err);
void       eh_curve2d_print              (const char* file,
                                          const double* x,
                                          const double* y,
                                          char* name,
                                          int len,
                                          GError** error);

#endif
