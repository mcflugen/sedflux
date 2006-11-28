#if !defined( EH_KEY_FILE_H )
#define EH_KEY_FILE_H

#include <glib.h>
#include "utils.h"
#include "eh_types.h"
#include "eh_symbol_table.h"

new_handle( Eh_key_file );

Eh_key_file   eh_key_file_new              ( void );
Eh_key_file   eh_key_file_destroy          ( Eh_key_file f );
gboolean      eh_key_file_has_group        ( Eh_key_file f           ,
                                             const gchar* group_name );
gboolean      eh_key_file_has_key          ( Eh_key_file f           ,
                                             const gchar* group_name ,
                                             const gchar* key );
gchar**       eh_key_file_get_groups       ( Eh_key_file f );
guint         eh_key_file_group_size       ( Eh_key_file f           ,
                                             const gchar* group_name );
gchar**       eh_key_file_get_keys         ( Eh_key_file f           ,
                                             const gchar* group_name );
gchar**       eh_key_file_get_all_values   ( Eh_key_file f           ,
                                             const gchar* group_name ,
                                             const gchar* key );
gchar*        eh_key_file_get_value        ( Eh_key_file f           ,
                                             const gchar* group_name ,
                                             const gchar* key );
gchar*        eh_key_file_get_str_value    ( Eh_key_file f           ,
                                             const gchar* group_name ,
                                             const gchar* key );
gchar**       eh_key_file_get_str_values   ( Eh_key_file f           ,
                                             const gchar* group_name ,
                                             const gchar* key );
gboolean      eh_key_file_get_bool_value   ( Eh_key_file f           ,
                                             const gchar* group_name ,
                                             const gchar* key );
gboolean*     eh_key_file_get_bool_values  ( Eh_key_file f           ,
                                             const gchar* group_name ,
                                             const gchar* key );
double        eh_key_file_get_dbl_value    ( Eh_key_file f ,
                                             const gchar* group_name ,
                                             const gchar* key );
double*       eh_key_file_get_dbl_values   ( Eh_key_file f           ,
                                             const gchar* group_name ,
                                             const gchar* key );
void          eh_key_file_set_value        ( Eh_key_file f           ,
                                             const gchar* group_name ,
                                             const gchar* key        ,
                                             const gchar* value );
void          eh_key_file_reset_value      ( Eh_key_file f           ,
                                             const gchar* group_name ,
                                             const gchar* key        ,
                                             const gchar* value );
Eh_symbol_table eh_key_file_get_symbol_table ( Eh_key_file f           ,
                                             const gchar* group_name );
Eh_symbol_table* eh_key_file_get_symbol_tables( Eh_key_file f          ,
                                              const gchar* group_name );
Eh_key_file   eh_key_file_scan              ( const char* file );
Eh_symbol_table eh_key_file_scan_for        ( const gchar* file      ,
                                              const gchar* name      ,
                                              Eh_symbol_table tab );

#endif
