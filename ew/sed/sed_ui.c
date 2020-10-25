#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include "etk_keyvalue.h"
#include "etk_addrem.h"

void
new_sim(void)
{
    g_printf("Create a new simulation\n");
}

void
open_sim(void)
{
    g_printf("Open an existing simulation\n");
}

void
about_sedflux(void)
{
    g_printf("Sedflux 2.0 by Eric\n");
}

void
quit_prog(void)
{
    g_printf("Quitting...");

    gtk_main_quit();
}

static GtkItemFactoryEntry menu_items[] = {
    { "/_File", NULL, NULL, 0, "<Branch>" },
    { "/_File/_New", "<CTRL>N", new_sim, 0, "<Item>" },
    { "/_File/_Open", "<CTRL>O", open_sim, 0, "<Item>" },
    { "/_File/sep1", NULL, NULL, 0, "<Separator>" },
    { "/_File/_Quit", "<CTRL>Q", quit_prog, 0, "<StockItem>", GTK_STOCK_QUIT },
    { "/_Help", NULL, NULL, 0, "<LastBranch>" },
    { "/_Help/_About", NULL, about_sedflux, 0, "<Item>" }
};

static gint n_items = sizeof(menu_items) / sizeof(menu_items[0]);

static GtkWidget*
get_menubar_menu(GtkWidget* window)
{
    GtkAccelGroup* accel_group = gtk_accel_group_new();
    GtkItemFactory* i_factory  = gtk_item_factory_new(GTK_TYPE_MENU_BAR,
            "<SedfluxMain>",
            accel_group);

    gtk_item_factory_create_items(i_factory, n_items, menu_items, NULL);

    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
    /*
       {
          GtkWidget* about_dialog = gtk_about_dialog_new();

          gtk_about_dialog_set_name   ( about_dialog , "Sedflux" );
          gtk_about_dialog_set_version( about_dialog , "2.0"     );
          gtk_about_dialog_set_authors( about_dialog , "Eric Hutton" );
       }
    */

    return gtk_item_factory_get_widget(i_factory, "<SedfluxMain>");
}

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

static GtkWidget*
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

static GtkWidget*
get_global_parameters(GtkWidget* window)
{
    GtkWidget* frame         = gtk_frame_new("Resolution");

    {
        GtkWidget* v_box    = gtk_vbox_new(FALSE, 2);
        GtkWidget* message  = gtk_text_view_new();

        {
            GtkTextBuffer* buff = gtk_text_buffer_new(NULL);

            gtk_text_buffer_set_text(buff, "Specify the spatial resolution for the simulation.\0",
                -1);

            gtk_text_view_set_buffer(GTK_TEXT_VIEW(message), buff);
            gtk_text_view_set_editable(GTK_TEXT_VIEW(message), FALSE);

            gtk_box_pack_start(GTK_BOX(v_box), message, TRUE, TRUE, 2);
        }

        {
            GtkWidget* v_res = key_value_new("Vertical resolution (m)");
            GtkWidget* x_res = key_value_new("Along-shore width (m)");
            GtkWidget* y_res = key_value_new("Cross shore resolution (m)");

            gtk_box_pack_start(GTK_BOX(v_box), v_res, TRUE, TRUE, 2);
            gtk_box_pack_start(GTK_BOX(v_box), x_res, TRUE, TRUE, 2);
            gtk_box_pack_start(GTK_BOX(v_box), y_res, TRUE, TRUE, 2);

        }

        gtk_container_set_border_width(GTK_CONTAINER(v_box), 10);

        gtk_container_add(GTK_CONTAINER(frame), v_box);
    }

    gtk_frame_set_label_align(GTK_FRAME(frame), 1., .5);
    gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

    return frame;
}

void
add_to_list(gpointer user_data)
{
    //GtkWidget* combo_box = GTK_WIDGET( user_data );
    return;

}

GtkWidget*
get_suspended_sediment_box(GList* items)
{
    GtkWidget* h_box = gtk_hbox_new(TRUE, 2);
    GtkWidget* dest_list_view;
    GtkWidget* arrow_box = gtk_vbox_new(FALSE, 2);
    GtkWidget* src_list_view;

    // Destination list
    {
        GtkListStore* dest_list = gtk_list_store_new(1, G_TYPE_STRING);
        GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn* col;
        GtkTreeSelection* sel;
        /*
              {
                 GList* this_item = items;

                 for ( ; this_item ; this_item = this_item->next  )
                 {
                    gtk_list_store_append( dest_list , &iter );
                    gtk_list_store_set( dest_list , &iter , 0 , this_item->data , -1 );
                 }
              }
        */
        dest_list_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(dest_list));
        col = gtk_tree_view_column_new_with_attributes("Sediment Name",
                renderer,
                "text", 0, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(dest_list_view), col);
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(dest_list_view), FALSE);

        sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(dest_list_view));
        gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
    }

    // left/right arrows
    {
        GtkWidget* right = gtk_button_new_from_stock(GTK_STOCK_GO_FORWARD);
        GtkWidget* left  = gtk_button_new_from_stock(GTK_STOCK_GO_BACK);
        GtkWidget* new    = gtk_button_new_from_stock(GTK_STOCK_NEW);

        gtk_box_pack_start(GTK_BOX(arrow_box), right, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(arrow_box), left, FALSE, FALSE, 2);
        gtk_box_pack_end(GTK_BOX(arrow_box), new, FALSE, FALSE, 2);
    }

    // Source list
    {
        GtkListStore* src_list = gtk_list_store_new(1, G_TYPE_STRING);
        GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
        GtkTreeIter iter;
        GtkTreeViewColumn* col;
        GtkTreeSelection* sel;

        {
            GList* this_item = items;

            for (; this_item ; this_item = this_item->next) {
                gtk_list_store_append(src_list, &iter);
                gtk_list_store_set(src_list, &iter, 0, this_item->data, -1);
            }
        }

        src_list_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(src_list));
        col = gtk_tree_view_column_new_with_attributes("Sediment Name",
                renderer,
                "text", 0, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(src_list_view), col);
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(src_list_view), FALSE);

        sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(src_list_view));
        gtk_tree_selection_set_mode(sel, GTK_SELECTION_MULTIPLE);
    }

    gtk_box_pack_start(GTK_BOX(h_box), dest_list_view, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(h_box), arrow_box, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(h_box), src_list_view, TRUE, TRUE, 2);

    return h_box;
}

