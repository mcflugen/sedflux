#include <gtk/gtksignal.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "etk_addrem.h"

enum {
    ADD_REM_ADD_LINE,
    ADD_REM_REM_LINE,
    LAST_SIGNAL
};

static guint add_rem_signals[LAST_SIGNAL] = { 0 };

static void
add_rem_class_init(Add_Rem_Class* c);
static void
add_rem_init(Add_Rem* obj);

static void
add_rem_class_init(Add_Rem_Class* c)
{
    add_rem_signals[ADD_REM_ADD_LINE] = g_signal_new("add_line",
            G_TYPE_FROM_CLASS(c),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET(Add_Rem_Class, add_line),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    add_rem_signals[ADD_REM_REM_LINE] = g_signal_new("rem_line",
            G_TYPE_FROM_CLASS(c),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET(Add_Rem_Class, rem_line),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    return;
}

GType
add_rem_get_type(void)
{
    static GType this_type = 0;

    if (!this_type) {
        static const GTypeInfo this_type_info = {
            sizeof(Add_Rem_Class),
            NULL,
            NULL,
            (GClassInitFunc)add_rem_class_init,
            NULL,
            NULL,
            sizeof(Add_Rem),
            0,
            (GInstanceInitFunc)add_rem_init
        };

        this_type = g_type_register_static(GTK_TYPE_HBOX, "Add_Rem", &this_type_info, 0);
    }

    return this_type;
}

static void
rem_line(GtkWidget* w, Add_Rem* ar)
{
    g_printf("Remove this entry.\n");
    g_signal_emit(G_OBJECT(ar), add_rem_signals[ADD_REM_REM_LINE], 0);
}

static void
add_line(GtkWidget* w, Add_Rem* ar)
{
    g_printf("Add another entry.\n");
    g_signal_emit(G_OBJECT(ar), add_rem_signals[ADD_REM_ADD_LINE], 0);
}

static void
add_rem_init(Add_Rem* obj)
{
    obj->label      = GTK_WIDGET(gtk_label_new(""));
    obj->add_button = GTK_WIDGET(gtk_button_new_from_stock(GTK_STOCK_ADD));
    obj->rem_button = GTK_WIDGET(gtk_button_new_from_stock(GTK_STOCK_REMOVE));

    gtk_box_set_homogeneous(GTK_BOX(obj), FALSE);
    gtk_box_set_spacing(GTK_BOX(obj), 2);

    gtk_box_pack_start(GTK_BOX(obj), obj->label, TRUE, TRUE, 2);
    gtk_box_pack_end(GTK_BOX(obj), obj->rem_button, FALSE, FALSE, 2);
    gtk_box_pack_end(GTK_BOX(obj), obj->add_button, FALSE, FALSE, 2);

    g_signal_connect(G_OBJECT(obj->rem_button), "clicked",
        G_CALLBACK(rem_line), (gpointer)obj);
    g_signal_connect(G_OBJECT(obj->add_button), "clicked",
        G_CALLBACK(add_line), (gpointer)obj);

    gtk_widget_show(obj->add_button);
    gtk_widget_show(obj->rem_button);
    gtk_widget_show(obj->label);
}

const gchar*
add_rem_get_name(Add_Rem* ar)
{
    return gtk_label_get_text(GTK_LABEL(ar->label));
}

GtkWidget*
add_rem_new(const char* label_str)
{
    Add_Rem* obj = g_object_new(ADD_REM_TYPE, NULL);

    gtk_label_set_text(GTK_LABEL(obj->label), label_str);

    return GTK_WIDGET(obj);
}

