#include <eh_utils.h>

#if G_BYTE_ORDER==G_LITTLE_ENDIAN

gsize
eh_fread_int32_from_be(void* ptr, gsize size, gsize nitems, FILE* stream)
{
    gsize n = 0;

    eh_require(size == sizeof(gint32));

    if (ptr && stream) {
        gsize i;
        gint32 i_val;

        for (i = 0 ; i < nitems ; i++) {
            n += fread(&i_val, sizeof(gint32), 1, stream);
            ((gint32*)ptr)[i] = GINT32_TO_BE(i_val);
        }
    }

    return n;
}

gsize
eh_fread_int64_from_be(void* ptr, gsize size, gsize nitems, FILE* stream)
{
    gsize n = 0;

    eh_require(size == sizeof(gint64));

    if (ptr && stream) {
        gsize i;
        gint64 i_val;

        for (i = 0 ; i < nitems ; i++) {
            n += fread(&i_val, sizeof(gint64), 1, stream);
            ((gint64*)ptr)[i] = GINT64_TO_BE(i_val);
        }
    }

    return n;
}

gsize
eh_fwrite_int32_to_be(const void* ptr, gsize size, gsize nitems, FILE* stream)
{
    gsize n = 0;

    eh_require(size == sizeof(gint32));

    if (ptr && stream) {
        gsize i;
        gint32 i_val;

        for (i = 0 ; i < nitems ; i++) {
            i_val = GINT32_TO_BE(((gint32*)(ptr))[i]);
            n += fwrite(&i_val, sizeof(gint32), 1, stream);
        }
    }

    return n;
}

gsize
eh_fwrite_int64_to_be(const void* ptr, gsize size, gsize nitems, FILE* stream)
{
    gsize n = 0;

    eh_require(size == sizeof(gint64));

    if (ptr && stream) {
        gsize i;
        gint64 i_val;

        for (i = 0 ; i < nitems ; i++) {
            i_val = GINT64_TO_BE(((gint64*)(ptr))[i]);
            n += fwrite(&i_val, sizeof(gint64), 1, stream);
        }
    }

    return n;
}

#else


gsize
eh_fread_int32_from_le(void* ptr, gsize size, gsize nitems, FILE* stream)
{
    gsize n = 0;

    eh_require(size == sizeof(gint32));

    if (ptr && stream) {
        gsize i;
        gint32 i_val;

        for (i = 0 ; i < nitems ; i++) {
            n += fread(&i_val, sizeof(gint32), 1, stream);
            ((gint32*)ptr)[i] = GINT32_TO_LE(i_val);
        }
    }

    return n;
}

gsize
eh_fread_int64_from_le(const void* ptr, gsize size, gsize nitems, FILE* stream)
{
    gsize n = 0;

    eh_require(size == sizeof(gint64));

    if (ptr && stream) {
        gsize i;
        gint64 i_val;

        for (i = 0 ; i < nitems ; i++) {
            n += fread(&i_val, sizeof(gint64), 1, stream);
            ((gint64*)ptr)[i] = GINT64_TO_LE(i_val);
        }
    }

    return n;
}

gsize
eh_fwrite_int32_to_le(const void* ptr, gsize size, gsize nitems, FILE* stream)
{
    gsize n = 0;

    eh_require(size == sizeof(gint32));

    if (ptr && stream) {
        gsize i;
        gint32 i_val;

        for (i = 0 ; i < nitems ; i++) {
            i_val = GINT32_TO_LE(((gint32*)(ptr))[i]);
            n += fwrite(&i_val, sizeof(gint32), 1, stream);
        }
    }

    return n;
}

gsize
eh_fwrite_int64_to_le(const void* ptr, gsize size, gsize nitems, FILE* stream)
{
    gsize n = 0;

    eh_require(size == sizeof(gint64));

    if (ptr && stream) {
        gsize i;
        gint64 i_val;

        for (i = 0 ; i < nitems ; i++) {
            i_val = GINT64_TO_LE(((gint64*)(ptr))[i]);
            n += fwrite(&i_val, sizeof(gint64), 1, stream);
        }
    }

    return n;
}

#endif

#undef HAVE_GETLINE

#ifndef HAVE_GETLINE
# ifdef HAVE_FGETLN

