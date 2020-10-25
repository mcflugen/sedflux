#include <eh_utils.h>
#include <stdarg.h>
#include <string.h>

CLASS(Eh_symbol_table)
{
    GHashTable* t;
};

typedef struct {
    FILE* fp;
    int max_key_len;
}
aligned_st;

GQuark
eh_symbol_table_error_quark(void)
{
    return g_quark_from_static_string("eh-symbol-table-error-quark");
}

//---
// Start of local functions
//---

guint
eh_str_case_hash(gconstpointer key);
gboolean
eh_str_case_equal(gconstpointer a, gconstpointer b);

void
eh_symbol_table_free_label(gpointer label)
{
    eh_free(label);
}

guint
eh_str_case_hash(gconstpointer key)
{
    gchar* new_key = g_ascii_strup((const gchar*)key, -1);
    guint ans;

    ans = g_str_hash(new_key);
    eh_free(new_key);

    return ans;
}

gboolean
eh_str_case_equal(gconstpointer a, gconstpointer b)
{
    return (g_ascii_strcasecmp((char*)a, (char*)b) == 0) ? TRUE : FALSE;
}

void
eh_destroy_key(gpointer key, gpointer value, gpointer user_data)
{
    eh_free(key);
}

void
__eh_symbol_table_insert(char* key, char* value, Eh_symbol_table t)
{
    eh_symbol_table_insert(t, g_strdup(key), g_strdup(value));
}

void
eh_print_symbol(char* key, char* value, FILE* fp)
{
    fprintf(fp, "%s : %s\n", key, value);
}

void
eh_print_symbol_aligned(char* key, char* value, aligned_st* st)
{
    int len = st->max_key_len - strlen(key) + 1;
    char* fill = g_strnfill(len, ' ');
    fprintf(st->fp, "%s%s: %s\n", key, fill, value);
    eh_free(fill);
}

void
eh_get_max_key_len(char* key, char* value, int* max_len)
{
    int len = strlen(key);

    if (len > *max_len) {
        *max_len = len;
    }
}

//---
// Start of API
//---

Eh_symbol_table
eh_symbol_table_new(void)
{
    Eh_symbol_table s;
    NEW_OBJECT(Eh_symbol_table, s);
    s->t = g_hash_table_new_full(&eh_str_case_hash,
            &eh_str_case_equal,
            &eh_symbol_table_free_label,
            &eh_symbol_table_free_label);
    return s;
}

Eh_symbol_table
eh_symbol_table_dup(Eh_symbol_table t)
{
    return eh_symbol_table_copy(NULL, t);
}

Eh_symbol_table
eh_symbol_table_copy(Eh_symbol_table dest, Eh_symbol_table src)
{
    if (!dest) {
        dest = eh_symbol_table_new();
    }

    g_hash_table_foreach(src->t, (GHFunc)&__eh_symbol_table_insert, dest);

    return dest;
}

void
eh_symbol_table_foreach(Eh_symbol_table s, GHFunc f, gpointer user_data)
{
    g_hash_table_foreach(s->t, f, user_data);
}

Eh_symbol_table
eh_symbol_table_merge(Eh_symbol_table table1, ...)
{
    va_list args;
    Eh_symbol_table rtn, table;

    rtn = eh_symbol_table_new();
    g_hash_table_foreach(table1->t, (GHFunc)&__eh_symbol_table_insert, rtn);

    va_start(args, table1);
    table = va_arg(args, Eh_symbol_table);

    while (table) {
        g_hash_table_foreach(table->t, (GHFunc)&__eh_symbol_table_insert, rtn);
        table = va_arg(args, Eh_symbol_table);
    }

    va_end(args);
    return rtn;
}

void
eh_symbol_table_insert(Eh_symbol_table s, char* key, char* value)
{
    g_hash_table_insert(s->t, g_strdup(key), g_strdup(value));
}

void
eh_symbol_table_replace(Eh_symbol_table s, const char* key, const char* value)
{
    g_hash_table_replace(s->t, g_strdup(key), g_strdup(value));
}

char*
eh_symbol_table_lookup(Eh_symbol_table s, const char* key)
{
    char* value = (char*)g_hash_table_lookup(s->t, key);

    if (value) {
        return value;
    }

    /*
       else
       {
          eh_info( "eh_symbol_table_lookup : unable to find key : %s\n" , key );
          eh_exit(-1);
       }
       eh_require_not_reached();
    */
    return NULL;
}

void
eh_symbol_table_print(Eh_symbol_table s, FILE* fp)
{
    g_hash_table_foreach(s->t, (GHFunc)&eh_print_symbol, fp);
}

void
eh_symbol_table_print_aligned(Eh_symbol_table s, FILE* fp)
{
    int max_len = 0;
    aligned_st st;
    g_hash_table_foreach(s->t, (GHFunc)&eh_get_max_key_len, &max_len);
    st.fp = fp;
    st.max_key_len = max_len;
    g_hash_table_foreach(s->t, (GHFunc)&eh_print_symbol_aligned, &st);
}

gssize
eh_symbol_table_size(Eh_symbol_table s)
{
    return g_hash_table_size(s->t);
}

Eh_symbol_table
eh_symbol_table_destroy(Eh_symbol_table s)
{
    if (s) {
        g_hash_table_destroy(s->t);
        eh_free(s);
    }

    return NULL;
}