static GtkWidget*
get_sediment(GtkWidget* window)
{
    GtkWidget* frame = gtk_frame_new("Sediment");
    GtkWidget* v_box = gtk_vbox_new(FALSE, 2);
    GList* sediment_items = NULL;

    gtk_frame_set_label_align(GTK_FRAME(frame), 1., .5);

    sediment_items = g_list_append(sediment_items, "Coarse sand");
    sediment_items = g_list_append(sediment_items, "Sand");
    sediment_items = g_list_append(sediment_items, "Fine sand");
    sediment_items = g_list_append(sediment_items, "Coarse silt");
    sediment_items = g_list_append(sediment_items, "Silt");
    sediment_items = g_list_append(sediment_items, "Fine silt");
    sediment_items = g_list_append(sediment_items, "Coarse clay");
    sediment_items = g_list_append(sediment_items, "clay");
    sediment_items = g_list_append(sediment_items, "Fine Clay");

    // Create a frame to choose bedload.
    {
        GtkWidget* bed_frame = gtk_frame_new("Bedload");
        GtkWidget* bed_select = gtk_combo_new();

        gtk_container_set_border_width(GTK_CONTAINER(bed_select), 10);

        gtk_frame_set_label_align(GTK_FRAME(bed_frame), 1., .5);

        gtk_combo_set_popdown_strings(GTK_COMBO(bed_select), sediment_items);

        gtk_container_add(GTK_CONTAINER(bed_frame), bed_select);
        gtk_container_set_border_width(GTK_CONTAINER(bed_frame), 5);

        gtk_box_pack_start(GTK_BOX(v_box), bed_frame, TRUE, TRUE, 2);
    }

    // Create a frame to choose suspended load.
    {
        GtkWidget* susp_frame = gtk_frame_new("Suspended Load");
        GtkWidget* susp_box   = get_suspended_sediment_box(sediment_items);

        gtk_container_set_border_width(GTK_CONTAINER(susp_box), 10);

        gtk_frame_set_label_align(GTK_FRAME(susp_frame), 1., .5);

        gtk_container_set_border_width(GTK_CONTAINER(susp_frame), 5);
        gtk_container_add(GTK_CONTAINER(susp_frame), susp_box);

        /*
              {
                 GtkWidget* select_box = gtk_hbox_new( FALSE , 2 );
                 GtkWidget* susp_select = gtk_combo_new();
                 GtkWidget* add_button = gtk_button_new_from_stock( GTK_STOCK_ADD );

                 g_signal_connect( G_OBJECT( add_button ) , "clicked" , G_CALLBACK(add_to_list) , susp_select );

                 gtk_combo_set_popdown_strings( GTK_COMBO(susp_select) , sediment_items );

                 gtk_box_pack_start( GTK_BOX( select_box ) , susp_select , TRUE , TRUE , 2 );
                 gtk_box_pack_start( GTK_BOX( select_box ) , add_button  , TRUE , TRUE , 2 );

                 gtk_box_pack_start( GTK_BOX( susp_v_box ) , select_box , TRUE , TRUE , 2 );
              }
        */

        gtk_box_pack_start(GTK_BOX(v_box), susp_frame, TRUE, TRUE, 2);
    }

    gtk_container_add(GTK_CONTAINER(frame), v_box);
    gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

    return frame;
}

static void
destroy(GtkWidget* widget, gpointer data)
{
    gtk_main_quit();
}

void
add(GtkWidget* w, gpointer data)
{
    g_print("We should be adding another entry after this one.\n");
}

int
main(int   argc,
    char* argv[])
{
    GtkWidget* window;
    GtkWidget* main_vbox;
    GtkWidget* menubar;
    GtkWidget* bathy_entry;
    GtkWidget* global_entry;
    GtkWidget* sediment_entry;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(window), "destroy",
        G_CALLBACK(destroy), NULL);
    gtk_window_set_title(GTK_WINDOW(window), "sedflux 2.0");
    gtk_widget_set_size_request(GTK_WIDGET(window), 600, 750);

    main_vbox = gtk_vbox_new(FALSE, 1);
    gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 1);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    menubar        = get_menubar_menu(window);
    bathy_entry    = get_bathymetry_file(window);
    global_entry   = get_global_parameters(window);
    sediment_entry = get_sediment(window);

    gtk_box_pack_start(GTK_BOX(main_vbox), menubar, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), bathy_entry, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), global_entry, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), sediment_entry, FALSE, TRUE, 0);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