gssize
getline(gchar** lineptr, gsize* n, FILE* stream)
{
    gssize result = -1;
    gsize  len;
    gchar* s = fgetln(stream, &len);

    if (s || feof(stream)) {
        if (len + 1 > *n) {
            *lineptr = eh_renew(gchar*, *lineptr, len + 1);
            *n       = len + 1;
        }

        memcpy(*lineptr, s, len);
        (*lineptr)[len] = '\0';

        if (!feof(stream)) {
            result   = len;    // Careful! Assignment of unsigned int to signed int
        } else {
            result   = -1;
        }
    } else {
        *n = 0;
        *lineptr = NULL;
    }

    return result;
}
# else /* don't have fgetln */
gssize
getline(gchar** lineptr, gsize* n, FILE* stream)
{
    const int block_size = 256;
    gssize result = -1;

    if (*lineptr == NULL || *n == 0) {
        *n = block_size;
        *lineptr = eh_new(gchar, *n);
    }

    if (*lineptr) {
        gchar* p;
        gchar* found_eol = NULL;
        const gint start = ftell(stream);
        int bytes_read = 0;

        p = fgets(*lineptr, *n - 1, stream); /* Read in a block of characters */

        if (p) {
            bytes_read = strlen(p);
            found_eol = (gchar*)memchr(p, '\n', bytes_read);  /* Look for the delimiter */
        } else {
            bytes_read = 0;
            found_eol = NULL;
        }

        if (bytes_read > 0 && !found_eol && !feof(stream)) {
            int len = *n;

            // Scan until a new line is found
            for (p += len; p && !found_eol; p += block_size, len += block_size) {
                *lineptr = eh_renew(gchar, *lineptr, len + block_size);
                *n = len + block_size;
                p = fgets(p, block_size - 1, stream);

                if (p) {
                    bytes_read += strlen(p);
                    found_eol = (gchar*) memchr(p, '\n', block_size);
                }
            }
        }

        if (ferror(stream)) {
            result = -1;
        } else {
            if (feof(stream) && bytes_read == 0) {
                result = -1;
            } else {
                result = strlen(*lineptr);

                // Move the file pointer to after the delimeter
                fseek(stream, start + result, SEEK_SET);
            }
        }

#if 0

        if (!feof(stream) && !t) { /* Delimeter not found */
            gint len = *n;

            for (p += len ; p && !t ; p += 2048, len += 2048) {
                *lineptr  = eh_renew(gchar, *lineptr,
                        len + 2048);   /* Extend the length of the string */
                *n        = len + 2048;
                p         = fgets(p, 2048, stream);     /* Read the next block of bytes */

                if (p) {
                    t = (gchar*)memchr(p, '\n', 2048);    /* Look for the delimiter */
                }
            }
        }

        if (ferror(stream)) {
            result = -1;
        } else {
            if (feof(stream))
                result = -1
                else {
                    // Careful! Assignment of unsigned int to signed int
                    result = strlen(*lineptr);

                    // Move the file pointer to after the delimeter
                    fseek(stream, start + result, SEEK_SET);
                }
        }

#endif
    }

    return result;
}
# endif
#endif /* don't have getline */


#include <stdio.h>
//#include <values.h>
#include <string.h>

#ifndef DATA_DELIMETER
    #define DATA_DELIMETER ":"
#endif

#ifndef TOKEN_DELIMETER
    #define TOKEN_DELIMETER ","
#endif

#include <errno.h>

gchar*
eh_scan_str(FILE* fp, GError** error)
{
    gchar* s = NULL;

    eh_require(fp);
    eh_return_val_if_fail(error == NULL || *error == NULL, NULL);

    if (fp) {
        gsize  n;
        gchar*  s;
        GError* tmp_err = NULL;

        getline(&s, &n, fp);

        if (s) {
            s = (gchar*)g_memdup(s, n + 1);
            s[n] = '\0';
        } else if (feof(fp)) {
            s    = NULL;
        } else {
            eh_set_file_error_from_errno(&tmp_err, NULL, errno);
        }

        if (tmp_err) {
            g_propagate_error(error, tmp_err);
        }
    }

    return s;
}

gchar**
eh_scan_str_array(FILE* fp, gint* len, GError** error)
{
    gchar** s_arr = NULL;

    eh_require(fp);
    eh_return_val_if_fail(error == NULL || *error == NULL, NULL);

    if (fp) {
        gint    n;
        GError* tmp_err = NULL;
        gchar*  s       = eh_scan_str(fp, &tmp_err);

        if (s) {
            s_arr = g_strsplit_set(s, ";,", -1);
            n     = g_strv_length(s_arr);

            eh_free(s);
        } else if (feof(fp)) {
            s_arr    = NULL;
            n        = 0;
        } else {
            eh_set_file_error_from_errno(&tmp_err, NULL, errno);
        }

        if (tmp_err) {
            g_propagate_error(error, tmp_err);
            s_arr = NULL;
            n     = 0;
        }

        if (len) {
            *len = n;
        }
    }

    return s_arr;
}

