//---
//
// This file is part of sedflux.
//
// sedflux is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// sedflux is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with sedflux; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//---

#include <eh_utils.h>

CLASS(Eh_opt_context)
{
    GOptionContext* context;
    Eh_symbol_table values;
    gchar* name;
    gchar* description;
    gchar* help_description;
};

//---
// Local functions.
//---

gboolean
add_new_value(const gchar* option_name,
    const gchar* value,
    gpointer data,
    GError** error)
{
    gchar* new_str = NULL;

    if (g_str_has_prefix(option_name, "--")) {
        new_str = g_strdup(option_name + 2);
    } else if (g_str_has_prefix(option_name, "-")) {
        new_str = g_strdup(option_name + 1);
    } else {
        eh_require_not_reached();
    }

    eh_symbol_table_replace((Eh_symbol_table)data, new_str, g_strdup(value));

    return TRUE;
}

gssize
get_n_entries(Eh_opt_entry* entries)
{
    gssize n_entries;

    for (n_entries = 0 ; entries[n_entries].long_name != NULL ; n_entries++);

    return n_entries;
}

GOptionEntry
convert_eh_option_entry_to_g_option_entry(Eh_opt_entry entry)
{
    GOptionEntry new_entry;

    new_entry.long_name       = entry.long_name;
    new_entry.short_name      = entry.short_name;
    new_entry.flags           = 0;

    new_entry.arg             = G_OPTION_ARG_CALLBACK;
    new_entry.arg_data        = (gpointer)&add_new_value;

    new_entry.description     = entry.description;
    new_entry.arg_description = entry.arg_description;

    return new_entry;
}

Eh_symbol_table
set_default_values(const char* name,
    Eh_opt_entry* entries,
    Eh_symbol_table tab)
{
    char* ehrc           = g_strdup(".ehrc");
    char* cur_dir        = g_get_current_dir();
    const char* home_dir = g_get_home_dir();
    char* rc_file;
    gssize n_entries = get_n_entries(entries);
    Eh_symbol_table default_tab = NULL;
    gssize i;

    //---
    // If the supplied symbol table is NULL, create a new one.
    //---
    if (tab) {
        default_tab = tab;
    } else {
        default_tab = eh_symbol_table_new();
    }

    //---
    // Create a symbol table with default values.
    //---
    for (i = 0 ; i < n_entries ; i++)
        eh_symbol_table_insert(default_tab,
            (gchar*)entries[i].long_name,
            (gchar*)entries[i].default_val);

    //---
    // Load values from .ehrc in home directoy.
    //---
    rc_file = g_build_filename(home_dir, ehrc, NULL);

    if (eh_is_readable_file(rc_file)) {
        default_tab = eh_key_file_scan_for(rc_file, name, default_tab, NULL);
    }

    eh_free(rc_file);

    //---
    // Load values from .ehrc in current directoy.
    //---
    rc_file = g_build_filename(cur_dir, ehrc, NULL);

    if (eh_is_readable_file(rc_file)) {
        default_tab = eh_key_file_scan_for(rc_file, name, default_tab, NULL);
    }

    //      default_tab = eh_key_file_scan_for( rc_file , name , NULL );
    eh_free(rc_file);

    eh_free(ehrc);
    eh_free(cur_dir);

    return default_tab;
}

//---
// Global functions.
//---

Eh_opt_context
eh_opt_create_context(const gchar* name,
    const gchar* description,
    const gchar* help_description)
{
    Eh_opt_context new_context;

    NEW_OBJECT(Eh_opt_context, new_context);

    new_context->context          = g_option_context_new(description);
    new_context->name             = g_strdup(name);
    new_context->description      = g_strdup(description);
    new_context->help_description = g_strdup(help_description);
    new_context->values           = eh_symbol_table_new();

    return new_context;
}

Eh_opt_context
eh_destroy_context(Eh_opt_context context)
{
    if (context) {
        eh_free(context->name);
        eh_free(context->description);
        eh_free(context->help_description);

        g_option_context_free(context->context);
        eh_symbol_table_destroy(context->values);

        eh_free(context);

        context = NULL;
    }

    return context;
}

Eh_opt_context
eh_opt_set_context(Eh_opt_context context,
    Eh_opt_entry* entries)
{
    gssize n_entries = get_n_entries(entries);
    GOptionEntry* g_entry = g_new(GOptionEntry, n_entries + 1);
    GOptionGroup* grp;
    gssize i;

    //---
    // Create a symbol table that holds the default values for each
    // key-value pair.
    //---
    set_default_values(context->name, entries, context->values);

    //---
    // Convert Eh_opt_entry to GOptionEntry.
    //---
    for (i = 0 ; i < n_entries ; i++) {
        g_entry[i] = convert_eh_option_entry_to_g_option_entry(entries[i]);
    }

    g_entry[n_entries].long_name = NULL;

    //---
    // Add the entries to a goup.
    //---
    grp = g_option_group_new(context->name,
            context->description,
            context->help_description,
            context->values,
            NULL);
    g_option_group_add_entries(grp, g_entry);

    //---
    // Add the group to the context.
    //---
    g_option_context_add_group(context->context, grp);

    return context;
}

