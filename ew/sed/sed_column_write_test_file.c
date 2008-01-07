#include "sed_sedflux.h"

static gchar* order_str = NULL;

static GOptionEntry entries[] =
{
   { "byte-order" , 'o' , 0 , G_OPTION_ARG_STRING , &order_str , "Byte order of output" , "STR" } ,
   { NULL }
};

int main( int argc , char* argv[] )
{
   gssize n_bytes;
   gint byte_order;
   GError* error = NULL;
   GOptionContext* context;

   eh_init_glib();

   context = g_option_context_new( "Write a Sed_column test file." );
   g_option_context_add_main_entries( context , entries , NULL );
   if ( !g_option_context_parse( context , &argc , &argv , &error ) )
      eh_error( "Error parsing command line arguments: %s" , error->message );

   if      ( !order_str )
      byte_order = G_BYTE_ORDER;
   else if ( g_ascii_strcasecmp( order_str , "BIG-ENDIAN" ) )
      byte_order = G_BIG_ENDIAN;
   else if ( g_ascii_strcasecmp( order_str , "LITTLE-ENDIAN" ) )
      byte_order = G_LITTLE_ENDIAN;
   else
      eh_require_not_reached();

   sed_sediment_set_env( sed_sediment_scan(SED_SEDIMENT_TEST_FILE,NULL) );

   {
      Sed_column c = sed_column_new( 5 );
      Sed_cell cell = sed_cell_new_classed( NULL , 23.1 , S_SED_TYPE_SAND|S_SED_TYPE_MUD );

      sed_column_add_cell( c , cell );

      sed_column_set_z_res      ( c , 2.718 );
      sed_column_set_x_position ( c , 3.14  );
      sed_column_set_y_position ( c , 9.81  );
      sed_column_set_base_height( c , 1.414 );
      sed_column_set_age        ( c , 33.   );

      n_bytes = sed_column_write_to_byte_order( stdout , c , byte_order );

      sed_cell_destroy  ( cell );
      sed_column_destroy( c    );
   }

   return 0;
}

