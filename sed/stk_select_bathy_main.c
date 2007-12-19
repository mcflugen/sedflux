#include <gtk/gtk.h>

void store_filename( GtkWidget* widget , GtkWidget* user_data[2] );
void create_file_selection( gpointer user_data );
GtkWidget* get_bathymetry_file( GtkWidget* window );

static void destroy( GtkWidget* widget , gpointer data )
{
   gtk_main_quit();
}

int main( int   argc,
          char *argv[] )
{
   GtkWidget *window;
   GtkWidget *main_vbox;
   GtkWidget *bathy_entry;
    
   gtk_init (&argc, &argv);
    
   window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
   g_signal_connect (G_OBJECT (window), "destroy",
                     G_CALLBACK (destroy), NULL);
   gtk_window_set_title( GTK_WINDOW(window) , "sedflux 2.0" );
   gtk_widget_set_size_request( GTK_WIDGET(window) , 600 , 750 );
    
   main_vbox = gtk_vbox_new( FALSE , 1 );
   gtk_container_set_border_width( GTK_CONTAINER(main_vbox) , 1 );
   gtk_container_add( GTK_CONTAINER(window) , main_vbox );

   bathy_entry    = get_bathymetry_file( window );

   gtk_box_pack_start( GTK_BOX(main_vbox) , bathy_entry , FALSE , TRUE , 0 );

   gtk_widget_show_all( window );

   gtk_main ();
    
   return 0;
}