gboolean
eh_opt_parse_context(Eh_opt_context context,
    gint* argc,
    gchar*** argv,
    GError** error)
{
    return g_option_context_parse(context->context, argc, argv, error);
}

void
eh_opt_print_label_value(Eh_opt_context context, char* label)
{
    fprintf(stderr, "%s=%s", label, eh_opt_value(context, label));
}

char*
eh_opt_value(Eh_opt_context context, char* label)
{
    char* value = eh_symbol_table_lookup(context->values, label);
    char* rtn = NULL;

    if (value) {
        rtn = value;
    }

    return rtn;
}

char*
eh_opt_str_value(Eh_opt_context c, char* label)
{
    char* value = eh_opt_value(c, label);
    char* rtn = NULL;

    if (value) {
        rtn = g_strdup(value);
    }

    return rtn;
}

gboolean
eh_opt_bool_value(Eh_opt_context c, char* label)
{
    char* value = eh_opt_value(c, label);
    gboolean rtn = FALSE;

    if (value) {
        rtn = eh_str_to_boolean(value, NULL);
    }

    return rtn;
}

int
eh_opt_key_value(Eh_opt_context c, char* label, char* keys[])
{
    char* value = eh_opt_value(c, label);
    int rtn = -1;

    if (value) {
        gssize i;

        for (i = 0 ; keys[i] ; i++)
            if (g_ascii_strcasecmp(value, keys[i]) == 0) {
                rtn = i;
                break;
            }

        if (!keys[i]) {
            fprintf(stderr,
                "error : unknown key (%s) for opt %s.\n",
                value,
                label);

            fprintf(stderr, "error : possible keys are: ");

            for (i = 0 ; keys[i + 1] ; i++) {
                fprintf(stderr, "%s, ", keys[i]);
            }

            fprintf(stderr, "or %s\n", keys[i]);

            eh_exit(-1);
        }
    }

    return rtn;
}

gint
eh_opt_int_value(Eh_opt_context c, char* label)
{
    char* value = eh_opt_value(c, label);
    int rtn = G_MININT;

    if (value) {
        sscanf(value, "%d", &rtn);
    }

    return rtn;
}

double
eh_opt_dbl_value(Eh_opt_context c, char* label)
{
    char* value = eh_opt_value(c, label);
    double rtn = G_MINDOUBLE;

    if (value) {
        sscanf(value, "%lf", &rtn);
    }

    return rtn;
}

void
eh_opt_print_label_value_helper(gchar* label, gchar* value, gpointer fp)
{
    fprintf((FILE*)fp, "%s=%s\n", label, value);
}

void
eh_opt_print_key_file(Eh_opt_context c, FILE* fp)
{
    fprintf(fp, "[ %s ]\n", c->name);
    eh_symbol_table_foreach(c->values,
        (GHFunc)&eh_opt_print_label_value_helper,
        fp);
    return;
}

typedef struct {
    int max_key_len;
    int max_value_len;
    Eh_opt_context context;
}
Print_opt_padded_st;

void
eh_opt_print_opt_padded(char* key,
    char* unused,
    Print_opt_padded_st* user_data)  ;
void
eh_opt_get_max_label_length(char* key,
    char* value,
    Print_opt_padded_st* user_data);

void
eh_opt_print_all_opts(Eh_opt_context c, FILE* fp)
{
    Print_opt_padded_st data;

    data.max_key_len   = 0;
    data.max_value_len = 0;
    data.context       = c;

    fprintf(fp, "--- %s ---\n", c->name);

    eh_symbol_table_foreach(c->values,
        (GHFunc)&eh_opt_get_max_label_length,
        &data);

    eh_symbol_table_foreach(c->values,
        (GHFunc)&eh_opt_print_opt_padded,
        &data);

    return;
}

void
eh_opt_print_opt_padded(char* key,
    char* unused,
    Print_opt_padded_st* user_data)
{
    char* str;
    char* value;

    str = g_strdup_printf("%%-%ds : %%-%ds\n",
            user_data->max_key_len,
            user_data->max_value_len);
    value = eh_opt_value(user_data->context, key);

    if (strlen(value) > 0 && g_strtod(value, NULL) != E_NOVAL) {
        fprintf(stderr, str, key, value);
    } else {
        fprintf(stderr, str, key, "<no value>");
    }

    eh_free(str);

    return;
}

void
eh_opt_get_max_label_length(char* key,
    char* value,
    Print_opt_padded_st* user_data)
{
    int key_len   = strlen(key);
    int value_len = strlen(value);

    if (key_len > user_data->max_key_len) {
        user_data->max_key_len = key_len;
    }

    if (value_len > user_data->max_value_len) {
        user_data->max_value_len = value_len;
    }

    return;
}

