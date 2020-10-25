#include <gtk/gtk.h>
#include "etk_keyvalue.h"
#include "etk_addrem.h"

void
store_filename(GtkWidget* widget, GtkWidget* user_data[2])
{
    GtkWidget* file_selector = GTK_WIDGET(user_data[0]);
    GtkWidget* entry         = GTK_WIDGET(user_data[1]);
    const gchar* filename;

    filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_selector));
    gtk_entry_set_text(GTK_ENTRY(entry), filename);

    g_print("Selected filename: %s\n", filename);
}

void
create_file_selection(gpointer user_data)
{
    GtkWidget* entry = GTK_WIDGET(user_data);
    GtkWidget* file_selector;
    static GtkWidget* data[2];

    file_selector = gtk_file_selection_new("Please select a file.");

    data[0] = file_selector;
    data[1] = entry;

    g_signal_connect(GTK_FILE_SELECTION(file_selector)->ok_button,
        "clicked",
        G_CALLBACK(store_filename),
        data);

    g_signal_connect_swapped(GTK_FILE_SELECTION(file_selector)->ok_button,
        "clicked",
        G_CALLBACK(gtk_widget_destroy),
        file_selector);
    g_signal_connect_swapped(GTK_FILE_SELECTION(file_selector)->cancel_button,
        "clicked",
        G_CALLBACK(gtk_widget_destroy),
        file_selector);

    gtk_widget_show(file_selector);
}

GtkWidget*
get_bathymetry_file(GtkWidget* window)
{
    GtkWidget* frame         = gtk_frame_new("Initial Bathymetry");

    {
        GtkWidget* v_box    = gtk_vbox_new(FALSE, 2);
        GtkWidget* h_box    = gtk_hbox_new(FALSE, 2);
        GtkWidget* message  = gtk_text_view_new();

        {
            GtkTextBuffer* buff = gtk_text_buffer_new(NULL);

            gtk_text_buffer_set_text(buff, "This is a test.\n\n   An indented line.\0", -1);

            gtk_text_view_set_buffer(GTK_TEXT_VIEW(message), buff);
            gtk_text_view_set_editable(GTK_TEXT_VIEW(message), FALSE);
        }

        {
            GtkWidget* entry = gtk_entry_new();
            GtkWidget* choose_button = gtk_button_new_with_label("Choose...");

            gtk_box_pack_start(GTK_BOX(h_box), entry, TRUE, TRUE, 2);
            gtk_box_pack_end(GTK_BOX(h_box), choose_button, FALSE, FALSE, 2);

            g_signal_connect_swapped(G_OBJECT(choose_button),
                "clicked",
                G_CALLBACK(create_file_selection),
                entry);
            /*
                     g_signal_connect_object( G_OBJECT( choose_button ) ,
                                              "clicked" ,
                                              G_CALLBACK(create_file_selection) ,
                                              entry , G_CONNECT_AFTER|G_CONNECT_SWAPPED );
            */
        }

        gtk_box_pack_start(GTK_BOX(v_box), message, TRUE, TRUE, 2);
        gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 2);

        gtk_container_set_border_width(GTK_CONTAINER(v_box), 10);

        gtk_container_add(GTK_CONTAINER(frame), v_box);
    }

    gtk_frame_set_label_align(GTK_FRAME(frame), 1., .5);
    gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

    return frame;
}

