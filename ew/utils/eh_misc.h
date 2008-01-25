#ifndef __EH_MISC_H__
#define __EH_MISC_H__

void eh_init_glib     ( void     );
void eh_exit          ( int code );
void eh_exit_on_error ( GError* error , const gchar* format , ... );
void eh_print_on_error( GError* error , const gchar* format , ... );

gint     eh_fprint_version_info( FILE* fp , const gchar* prog , gint maj , gint min , gint micro );
gboolean eh_is_in_domain( gssize n_i , gssize n_j , gssize i , gssize j );

typedef struct { double day, month, year; } Eh_date_t;
double eh_date_to_years( Eh_date_t* d );

double sigma( double s , double t , double p );

typedef gboolean (*Eh_test_func)( void );
void eh_test_function( const char *func_name , Eh_test_func f );

gboolean eh_check_to_s( gboolean assert , const gchar* str , gchar*** str_list );
void eh_set_error_strv( GError** error , GQuark domain , gint code , gchar** err_s );
gchar* eh_render_error_str( GError* error , const gchar* err_str );
gchar* eh_render_command_str( int argc , char* argv[] );


#endif
