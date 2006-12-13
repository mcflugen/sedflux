#if !defined( EH_SYMBOL_TABLE_H )
#define EH_SYMBOL_TABLE_H

#include <stdio.h>
#include <glib.h>
#include "eh_types.h"
#include "eh_input_val.h"

new_handle( Eh_symbol_table );

Eh_symbol_table  eh_symbol_table_new            ( void                   );
Eh_symbol_table  eh_symbol_table_dup            ( Eh_symbol_table t      );
Eh_symbol_table  eh_symbol_table_copy           ( Eh_symbol_table dest   ,
                                                  Eh_symbol_table src    );
void             eh_symbol_table_foreach        ( Eh_symbol_table s      ,
                                                  GHFunc f               ,
                                                  gpointer user_data );
Eh_symbol_table  eh_symbol_table_merge          ( Eh_symbol_table table1 ,
                                                  ...                    );
void             eh_symbol_table_insert         ( Eh_symbol_table s      ,
                                                  char *key              ,
                                                  char *value            );
void             eh_symbol_table_replace        ( Eh_symbol_table s      ,
                                                  char* key              ,
                                                  char* value            );
char*            eh_symbol_table_lookup         ( Eh_symbol_table s      ,
                                                  const char *key        );
void             eh_symbol_table_print          ( Eh_symbol_table s      ,
                                                  FILE *fp               );
void             eh_symbol_table_print_aligned  ( Eh_symbol_table s      ,
                                                  FILE *fp               );
gssize           eh_symbol_table_size           ( Eh_symbol_table s      );
Eh_symbol_table  eh_symbol_table_destroy        ( Eh_symbol_table s      );


gboolean         eh_symbol_table_has_label      ( Eh_symbol_table s      ,
                                                  gchar* label           );
gboolean         eh_symbol_table_has_labels     ( Eh_symbol_table s      ,
                                                  gchar** labels );
gchar*           eh_symbol_table_value          ( Eh_symbol_table s      ,
                                                  const gchar* label     );
gchar**          eh_symbol_table_values         ( Eh_symbol_table s      ,
                                                  const gchar* label     ,
                                                  const gchar* delimiters );

double           eh_symbol_table_dbl_value      ( Eh_symbol_table s      ,
                                                  gchar* label           );
double*          eh_symbol_table_dbl_array_value( Eh_symbol_table s      ,
                                                  gchar* label           ,
                                                  gint* len              ,
                                                  const gchar* delims    );
double           eh_symbol_table_time_value     ( Eh_symbol_table s      ,
                                                  gchar* label );
gboolean         eh_symbol_table_bool_value     ( Eh_symbol_table s      ,
                                                  gchar* label );
gint             eh_symbol_table_int_value      ( Eh_symbol_table s      ,
                                                  gchar* label           );
Eh_input_val     eh_symbol_table_input_value    ( Eh_symbol_table s      ,
                                                  gchar* label           ,
                                                  GError** err );

#endif
