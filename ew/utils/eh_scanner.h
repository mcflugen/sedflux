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

#ifndef __EH_SCANNER_H__
#define __EH_SCANNER_H__

#include <glib.h>
#include <utils/eh_symbol_table.h>
#include <utils/eh_data_record.h>

//typedef GHashTable Symbol_table;

#ifdef OLD
typedef struct
{
   GHashTable *symbol_table;
   GPtrArray *data;
}
Eh_data_record;

typedef struct
{
   char *filename;
   GHashTable *records; // a table of Eh_data_records
}
Eh_record_file;
#endif

typedef struct
{
   char *filename;
   char *delimeter;
   gboolean row_major;
   gint fast_dim;
   gboolean with_header;
   Eh_data_record* records;
//   GPtrArray *records; // an array of Eh_data_records
}
Eh_data_file;

typedef struct
{
   char *delimeter;
   gint fast_dim;
   gboolean row_major;
   gboolean with_header;
}
Eh_data_file_attr;

// create a symbol table.
// return value : the newly created symbol table.
//Symbol_table *eh_create_symbol_table(void);

// insert a key/value pair into a symbol table.
// t            : a pointer to a symbol table.
// key          : a string containing the key.
// value        : a string containing the value.
// return value : the newly created symbol table.
//void eh_symbol_table_insert(Symbol_table *t,char *key, char *value);

// replace a key/value pair into a symbol table.
// t            : a pointer to a symbol table.
// key          : a string containing the key.
// value        : a string containing the value.
// return value : the newly created symbol table.
//void eh_symbol_table_replace( Symbol_table* t , char* key , char* value );

// lookup up a symbol in a symbol table.
// t            : a pointer to a symbol table.
// key          : a string containing the key to lookup.
// return value : a string containint the value of the key.
//char *eh_symbol_table_lookup(Symbol_table *t,char *key);

// print a key/value pair.  intended to be used as a GHFunc.
// key          : a key string.
// value        : a value string.
// fp           : file pointer to print to.
// return value : nothing.
void eh_print_symbol(char *key, char  *value, FILE *fp);

// print all of the key/value pairs of a symbol table.
// t            : a pointer to a symbol table.
// fp           : file pointer to print to.
// return value : nothing.
//void eh_print_symbol_table( Symbol_table *t , FILE *fp);

// print all of the key/value pairs of a symbol table and align them.
// t            : a pointer to a symbol table.
// fp           : file pointer to print to.
// return value : nothing.
//void eh_print_symbol_table_aligned( Symbol_table *t , FILE *fp);

// get the number of entries in a symbol table
// t            : a pointer to a symbol table.
// return value : the number of entries.
//gssize eh_symbol_table_size( Symbol_table *t );

// destroy a symbol table.
// t            : a pointer to a symbol table to destroy.
// return value : nothing.
//Symbol_table* eh_destroy_symbol_table(Symbol_table*);

//void eh_free_symbol_table_labels( Symbol_table *t );

//Symbol_table* eh_symbol_table_dup( Symbol_table* t );
//Symbol_table* eh_symbol_table_copy( Symbol_table* dest , Symbol_table* src );

// merge a bunch of symbol tables into one.
// table1       : the first symbol table to add, which must not be null.
// ...          : a NULL terminated list of symbol tables to merge.
// return value : a newly create symbol table composed of all the symbol table arguments.
//Symbol_table *eh_merge_symbol_table( Symbol_table* table1 , ... );

GScanner* eh_open_scanner_text( const gchar* text , gint len , GError** error );

// open a scanner file.
// filename     : the name of the file to open.
// return value : the opened scanner.
GScanner *eh_open_scanner(const char *filename , GError** error );

void eh_close_scanner( GScanner* );

// scan a data file.
// filename     : the name of the data file to scan.
// delimeter    : the delimeter to use to split a row of values.
// row_major    : if TRUE, the data is read in row major order.
// with_header  : if TRUE, the data is preceded by a header.
// return value : a pointer array containing the data.
GPtrArray *eh_scan_data_file(const char *filename,const char *delimiter,gboolean row_major, gboolean with_header);

// scan a data record.
// s            : a scanner set up to read the data.  created with eh_open_scanner.
// delimeter    : the delimeter to use to split a row of values.
// row_major    : if TRUE, the data is read in row major order.
// data         : a pointer array to put the data.
// return value : a pointer array of the data.
GPtrArray *eh_scan_data_record(GScanner *s,const char *delimeter, gboolean row_major, GPtrArray *data);

double* eh_scan_ascii_data_line_dbl( GScanner* s , const char* delimeter ,  gssize *len );

// scan a line of ascii data.
// s            : a scanner set up to read the data.  created with eh_open_scanner.
// return value : a string of the line of data.
char *eh_scan_ascii_data_line(GScanner *s);

// scan the records of a record file.
// filename     : the name of the file to scan.
// return value : a hash table with a symbol table entry for each record.
//GHashTable *eh_scan_record_file(const char *filename);

// scan a record file for a specific record.
// filename     : the name of the file to scan.
// rec_name     : the name of the record to search for.
// return value : a symbol table representation of the record.
//Symbol_table *eh_scan_record_file_for( const char *filename , const char *rec_name );

// scan a key file for a specific group.
// file         : the name of the file to scan.
// grp_name     : the name of the group to search for.
// tab          : if non-NULL, store the key-value pairs in tab.  If NULL, create a new symbol table.
// return value : a symbol table representation of the record.
//Symbol_table* eh_scan_key_file_for( const gchar* file , const gchar* grp_name , Symbol_table *tab );

// scan the next record from a scanner.
// s            : a scanner set up to read the data.  created with eh_open_scanner.
// symbol_table : a symbol table to add the record entries to.
// return value : the name of the record.
char *eh_scan_next_record(GScanner *s,Eh_symbol_table symbol_table);

