#if !defined( EH_PROJECT_INCLUDED )
#define EH_PROJECT_INCLUDED

#include <glib.h>
#include <glib/gstdio.h>
#include "eh_types.h"

new_handle( Eh_project );

Eh_project eh_create_project              ( const char *working_dir_name );

char*      eh_project_name                ( Eh_project p                   );
char*      eh_project_dir_name            ( Eh_project p                   );
GDir*      eh_project_dir                 ( Eh_project p                   );
GKeyFile*  eh_project_info_file           ( Eh_project p                   );
char*      eh_project_info_file_full_name ( Eh_project p );
char*      eh_project_get_info_val        ( Eh_project p , const char* key );
Eh_project eh_project_set_info_val        ( Eh_project p     ,
                                            const gchar* key ,
                                            const gchar* value             );
Eh_project eh_project_add_info_val        ( Eh_project p ,
                                            char* key    ,
                                            const gchar* val                      );

Eh_project eh_set_project_dir             ( Eh_project proj  ,
                                            const char* dir_name           );
Eh_project eh_fill_project_info           ( Eh_project proj                );
gint       eh_write_project_info_file     ( Eh_project proj                );
gboolean   eh_read_project_info_file      ( Eh_project proj                );
Eh_project eh_load_project                ( gchar* info_file               );
Eh_project eh_destroy_project             ( Eh_project proj                );
FILE*      eh_open_project_file           ( Eh_project proj  ,
                                            const char *file ,
                                            const char *mode               );
void       eh_close_project_file          ( Eh_project proj ,
                                            FILE *fp                       );
void       eh_close_project_file_all      ( Eh_project proj                );

#endif
