#include <eh_utils.h>

FILE*
eh_fopen(const char* filename, const char* type)
{
    FILE* fp;

    eh_require(filename != NULL);

    fp = fopen(filename, type);

    if (!fp) {
        g_error("eh_fopen: could not open file: %s", filename);
        fp = NULL;
    }

    return fp;
}

#include <errno.h>

gchar*
eh_render_file_error_str(gint err_no)
{
    gchar* err_str = NULL;

    if (err_no != 0) {
        GFileError file_error = g_file_error_from_errno(err_no);

        switch (file_error) {
            case G_FILE_ERROR_EXIST:
                err_str = g_strdup("Operation not permitted");
                break;

            case G_FILE_ERROR_ISDIR:
                err_str = g_strdup("File is a directory");
                break;

            case G_FILE_ERROR_ACCES:
                err_str = g_strdup("Permission denied");
                break;

            case G_FILE_ERROR_NAMETOOLONG:
                err_str = g_strdup("Filename too long");
                break;

            case G_FILE_ERROR_NOENT:
                err_str = g_strdup("No such file or directory");
                break;

            case G_FILE_ERROR_NOTDIR:
                err_str = g_strdup("Not a directory");
                break;

            case G_FILE_ERROR_NXIO:
                err_str = g_strdup("No such device or address");
                break;

            case G_FILE_ERROR_NODEV:
                err_str = g_strdup("No such device");
                break;

            case G_FILE_ERROR_ROFS:
                err_str = g_strdup("Read-only file system");
                break;

            case G_FILE_ERROR_TXTBSY:
                err_str = g_strdup("Text file busy");
                break;

            case G_FILE_ERROR_FAULT:
                err_str = g_strdup("Pointer to bad memory");
                break;

            case G_FILE_ERROR_LOOP:
                err_str = g_strdup("Too many levels of symbolic links");
                break;

            case G_FILE_ERROR_NOSPC:
                err_str = g_strdup("No space left on device");
                break;

            case G_FILE_ERROR_NOMEM:
                err_str = g_strdup("No memory available");
                break;

            case G_FILE_ERROR_MFILE:
                err_str = g_strdup("The current process has too many open files");
                break;

            case G_FILE_ERROR_NFILE:
                err_str = g_strdup("The entire system has too many open files");
                break;

            case G_FILE_ERROR_BADF:
                err_str = g_strdup("Bad file descriptor");
                break;

            case G_FILE_ERROR_INVAL:
                err_str = g_strdup("Invalid argument");
                break;

            case G_FILE_ERROR_PIPE:
                err_str = g_strdup("Broken pipe");
                break;

            case G_FILE_ERROR_AGAIN:
                err_str = g_strdup("Resource temporarily unavailable");
                break;

            case G_FILE_ERROR_INTR:
                err_str = g_strdup("Interrupted function call");
                break;

            case G_FILE_ERROR_IO:
                err_str = g_strdup("Input/output error");
                break;

            case G_FILE_ERROR_PERM:
                err_str = g_strdup("Output not permitted");
                break;

            case G_FILE_ERROR_NOSYS:
                err_str = g_strdup("Function not implemented");
                break;

            case G_FILE_ERROR_FAILED:
                err_str = g_strdup("Error unknown");
                break;
        }
    }

    return err_str;
}

FILE*
eh_fopen_error(const char* file, const char* type, GError** error)
{
    FILE*   fp = NULL;

    eh_return_val_if_fail(error == NULL || *error == NULL, NULL);

    fp = fopen(file, type);

    if (!fp) {
        GError* tmp_err = NULL;
        eh_set_file_error_from_errno(&tmp_err, file, errno);
        g_propagate_error(error, tmp_err);
    }

    return fp;
}

void
eh_set_file_error_from_errno(GError** error, const gchar* file, gint err_no)
{
    if (error) {
        eh_return_if_fail(*error == NULL);

        if (err_no != 0) {
            gchar* err_str = eh_render_file_error_str(err_no);

            if (file)
                g_set_error(error,
                    G_FILE_ERROR,
                    g_file_error_from_errno(err_no),
                    "%s: %s", file, err_str);
            else
                g_set_error(error,
                    G_FILE_ERROR,
                    g_file_error_from_errno(err_no),
                    "%s", err_str);

            eh_free(err_str);
        }
    }

    return;
}

FILE*
eh_open_file(const char* filename, const char* type)
{
    FILE* fp;

    if (filename) {
        fp = eh_fopen(filename, type);
    } else {
        fp = stdin;
    }

    return fp;
}

FILE*
eh_open_temp_file(const char* tmpl, char** name_used)
{
    GError* error = NULL;
    int fd = g_file_open_tmp(tmpl, name_used, &error);
    if (error != NULL) {
        g_assert(fd < 0);
        fprintf(stderr, "Unable to open temp file: %s\n", error->message);
        g_error_free(error);
    } else {
        g_assert(fd >= 0);
        return NULL;
    }
    return fdopen(fd, "w+");
}

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

