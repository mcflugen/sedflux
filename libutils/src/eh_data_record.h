
#if !defined( EH_DATA_RECORD_H )
#define EH_DATA_RECORD_H

#include <glib.h>
#include "eh_types.h"
#include "eh_symbol_table.h"

#define EH_FAST_DIM_ROW (0)
#define EH_FAST_DIM_COL (1)

new_handle( Eh_data_record );

Eh_data_record  eh_data_record_new             ( void );
Eh_data_record  eh_data_record_destroy         ( Eh_data_record p );
void            eh_data_record_print           ( Eh_data_record p     ,
                                                 char *rec_name       ,
                                                 char *delim          ,
                                                 gboolean row_major   ,
                                                 gboolean with_header ,
                                                 FILE *fp );
int             eh_data_record_size            ( Eh_data_record p     ,
                                                 int dim );
Eh_symbol_table eh_data_record_table           ( Eh_data_record p );
double*         eh_data_record_row             ( Eh_data_record p     ,
                                                 gssize row );
double*         eh_data_record_dup_row         ( Eh_data_record p     ,
                                                 gssize row );
void            eh_data_record_set_row         ( Eh_data_record p     ,
                                                 int row              ,
                                                 double* a );
void            eh_data_record_add_row         ( Eh_data_record p     ,
                                                 double* a );
void            eh_data_record_add_column      ( Eh_data_record p     ,
                                                 double* a );
void            eh_data_record_add_label       ( Eh_data_record p     ,
                                                 char *label          ,
                                                 char *value );
void            eh_data_record_interpolate_rows( Eh_data_record p     ,
                                                 gssize row           ,
                                                 double* y            ,
                                                 gssize new_len );
Eh_data_record* eh_data_record_scan_file       ( const char* file     ,
                                                 const char* delim    ,
                                                 int fast_dim         ,
                                                 gboolean with_header ,
                                                 GError** error );
Eh_data_record  eh_data_record_scan            ( GScanner* s          ,
                                                 const char* delim    ,
                                                 int fast_dim         ,
                                                 gboolean with_header );

#endif

