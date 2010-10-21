#ifndef __EH_STR_H__
#define __EH_STR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <utils/eh_symbol_table.h>

typedef enum
{
   EH_STR_ERROR_BAD_RANGE ,
   EH_STR_ERROR_NO_RANGE ,
   EH_STR_ERROR_RANGE_OVERLAP ,
   EH_STR_ERROR_BAD_STRING ,
   EH_STR_ERROR_BAD_UNIT ,
   EH_STR_ERROR_NO_UNIT ,
   EH_STR_ERROR_BAD_LOGICAL_VAL
}
Eh_str_error;

#define EH_STR_ERROR eh_str_error_quark()


double** eh_str_to_time_range_piecewise( const gchar* s , GError** error );
double** eh_str_to_dbl_range_piecewise ( const gchar* s , GError** error );
double*  eh_str_to_dbl_range           ( const gchar* s , GError** error );
double*  eh_str_to_time_range          ( const gchar* s , GError** error );
double   eh_str_to_dbl                 ( const gchar* s , GError** error );
gint64   eh_str_to_int                 ( const gchar* s , GError** error );
double   eh_str_to_time_in_years       ( const gchar* s , GError** error );
gboolean eh_str_is_boolean             ( const gchar* s );
gboolean eh_str_to_boolean             ( const gchar* s , GError** error );


gchar**         eh_strv_append                ( gchar*** str_l , gchar* new_str );
gchar**         eh_strv_concat                ( gchar*** str_l , gchar** new_l );

gint            eh_strv_find                  ( const gchar** str_l , const gchar* needle );

gchar*          eh_str_replace                ( gchar* str , gchar old_c , gchar new_c );
gchar*          eh_str_remove                 ( gchar* str , gchar* start , gint n );
gchar*          eh_str_remove_blocks          ( gchar* str ,
                                                gchar** block_start ,
                                                gchar** block_end );
Eh_symbol_table eh_str_parse_key_value (gchar* str, const gchar* delim_1,
                                        const gchar* delim_2);
gint            eh_str_count_chr              ( gchar* str , gchar* end , gint delim );

gchar* eh_str_remove_to_eol_comments (gchar* str, const gchar* com_start);
gchar*          eh_str_remove_c_style_comments( gchar* str );
gchar*          eh_str_remove_comments        ( gchar* str             ,
                                                const gchar* start_str ,
                                                const gchar* end_str   ,
                                                gchar*** comments );

gchar* eh_str_trim_left         ( gchar *str );
gchar* eh_str_trim_right        ( gchar *str );
gchar* eh_str_remove_white_space( gchar *str );

#ifdef __cplusplus
}
#endif

#endif /* EH_STR_H */
