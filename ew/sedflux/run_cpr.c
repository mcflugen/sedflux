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

#define SED_CPR_PROC_NAME "cpr"
#define EH_LOG_DOMAIN SED_CPR_PROC_NAME

#include <string.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "my_processes.h"

gboolean
dump_cpr_data(gpointer ptr, FILE* fp);
gpointer
load_cpr_data(FILE* fp);
void
eh_dump_file_list(Eh_file_list* fl, FILE* fp);
Eh_file_list*
eh_load_file_list(FILE* fp);
gboolean
init_cpr_data(Sed_process proc, Sed_cube prof, GError** error);

Sed_process_info
run_cpr(Sed_process proc, Sed_cube prof)
{
    Cpr_t*           data = (Cpr_t*)sed_process_user_data(proc);
    Sed_process_info info = SED_EMPTY_INFO;
    gchar* file_name;
    FILE*  fp;

    if (sed_process_run_count(proc) == 0) {
        init_cpr_data(proc, prof, NULL);
    }

    file_name = eh_get_next_file(data->file_list);

    fp = fopen(file_name, "wb");

    sed_cube_write(fp, prof);

    fclose(fp);

    return info;
}

#define S_KEY_DIR       "output directory"

gboolean
init_cpr(Sed_process p, Eh_symbol_table tab, GError** error)
{
    Cpr_t*   data    = sed_process_new_user_data(p, Cpr_t);
    GError*  tmp_err = NULL;
    gboolean is_ok   = TRUE;

    eh_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    data->file_list  = NULL;

    data->output_dir = eh_symbol_table_value(tab, S_KEY_DIR);

    if (!tmp_err) {
        try_dir(data->output_dir, &tmp_err);
    }

    if (tmp_err) {
        g_propagate_error(error, tmp_err);
        is_ok = FALSE;
    }

    return is_ok;
}

gboolean
init_cpr_data(Sed_process proc, Sed_cube prof, GError** error)
{
    Cpr_t* data = (Cpr_t*)sed_process_user_data(proc);

    if (data) {
        gchar* cube_name = sed_cube_name(prof);
        gchar* base_name = g_strconcat(data->output_dir,
                G_DIR_SEPARATOR_S,
                cube_name,
                "#",
                ".cpr", NULL);

        data->file_list = eh_create_file_list(base_name);

        eh_free(base_name);
        eh_free(cube_name);
    }

    return TRUE;
}

gboolean
destroy_cpr(Sed_process p)
{
    if (p) {
        Cpr_t* data = (Cpr_t*)sed_process_user_data(p);

        if (data) {
            eh_destroy_file_list(data->file_list);

            eh_free(data->output_dir);
            data->output_dir = NULL;
            eh_free(data);
        }
    }

    return TRUE;
}

gboolean
dump_cpr_data(gpointer ptr, FILE* fp)
{
    Cpr_t* data = (Cpr_t*)ptr;
    gint len;

    eh_require(ptr != NULL);
    eh_require(fp != NULL);

    len = strlen(data->output_dir) + 1;

    eh_dump_file_list(data->file_list, fp);
    fwrite(&len, sizeof(gint), 1, fp);
    fwrite(data->output_dir, sizeof(char), len, fp);

    return TRUE;
}

gpointer
load_cpr_data(FILE* fp)
{
    Cpr_t* data;
    gint len;

    eh_require(fp != NULL);

    data = eh_new(Cpr_t, 1);

    data->file_list = eh_load_file_list(fp);
    fread(&len, sizeof(gint), 1, fp);
    fread(data->output_dir, sizeof(char), len, fp);

    return (gpointer)data;
}

void
eh_dump_file_list(Eh_file_list* fl, FILE* fp)
{
    int len;

    len = strlen(fl->prefix);
    fwrite(&len, sizeof(int), 1, fp);
    fwrite(fl->prefix, sizeof(char), len, fp);

    len = strlen(fl->suffix);
    fwrite(&len, sizeof(int), 1, fp);
    fwrite(fl->suffix, sizeof(char), len, fp);

    len = strlen(fl->format);
    fwrite(&len, sizeof(int), 1, fp);
    fwrite(fl->suffix, sizeof(char), len, fp);

    fwrite(&(fl->count), sizeof(int), 1, fp);

}

Eh_file_list*
eh_load_file_list(FILE* fp)
{
    int len;
    Eh_file_list* fl = eh_new(Eh_file_list, 1);

    fread(&len, sizeof(int), 1, fp);
    fl->prefix = eh_new(char, len);
    fread(fl->prefix, sizeof(char), len, fp);

    fread(&len, sizeof(int), 1, fp);
    fl->suffix = eh_new(char, len);
    fread(fl->suffix, sizeof(char), len, fp);

    fread(&len, sizeof(int), 1, fp);
    fl->format = eh_new(char, len);
    fread(fl->format, sizeof(char), len, fp);

    fread(&(fl->count), sizeof(int), 1, fp);

    return fl;
}