gint*
eh_scan_int_array(FILE* fp, gint* len, GError** error)
{
    gint* i_arr = NULL;

    eh_require(fp);
    eh_return_val_if_fail(error == NULL || *error == NULL, NULL);

    if (fp) {
        gint    n       = 0;
        GError* tmp_err = NULL;
        gchar** str_arr = eh_scan_str_array(fp, &n, &tmp_err);

        if (str_arr) {
            gint i;

            n = g_strv_length(str_arr);

            i_arr = eh_new(gint, n);

            for (i = 0 ; i < n && !tmp_err ; i++) {
                i_arr[i] = eh_str_to_int(str_arr[i], &tmp_err);
            }

            g_strfreev(str_arr);

            if (tmp_err) {
                eh_free(i_arr);
                i_arr = NULL;
                n     = 0;
            }
        }

        if (tmp_err) {
            g_propagate_error(error, tmp_err);
            i_arr = NULL;
            n     = 0;
        }

        if (len) {
            *len = n;
        }
    }

    return i_arr;
}

double*
eh_scan_dbl_array(FILE* fp, gint* len, GError** error)
{
    double* d_arr = NULL;

    eh_require(fp);
    eh_return_val_if_fail(error == NULL || *error == NULL, NULL);

    if (fp) {
        gint    n       = 0;
        GError* tmp_err = NULL;
        gchar** str_arr = eh_scan_str_array(fp, &n, &tmp_err);

        if (str_arr) {
            gint i;
            n = g_strv_length(str_arr);

            d_arr = eh_new(double, n);

            for (i = 0 ; i < n && !tmp_err ; i++) {
                d_arr[i] = eh_str_to_dbl(str_arr[i], &tmp_err);
            }

            g_strfreev(str_arr);

            if (tmp_err) {
                eh_free(d_arr);
                d_arr = NULL;
                n     = 0;
            }
        }

        if (tmp_err) {
            g_propagate_error(error, tmp_err);
            d_arr = NULL;
            n     = 0;
        }

        if (len) {
            *len = n;
        }
    }

    return d_arr;
}

