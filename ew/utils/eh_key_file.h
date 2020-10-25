#ifndef __EH_KEY_FILE_H__
#define __EH_KEY_FILE_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <glib.h>
//#include "utils.h"
#include <utils/eh_types.h>
#include <utils/eh_symbol_table.h>

new_handle(Eh_key_file);

typedef enum {
    EH_ARG_DBL,
    EH_ARG_INT,
    EH_ARG_DARRAY,
    EH_ARG_FILENAME,
}
Eh_arg_type;

typedef struct {
    const gchar*  label;
    Eh_arg_type   arg;
    gpointer      arg_data;
    gint*         arg_data_len;
}
Eh_key_file_entry;

typedef enum {
    EH_KEY_FILE_ERROR_ARRAY_LEN_MISMATCH,
    EH_KEY_FILE_ERROR_MISSING_ENTRY
}
Eh_key_file_error;

#define EH_KEY_FILE_ERROR eh_key_file_error_quark()

Eh_key_file   eh_key_file_new(void);
Eh_key_file   eh_key_file_destroy(Eh_key_file f);
gboolean      eh_key_file_has_group(Eh_key_file f,
    const gchar* group_name);
gboolean      eh_key_file_has_key(Eh_key_file f,
    const gchar* group_name,
    const gchar* key);
gchar**       eh_key_file_get_groups(Eh_key_file f);
gint          eh_key_file_group_size(Eh_key_file f,
    const gchar* group_name);
gint          eh_key_file_size(Eh_key_file f);
gchar**       eh_key_file_get_keys(Eh_key_file f,
    const gchar* group_name);
gchar**       eh_key_file_get_all_values(Eh_key_file f,
    const gchar* group_name,
    const gchar* key);
gchar*        eh_key_file_get_value(Eh_key_file f,
    const gchar* group_name,
    const gchar* key);
gchar*        eh_key_file_get_str_value(Eh_key_file f,
    const gchar* group_name,
    const gchar* key);
gchar**       eh_key_file_get_str_values(Eh_key_file f,
    const gchar* group_name,
    const gchar* key);
gboolean      eh_key_file_get_bool_value(Eh_key_file f,
    const gchar* group_name,
    const gchar* key);
gboolean*     eh_key_file_get_bool_values(Eh_key_file f,
    const gchar* group_name,
    const gchar* key);
double        eh_key_file_get_dbl_value(Eh_key_file f,
    const gchar* group_name,
    const gchar* key);
double*       eh_key_file_get_dbl_array(Eh_key_file      f,
    const gchar* group_name,
    const gchar* key,
    gint*        len);
gint          eh_key_file_get_int_value(Eh_key_file f,
    const gchar* group_name,
    const gchar* key);
double*       eh_key_file_get_dbl_values(Eh_key_file f,
    const gchar* group_name,
    const gchar* key);
void          eh_key_file_set_value(Eh_key_file f,
    const gchar* group_name,
    const gchar* key,
    const gchar* value);
void          eh_key_file_reset_value(Eh_key_file f,
    const gchar* group_name,
    const gchar* key,
    const gchar* value);
Eh_symbol_table eh_key_file_get_symbol_table(Eh_key_file f,
    const gchar* group_name);
Eh_symbol_table* eh_key_file_get_symbol_tables(Eh_key_file f,
    const gchar* group_name);
Eh_key_file   eh_key_file_scan(const char* file, GError** error);
Eh_key_file   eh_key_file_scan_text(const gchar* buffer, GError** error);
gint          eh_key_file_scan_from_template(const gchar* file,
    const gchar* group_name,
    Eh_key_file_entry* t,
    GError** error);
gint          eh_key_file_scan_text_from_template(
    const gchar* buffer,
    const gchar* group_name,
    Eh_key_file_entry t[],
    GError** error);
gssize        eh_key_file_fprint_template(FILE* fp,
    const gchar* group_name,
    Eh_key_file_entry entry[]);
Eh_symbol_table eh_key_file_scan_for(const gchar* file,
    const gchar* name,
    Eh_symbol_table tab,
    GError** error);
Eh_symbol_table eh_key_file_scan_text_for(const gchar* buffer,
    const gchar* name,
    Eh_symbol_table tab,
    GError** error);
Eh_symbol_table eh_key_file_pop_group(Eh_key_file f);

#ifdef __cplusplus
}
#endif

#endif