gboolean
eh_is_readable_file(const char* filename)
{
    FILE* fp = fopen(filename, "r");

    if (fp) {
        fclose(fp);
        return TRUE;
    } else {
        return FALSE;
    }
}

gboolean
eh_is_writable_file(const char* filename)
{
    FILE* fp = fopen(filename, "a");

    if (fp) {
        fclose(fp);
        return TRUE;
    } else {
        return FALSE;
    }
}

#include <glib/gstdio.h>

gboolean
eh_try_open(const char* file)
{
    gboolean open_ok = FALSE;

    if (file) {
        if (g_file_test(file, G_FILE_TEST_EXISTS)) {
            open_ok = TRUE;
        } else {
            char* dir = g_path_get_dirname(file);

            open_ok = eh_open_dir(dir, NULL);

            eh_free(dir);
        }
    }

    return open_ok;
}

gboolean
eh_open_dir(const char* dir, GError** error)
{
    gboolean open_ok = TRUE;

    eh_require(dir);
    eh_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    if (dir) {
        if (g_mkdir_with_parents(dir, S_ISUID | S_ISGID | S_IRWXU | S_IRWXG) != 0) {
            GError* tmp_err = NULL;
            eh_set_file_error_from_errno(&tmp_err, dir, errno);
            g_propagate_error(error, tmp_err);
            open_ok = FALSE;
        }
    }

    return open_ok;
}

gboolean
try_open(const char* filename, int flags)
{
    int fd = open(filename, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

    if (fd < 0) {
        fprintf(stderr, "warning : try_open : %s : %s\n", strerror(errno), filename);
        return FALSE;
    } else {
        close(fd);
        return TRUE;
    }
}

gboolean
eh_touch_file(const gchar* file, int flags, GError** error)
{
    gboolean is_ok = TRUE;

    eh_require(file);
    eh_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    if (file) {
        GError* tmp_err  = NULL;
        gchar*  dir_name = g_path_get_dirname(file);

        eh_open_dir(dir_name, &tmp_err);

        eh_free(dir_name);

        if (!tmp_err) {
            gint fd = g_open(file, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

            if (fd == -1) {
                eh_set_file_error_from_errno(&tmp_err, file, errno);
            } else {
                close(fd);
            }
        }

        if (tmp_err) {
            g_propagate_error(error, tmp_err);
            is_ok = FALSE;
        }
    }

    return is_ok;
}

#include <dirent.h>

gboolean
try_dir(const char* file, GError** error)
{
    gboolean is_ok = FALSE;
    int      fd    = g_open(file, O_RDONLY);

    if (fd < 0) {
        GError* tmp_err = NULL;
        eh_set_file_error_from_errno(&tmp_err, file, errno);
        g_propagate_error(error, tmp_err);
        is_ok = FALSE;
    } else {
        close(fd);
        is_ok = TRUE;
    }

    return is_ok;
}

/** Create a list of successive file name.

Create a list of files that all have the same form but contain a numerical
portion that is incremented.  For instance, a list of files might be something
like: file0001.txt, file0002.txt, etc.

The input is a template from which the file of the list will be made.  The
template must contain one '#' which indicates the position of the increment
within the file names.  The '#' will be replaced by a number with a format
of "%04d".

Use eh_get_next_file to obtain the next file name in the list.

@param base_name The form for the files in the list.

@return A pointer to an Eh_file_list.

@see eh_get_next_file , eh_destroy_file_list .
*/
Eh_file_list*
eh_create_file_list(char* base_name)
{
    char** str_array;
    Eh_file_list* list;

    list = eh_new(Eh_file_list, 1);

    str_array = g_strsplit(base_name, "#", -1);
    list->prefix = str_array[0];
    list->suffix = str_array[1];
    list->format = g_strdup("%04d");
    list->count  = 0;

    eh_free(str_array);

    return list;
}

/** Get the next file name from an Eh_file_list.

@param list A pointer to an Eh_file_list.

@return A pointer to string containing the next file name in the list.

@see eh_create_file_list , eh_destroy_file_list .
*/
gchar*
eh_get_next_file(Eh_file_list* list)
{
    char* new_file;
    char* count;

    count = g_strdup_printf(list->format, ++(list->count));
    new_file = g_strconcat(list->prefix, count, list->suffix, NULL);
    eh_free(count);
    return new_file;
}

/** Destroy an Eh_file_list.

@param list A pointer to an Eh_file_list.

@see eh_create_file_list , eh_get_next_file .
*/
void
eh_destroy_file_list(Eh_file_list* list)
{
    if (list) {
        eh_free(list->prefix);
        eh_free(list->suffix);
        eh_free(list->format);
        eh_free(list);
    }
}