gboolean*
eh_scan_boolean_array(FILE* fp, gint* len, GError** error)
{
    gboolean* b_arr = NULL;

    eh_require(fp);
    eh_return_val_if_fail(error == NULL || *error == NULL, NULL);

    if (fp) {
        gint    n       = 0;
        GError* tmp_err = NULL;
        gchar** str_arr = eh_scan_str_array(fp, &n, &tmp_err);

        if (str_arr) {
            gint i;
            n = g_strv_length(str_arr);

            b_arr = eh_new(gboolean, n);

            for (i = 0 ; i < n && !tmp_err ; i++) {
                b_arr[i] = eh_str_to_boolean(str_arr[i], &tmp_err);
            }

            g_strfreev(str_arr);

            if (tmp_err) {
                eh_free(b_arr);
                b_arr = NULL;
                n     = 0;
            }
        }

        if (tmp_err) {
            g_propagate_error(error, tmp_err);
            b_arr = NULL;
            n     = 0;
        }

        if (len) {
            *len = n;
        }
    }

    return b_arr;
}
/*
gint
read_double_vector(FILE *fp,double *val,int len)
{
   char **str_vec=NULL;
   gint i=0;

   str_vec=read_string_vector(fp,len);
   if ( str_vec )
      for (i=0;str_vec[i]!=NULL;i++)
         sscanf(str_vec[i],"%lf",&val[i]);
   g_strfreev( str_vec );

   return i;
}

gint
read_time_vector( FILE *fp , double *val , int len )
{
   char **str_vec=NULL;
   gint i=0;

   str_vec=read_string_vector(fp,len);
   if ( str_vec )
      for ( i=0 ; str_vec[i]!=NULL ; i++ )
         val[i] = eh_str_to_time_in_years( str_vec[i] );
   g_strfreev( str_vec );
   return i;
}

gint
read_int_vector(FILE *fp,int *val,int len)
{
   char **str_vec=NULL;
   gint i=0;

   str_vec=read_string_vector(fp,len);
   if ( str_vec )
      for (i=0;str_vec[i]!=NULL;i++)
         sscanf(str_vec[i],"%d",&(val[i]));
   g_strfreev( str_vec );

   return i;
}

#undef DATA_DELIMETER

double **read_matrix(FILE *fp,int nColumns,int *nRows)
{
   double **matrix=NULL;
   double *row;
   int i, j;

//   row = (double*)malloc1D(sizeof(double)*nColumns);
   row = eh_new( double , nColumns );

   // See how many rows there will be.
   for (i=0; read_double_vector(fp,row,nColumns) ;i++);
   rewind(fp);
   *nRows = i;

//   matrix = (double**)malloc2Ddouble(nColumns,*nRows);
   matrix = eh_new_2( double , nColumns , *nRows );
   for (i=0; read_double_vector(fp,row,nColumns) ;i++)
      for (j=0;j<nColumns;j++)
         matrix[j][i] = row[j];

   return matrix;
}

#include <string.h>

gboolean
read_logical_value( FILE *fp )
{
   char str[S_LINEMAX];
   read_string_value(fp,str);
   eh_str_trim_left(eh_str_trim_right(str));
   if (    g_ascii_strcasecmp( str , "ON"  ) == 0
        || g_ascii_strcasecmp( str , "YES" ) == 0 )
      return TRUE;
   if (    g_ascii_strcasecmp( str , "OFF" ) == 0
        || g_ascii_strcasecmp( str , "NO"  ) == 0 )
      return FALSE;
   else
   {
      eh_info( "read_logical_value: unknown flag -- %s\n" , str );
      eh_info( "read_logical_value: valid options are: 'ON', or 'OFF'.\n");
      eh_exit(-1);
   }

   eh_require_not_reached();
   return FALSE;

}

*/
void
eh_print_msg(int msg_level, char* function_name, char* msg)
{

    g_strchomp(msg);

    switch (msg_level) {
        case 0:
            break;

        case 1:
            fprintf(stderr, "%s: %s: %s\n", "warning", function_name, msg);
            break;

        case 2:
            fprintf(stderr, "%s: %s: %s\n", "error", function_name, msg);
            fprintf(stderr, "quitting...\n");
            eh_exit(-1);
    }

}

char*
eh_input_str(const char* msg, const char* default_str)
{
    char* str = eh_new(char, S_LINEMAX);

    //---
    // Print the message followed by the default string in [].
    //---
    fprintf(stderr, "%s ", msg);

    if (default_str) {
        fprintf(stderr, "[%s] ", default_str);
    }

    fprintf(stderr, ": ");

    //---
    // Read a line of input.  If the line is blank, use the default string.
    // Remove leading and trailing whitespace from the string.
    //---
    fgets(str, S_LINEMAX, stdin);

    if (default_str && strncmp(str, "\n", 1) == 0) {
        strcpy(str, default_str);
    }

    g_strstrip(str);

    return str;
}

gboolean
eh_input_boolean(const char* msg, gboolean default_val)
{
    char* str = eh_new(char, S_LINEMAX);
    gboolean ans, valid_ans = FALSE;

    while (!valid_ans) {
        fprintf(stderr, "%s [%s]: ", msg, (default_val) ? "yes" : "no");

        fgets(str, S_LINEMAX, stdin);

        if (g_ascii_strncasecmp(str, "\n", 1) == 0) {
            ans = default_val;
            valid_ans = TRUE;
        } else if (g_ascii_strncasecmp(str, "YES", 2) == 0
            || g_ascii_strncasecmp(str, "Y", 1) == 0) {
            ans = TRUE;
            valid_ans = TRUE;
        } else if (g_ascii_strncasecmp(str, "NO", 2) == 0
            || g_ascii_strncasecmp(str, "N", 1) == 0) {
            ans = FALSE;
            valid_ans = TRUE;
        }
    }

    eh_free(str);

    return ans;
}

gchar*
eh_get_input_val(FILE* fp, const char* msg, const char* default_str)
{
    char* str = eh_new(char, S_LINEMAX);

    fprintf(stderr, "%s ", msg);

    if (default_str) {
        fprintf(stderr, "[%s] ", default_str);
    }

    fprintf(stderr, ": ");

    fgets(str, S_LINEMAX, fp);

    if (default_str && strncmp(str, "\n", 1) == 0) {
        strcpy(str, default_str);
    }

    g_strstrip(str);
    return str;
}
