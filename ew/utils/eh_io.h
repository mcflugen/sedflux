#ifndef __EH_IO_H__
#define __EH_IO_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <glib.h>

#if G_BYTE_ORDER==G_LITTLE_ENDIAN

gsize eh_fwrite_int32_to_be(const void* ptr, gsize size, gsize nitems, FILE* stream);
gsize eh_fwrite_int64_to_be(const void* ptr, gsize size, gsize nitems, FILE* stream);
gsize eh_fread_int32_from_be(void* ptr, gsize size, gsize nitems, FILE* stream);
gsize eh_fread_int64_from_be(void* ptr, gsize size, gsize nitems, FILE* stream);

#define eh_fwrite_int32_to_le  ( fwrite )
#define eh_fwrite_int64_to_le  ( fwrite )
#define eh_fread_int32_from_le ( fwrite )
#define eh_fread_int64_from_le ( fwrite )

#define eh_fwrite_dbl_to_be    ( eh_fwrite_int64_to_be  )
#define eh_fwrite_flt_to_be    ( eh_fwrite_int32_to_be  )
#define eh_fread_dbl_from_be   ( eh_fread_int64_from_be )
#define eh_fread_flt_from_be   ( eh_fread_int32_from_be )

#define eh_fwrite_int32_swap   ( eh_fwrite_int32_to_be  )
#define eh_fwrite_int64_swap   ( eh_fwrite_int64_to_be  )
#define eh_fread_int32_swap    ( eh_fread_int32_from_be )
#define eh_fread_int64_swap    ( eh_fread_int64_from_be )

#define eh_fread_flt_swap      ( eh_fread_int32_from_be )
#define eh_fread_dbl_swap      ( eh_fread_int64_from_be )
#define eh_fwrite_flt_swap     ( eh_fwrite_int32_to_be )
#define eh_fwrite_dbl_swap     ( eh_fwrite_int64_to_be )

#else

gsize eh_fwrite_int32_to_le(const void* ptr, gsize size, gsize nitems, FILE* stream);
gsize eh_fwrite_int64_to_le(const void* ptr, gsize size, gsize nitems, FILE* stream);
gsize eh_fread_int32_from_le(const void* ptr, gsize size, gsize nitems, FILE* stream);
gsize eh_fread_int64_from_le(const void* ptr, gsize size, gsize nitems, FILE* stream);

#define eh_fwrite_int32_to_be  ( fwrite )
#define eh_fwrite_int64_to_be  ( fwrite )
#define eh_fread_int32_from_be ( fwrite )
#define eh_fread_int64_from_be ( fwrite )

#define eh_fwrite_dbl_to_le    ( eh_fwrite_int64_to_le  )
#define eh_fwrite_flt_to_le    ( eh_fwrite_int32_to_le  )
#define eh_fread_dbl_from_le   ( eh_fread_int64_from_le )
#define eh_fread_flt_from_le   ( eh_fread_int32_from_le )

#define eh_fwrite_int32_swap   ( eh_fwrite_int32_to_le  )
#define eh_fwrite_int64_swap   ( eh_fwrite_int64_to_le  )
#define eh_fread_int32_swap    ( eh_fread_int32_from_le )
#define eh_fread_int64_swap    ( eh_fread_int64_from_le )

#define eh_fread_flt_swap      ( eh_fread_int32_from_le )
#define eh_fread_dbl_swap      ( eh_fread_int64_from_le )
#define eh_fwrite_flt_swap     ( eh_fwrite_int32_to_le )
#define eh_fwrite_dbl_swap     ( eh_fwrite_int64_to_le )

#endif

#if !defined( HAVE_GETLINE )
gssize getline(gchar** lineptr, gsize* n, FILE* stream);
#endif

gint read_double_vector(FILE* fp, double* val, int len);
gint read_time_vector(FILE* fp, double* val, int len);
gint read_int_vector(FILE* fp, int* val, int len);

gchar*    eh_scan_str(FILE* fp, GError** error);
gchar**   eh_scan_str_array(FILE* fp, gint* len, GError** error);
gint*     eh_scan_int_array(FILE* fp, gint* len, GError** error);
double*   eh_scan_dbl_array(FILE* fp, gint* len, GError** error);
gboolean* eh_scan_boolean_array(FILE* fp, gint* len, GError** error);

void     eh_print_msg(int   msg_level, char* function_name, char* msg);
gchar*   eh_input_str(const char* msg, const char* default_str);
gboolean eh_input_boolean(const char* msg, gboolean default_val);
gchar*   eh_get_input_val(FILE* fp, const char* msg, const char* default_str);

#ifdef __cplusplus
}
#endif

#endif
