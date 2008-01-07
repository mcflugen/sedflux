#include <eh_utils.h>

CLASS ( Eh_data_record )
{
   Eh_symbol_table t;
   Eh_dbl_grid g;
};

Eh_data_record eh_data_record_new()
{
   Eh_data_record p;

   NEW_OBJECT( Eh_data_record , p );

   p->t = eh_symbol_table_new();
   p->g = eh_grid_new( double , 0 , 0 );

   return p;
}

Eh_data_record eh_data_record_destroy( Eh_data_record p )
{
   if ( p )
   {
      eh_symbol_table_destroy( p->t );
      eh_grid_destroy( p->g , TRUE );
      eh_free( p );
   }

   return NULL;
}

void eh_data_record_print( Eh_data_record p     ,
                           char *rec_name       ,
                           char *delim          ,
                           gboolean row_major   ,
                           gboolean with_header ,
                           FILE *fp )
{
   if ( with_header )
   {
      fprintf( fp , "--- %s ---\n" , rec_name );
      eh_symbol_table_print_aligned( p->t , fp );
      fprintf(fp,"--- data ---\n");
   }

   {
      gchar* format = g_strconcat( "%f" , delim , " " , NULL );
   
      if ( row_major )
         eh_dbl_grid_fprintf( fp , format , p->g );
      else
      {
         eh_dbl_grid_fprintf( fp , format , eh_grid_transpose(p->g) );
         eh_grid_transpose( p->g );
      }

      eh_free( format );
   }
}

int eh_data_record_size( Eh_data_record p , int dim )
{
   if ( p && p->g && (dim==0||dim==1) )
   {
      if ( dim==0 )
         return eh_grid_n_x(p->g);
      else if ( dim==1 )
         return eh_grid_n_y(p->g);
   }
   
   return 0;
}

Eh_symbol_table eh_data_record_table( Eh_data_record p )
{
   return p->t;
}

double* eh_data_record_row( Eh_data_record p , gssize row )
{
   if ( p && row < eh_grid_n_x(p->g) && row >=0 )
      return eh_grid_row(p->g,row);

   return NULL;
}

double* eh_data_record_dup_row( Eh_data_record p , gssize row )
{
   return g_memdup( eh_data_record_row(p,row) , eh_data_record_size(p,1)*sizeof(double) );
   
}

void eh_data_record_set_row( Eh_data_record p , int row , double* a )
{
   if ( p && row<eh_grid_n_x(p->g) && a )
      g_memmove( eh_data_record_row(p,row) , a , eh_grid_el_size(p->g)*eh_grid_n_y(p->g) );
}

void eh_data_record_add_row( Eh_data_record p , double* a )
{
   eh_grid_add_row( p->g , a );
}

void eh_data_record_add_column( Eh_data_record p , double* a )
{
   eh_grid_add_column( p->g , a );
}

void eh_data_record_add_label( Eh_data_record p , char *label , char *value )
{
   eh_symbol_table_insert( p->t , label , value );
}

void eh_data_record_interpolate_rows( Eh_data_record p , gssize row , double* y , gssize new_len )
{
   eh_require( p          );
   eh_require( row>=0     );
   eh_require( row<eh_grid_n_x(p->g) );
   eh_require( y          );
   eh_require( new_len>0  );

   if ( p )
   {
      gssize i;
      double* y_0 = eh_data_record_row( p , row );
      Eh_dbl_grid new_grid = eh_grid_new( double , eh_grid_n_x(p->g) , new_len );

      for ( i=0 ; i<eh_grid_n_x(p->g) ; i++ )
      {
         if ( i!=row )
            interpolate( y_0 , eh_grid_row(p->g,i)     , eh_grid_n_y(p->g) ,
                         y   , eh_grid_row(new_grid,i) , new_len );
      }

      eh_grid_destroy( p->g , TRUE );
      p->g = new_grid;
   }
}

Eh_data_record*
eh_data_record_scan_file( const char* file , const char* delim , int fast_dim , gboolean with_header , GError** error )
{
   Eh_data_record* all_records = NULL;
   GError*         tmp_err     = NULL;
   GScanner*       s           = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   s = eh_open_scanner( file , &tmp_err );

   if ( s )
   {
      gssize n_recs = 0;
      Eh_data_record next_record = eh_data_record_scan( s , delim , fast_dim , with_header );

      while ( next_record )
      {
         n_recs++;
         all_records = eh_renew( Eh_data_record , all_records , n_recs+1 );
         all_records[n_recs-1] = next_record;
         next_record = eh_data_record_scan( s , delim , fast_dim , with_header );
      }
      if ( all_records )
         all_records[n_recs] = NULL;
   }
   else
      g_propagate_error( error , tmp_err );

   eh_close_scanner( s );

   return all_records;
}

Eh_data_record eh_data_record_scan( GScanner* s , const char* delim , int fast_dim , gboolean with_header )
{
   Eh_data_record ans;

   if ( !g_scanner_scope_lookup_symbol(s,0,"---") )
      g_scanner_scope_add_symbol(s,0,"---",g_strdup("---") );

   ans = eh_data_record_new();

   if ( with_header )
   {
      char *record_name = eh_scan_next_record( s , ans->t );
      if ( !record_name )
         return eh_data_record_destroy( ans );
      eh_free( record_name );
   }

   eh_debug( "Find the next record" );
   {
      char *record_name = eh_seek_record_start(s);
      if ( !record_name )
         return eh_data_record_destroy( ans );
      eh_free( record_name );
   }

   eh_debug( "Read the input file line by line" );
   {
      gssize n_cols;
      double* row = eh_scan_ascii_data_line_dbl( s , delim , &n_cols );

      eh_grid_resize( ans->g , 0 , n_cols );

      while ( row )
      {
         eh_grid_add_row( ans->g , row );
         eh_free( row );

         row = eh_scan_ascii_data_line_dbl( s , delim , &n_cols );
      }
   }

   eh_debug( "Switch rows and columns if necessary" );
   if ( fast_dim==EH_FAST_DIM_COL )
      eh_grid_transpose( ans->g );

   return ans;
}