// move to the begining of the data of the next record.
// s            : a scanner set up to read the data.  created with eh_open_scanner.
// return value : the name of the record.
char *eh_seek_record_start(GScanner *s);

// scan a label from a scanner.
// s            : a scanner set up to read the data.  created with eh_open_scanner.
// return value : a string containing the label read.
char *eh_scan_label(GScanner *s);

// scan a entry from a scanner.
// s            : a scanner set up to read the data.  created with eh_open_scanner.
// return value : a string containing the entry read.
char *eh_scan_entry(GScanner *s);

// print the symbol tables of a list of symbol tables. intended to be used as a GHFunc.
// rec_name     : the name of the record.
// t            : the symbol table to print.
// fp           : the file to print to.
// return value : nothing.
//void eh_print_record( const char *rec_name , Symbol_table *t , FILE *fp );

// indicate if the end of a record has been reached.
// s            : a scanner set up to read the data.  created with eh_open_scanner.
// return value : TRUE if the end of a record has been encountered.
gboolean eh_scanner_eor(GScanner *s);

// create a data record.
// return value : the newly created data record.
//Eh_data_record *eh_create_data_record();

// free the memory allocated for a data record.
// p            : the data record to destroy.
// return value : nothing.
//void eh_free_data_record(Eh_data_record *p);

// print a data record to a file.
// p            : a pointer to a data record.
// rec_name     : the name of the record.
// delimeter    : a pointer to the delimeter.
// row_major    : TRUE if the data is written row by row.
// with_header  : TRUE if the data is preceded by a header.
// fp           : a pointer to a file to print the record.
// record value : nothing.
//void eh_print_data_record( Eh_data_record *p , char *rec_name , char *delimeter , gboolean row_major , gboolean with_header , FILE *fp );

// get the size of a data record.
// p            : a pointer to a data record.
// i            : the dimension to get the size of.
// return value : the size of the data matrix in the specified dimension.
//int eh_get_data_record_size(Eh_data_record *p,int i);

// get the symbol table associated with the data record.
// p            : a pointer to a data record.
// return value : the symbol table of the data record.
//Symbol_table *eh_get_data_record_sym_table(Eh_data_record *p);

// get a row of data from a data file.
// p            : a pointer to a data record.
// row          : the row number of the data matrix to return.
// return value : an array of data values.
//GArray *eh_get_data_record_row(Eh_data_record *p,int row);

// insert an array into a data record.
// p            : a pointer to a data record.
// row          : the row number of the data matrix to add the array.
// a            : the array to add.
// return value : nothing.
//void eh_set_data_record_row( Eh_data_record *p , int row , GArray *a );

// add a label to the symbol table of a data record.
// p            : a pointer to a data record.
// a            : the array to add.
// return value : nothing.
//void eh_add_data_record_row( Eh_data_record *p , GArray *a );

// add a label to the symbol table of a data record.
// p            : a pointer to a data record.
// label        : the label to add.
// value        : the value to add.
// return value : nothing.
//void eh_add_data_record_label( Eh_data_record *p , char *label , char *value );

// get a pointer to a row of data.
// p            : a pointer to a data record.
// row          : the row number of the data matrix to return.
// type         : the type of data.
// return value : a pointer to the data.
/*
#define eh_get_data_record_row_ptr( p , row , type ) \
                                  ( ((type*)eh_get_data_record_row(p,row)->data) )
*/

// interpolate the rows of a data record.
// p            : a pointer to a data record.
// row          : the row to use to interpolate the rest of the data.
// x            : the array of points to interpolate to.
// return value : nothing.
//void eh_interpolate_data_record_rows( Eh_data_record *p , int row , GArray *x );

// open a record file.
// filename     : the name of the file (NULL for stdin);
// return value : pointer to the opened record file.
//Eh_record_file *eh_open_record_file( const char *filename );

// close a record file.
// rec_file     : pointer to a record file.
// return value : nothing.
//void eh_close_record_file( Eh_record_file *rec_file );

// get a record from a record file.
// rec_file     : pointer to a record file.
// name         : the name of the record to retrieve
// return value : a symbol table of the record.
//Symbol_table *eh_get_record_from_record_file( Eh_record_file *rec_file , const char *name );

// get a value from a record within a record a file.
// rec_file     : pointer to a record file.
// rec_name     : the name of the record containing the value.
// label        : the label for the value to retrieve.
// return value : the value as a string.
//char *eh_get_value_from_record_file( Eh_record_file *rec_file , const char *rec_name , const char *label );

// open a data file.
// filename     : the name of the file (NULL for stdin);
// attr         : attributes to use when opening a data file.  use NULL for defaults.
// return value : pointer to the opened data file.
Eh_data_file *eh_open_data_file( const char *filename , Eh_data_file_attr *attr , GError** error );

// close a data file.
// data_file    : pointer to a data file.
// return value : nothing.
void eh_close_data_file( Eh_data_file *data_file );

// get a row of data from a data file.
// name         : the name of the record to retrieve.
// return value : an array containing the row of data.
GArray *eh_get_row_from_data_file( Eh_data_file *data_file , int row );

// interpolate a row of data from a data file.
// data_file    : pointer to a data file.
// interp_data  : an array of data of interpolation points.
// return value : the interpolated row.
GArray *eh_interpolate_data_file( Eh_data_file *data_file , GArray *interp_data , int row );

// get the i-th data record from a data file.
// data_file    : pointer to a data file.
// i            : the record to get.
// return value : the i-th data record.
//Eh_data_record *eh_get_data_from_file( Eh_data_file *data_file , int i );

#endif
