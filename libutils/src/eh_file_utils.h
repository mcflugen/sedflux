#if !defined( EH_FILE_UTILS_H )
#define EH_FILE_UTILS_H 

FILE *eh_fopen(const char *filename, const char *type);
gchar* eh_render_file_error_str( gint err_no );
FILE* eh_fopen_error( const char* file , const char* type , GError** error );
void eh_set_file_error_from_errno( GError** error , const gchar* file , gint err_no );
FILE* eh_open_file( const char *filename , const char *type );
FILE* eh_open_temp_file( const char *template , char **name_used );
gboolean eh_is_readable_file( const char* filename );
gboolean eh_is_writable_file( const char* filename );
gboolean eh_try_open( const char* file );
gboolean eh_open_dir( const char* dir , GError** error );
gboolean try_open(const char *filename, int flags);
gboolean eh_touch_file( const gchar* file , int flags , GError** error );
gboolean try_dir( const char *file , GError **error );

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
