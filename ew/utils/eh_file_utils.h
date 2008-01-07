#ifndef __EH_FILE_UTILS_H__
#define __EH_FILE_UTILS_H__

FILE*      eh_fopen                    ( const gchar* file , const gchar *type );
gchar*     eh_render_file_error_str    ( gint err_no );
FILE*      eh_fopen_error              ( const gchar* file , const char* type , GError** error );
void       eh_set_file_error_from_errno( GError** error , const gchar* file , gint err_no );
FILE*      eh_open_file                ( const gchar *file , const gchar *type );
FILE*      eh_open_temp_file           ( const gchar *template , gchar **name_used );
gboolean   eh_is_readable_file         ( const gchar* file );
gboolean   eh_is_writable_file         ( const gchar* file );
gboolean   eh_try_open                 ( const gchar* file );
gboolean   eh_open_dir                 ( const gchar* dir  , GError** error );
gboolean   try_open                    ( const gchar* file , int flags);
gboolean   eh_touch_file               ( const gchar* file , int flags , GError** error );
gboolean   try_dir                     ( const gchar* file , GError **error );

typedef struct
{
   char *prefix;
   char *suffix;
   char *format;
   int count;
}
Eh_file_list;

Eh_file_list* eh_create_file_list ( char *base_name    );
gchar*        eh_get_next_file    ( Eh_file_list *list );
void          eh_destroy_file_list( Eh_file_list *list );



#endif /* eh_file_utils.h */
