#if !defined( __EH_DLM_FILE_H__ )
# define __EH_DLM_FILE_H__

#include <glib.h>
#include "eh_symbol_table.h"

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
#endif