gboolean
eh_symbol_table_has_label(Eh_symbol_table s, const gchar* label)
{
    if (s && label) {
        if (g_hash_table_lookup(s->t, label)) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    return FALSE;
}

gboolean
eh_symbol_table_has_labels(Eh_symbol_table s, const gchar** labels)
{
    gboolean has_all_labels = TRUE;

    if (labels) {
        const gchar** this_label;

        for (this_label = labels ; *this_label && has_all_labels ; this_label++) {
            has_all_labels = has_all_labels && eh_symbol_table_has_label(s, *this_label);
        }
    }

    return has_all_labels;
}

gboolean
eh_symbol_table_require_labels(Eh_symbol_table s, const gchar** labels, GError** error)
{
    gboolean has_all_labels = TRUE;

    eh_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    if (labels) {
        GError* tmp_err = NULL;
        gchar** err_s   = NULL;
        const gchar** this_label;

        for (this_label = labels ; *this_label ; this_label++) {
            if (!eh_symbol_table_has_label(s, *this_label)) {
                eh_strv_append(&err_s, g_strdup_printf("Label not found: %s", *this_label));
            }
        }

        if (err_s) {
            eh_set_error_strv(&tmp_err, EH_SYM_TABLE_ERROR, EH_SYM_TABLE_ERROR_MISSING_LABEL,
                err_s);

            g_propagate_error(error, tmp_err);
            g_strfreev(err_s);

            has_all_labels = FALSE;
        }
    }

    return has_all_labels;
}

/** Return value string from a symbol table

Retrieve a copy of the string associated with a label in a Eh_symbol_table.
If symbol table does not contain the label, return NULL.  Note that there
is no way to tell the difference between the case where the requested value
is NULL and the table not containing the specified label.

@param s A Eh_symbol_table
@param label The label to lookup

@returns The associated value, or NULL if the label is not found.
*/
gchar*
eh_symbol_table_value(Eh_symbol_table s, const gchar* label)
{
    gchar* ans = NULL;

    if (s && label) {
        ans = g_strdup((const gchar*)g_hash_table_lookup(s->t, label));
    }

    return ans;
}

gchar**
eh_symbol_table_values(Eh_symbol_table s, const gchar* label, const gchar* delimiters)
{
    gchar** str_list = NULL;

    if (s) {
        gchar* value_str = eh_symbol_table_value(s, label);

        if (value_str) {
            if (!delimiters) {
                delimiters = ",:;";
            }

            str_list = g_strsplit_set(value_str, delimiters, -1);
        }

        eh_free(value_str);
    }

    return str_list;
}

double
eh_symbol_table_dbl_value(Eh_symbol_table s, const gchar* label)
{
    gchar* str = eh_symbol_table_value(s, label);
    double ans;

    if (str) {
        ans = g_ascii_strtod(str, NULL);
    } else {
        ans = eh_nan();
    }

    eh_free(str);

    return ans;
}

double*
eh_symbol_table_dbl_range(Eh_symbol_table s, const gchar* label)
{
    gchar* str = eh_symbol_table_value(s, label);
    double* ans;

    if (str) {
        ans = eh_str_to_dbl_range(str, NULL);
    } else {
        ans = NULL;
    }

    eh_free(str);

    return ans;
}

double*
eh_symbol_table_dbl_array_value(Eh_symbol_table s, const gchar* label, gint* len,
    const gchar* delims)
{
    double* dbl_array = NULL;

    eh_require(s);
    eh_require(label);

    if (s && label) {
        gchar** str_array = eh_symbol_table_values(s, label, delims);

        if (str_array) {
            gint i;
            gchar** str;

            dbl_array = eh_new(double, g_strv_length(str_array));

            for (str = str_array, i = 0 ; *str ; str++, i++) {
                dbl_array[i] = g_ascii_strtod(*str, NULL);
            }

            *len = i;
        }

        eh_free(str_array);
    }

    return dbl_array;
}

double
eh_symbol_table_time_value(Eh_symbol_table s, const gchar* label)
{
    gchar* str = eh_symbol_table_value(s, label);
    double ans;

    if (str) {
        ans = eh_str_to_time_in_years(str, NULL);
    } else {
        ans = eh_nan();
    }

    eh_free(str);

    return ans;
}

double*
eh_symbol_table_time_array_value(Eh_symbol_table s, const gchar* label, gint* len,
    const gchar* delims)
{
    double* dbl_array = NULL;

    eh_require(s);
    eh_require(label);

    if (s && label) {
        gchar** str_array = eh_symbol_table_values(s, label, delims);

        if (str_array) {
            gint i;
            gchar** str;

            dbl_array = eh_new(double, g_strv_length(str_array));

            for (str = str_array, i = 0 ; *str ; str++, i++) {
                dbl_array[i] = eh_str_to_time_in_years(*str, NULL);
            }

            *len = i;
        }

        eh_free(str_array);
    }

    return dbl_array;
}

double*
eh_symbol_table_time_range(Eh_symbol_table s, const gchar* label)
{
    gchar*  str = eh_symbol_table_value(s, label);
    double* ans;

    if (str) {
        ans = eh_str_to_time_range(str, NULL);
    } else {
        ans = NULL;
    }

    eh_free(str);

    return ans;
}

gboolean
eh_symbol_table_bool_value(Eh_symbol_table s, const gchar* label)
{
    gchar* str = eh_symbol_table_value(s, label);
    gboolean ans;

    if (str) {
        ans = eh_str_to_boolean(str, NULL);
    }

    eh_free(str);

    return ans;
}

gint
eh_symbol_table_int_value(Eh_symbol_table s, const gchar* label)
{
    gchar* str = eh_symbol_table_value(s, label);
    gint ans = G_MAXINT;

    if (str) {
        sscanf(str, "%d", &ans);
    }

    eh_free(str);

    return ans;
}

Eh_input_val
eh_symbol_table_input_value(Eh_symbol_table s, const gchar* label, GError** err)
{
    return eh_input_val_set(eh_symbol_table_lookup(s, label), err);
}

