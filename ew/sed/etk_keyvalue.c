#include "etk_keyvalue.h"

static void
key_value_init(Key_Value* kv);

static void
key_value_class_init(Key_Value_Class* kv_class)
{
    return;
}

GType
key_value_get_type(void)
{
    static GType kv_type = 0;

    if (!kv_type) {
        static const GTypeInfo kv_info = {
            sizeof(Key_Value_Class),
            NULL,
            NULL,
            (GClassInitFunc)key_value_class_init,
            NULL,
            NULL,
            sizeof(Key_Value),
            0,
            (GInstanceInitFunc)key_value_init
        };

        kv_type = g_type_register_static(GTK_TYPE_HBOX, "Key_Value", &kv_info, 0);
    }

    return kv_type;
}

static void
key_value_init(Key_Value* kv)
{
    gtk_box_set_homogeneous(GTK_BOX(kv), FALSE);

    kv->key_label   = gtk_label_new("");
    kv->value_entry = gtk_entry_new();

    gtk_misc_set_alignment(GTK_MISC(kv->key_label), 0, 1);
    gtk_entry_set_width_chars(GTK_ENTRY(kv->value_entry), 10);

    gtk_box_pack_start(GTK_BOX(kv), kv->key_label, TRUE, TRUE, 2);
    gtk_box_pack_end(GTK_BOX(kv), kv->value_entry, FALSE, FALSE, 2);

    gtk_widget_show(kv->key_label);
    gtk_widget_show(kv->value_entry);
}

const gchar*
key_value_get_name(Key_Value* kv)
{
    return gtk_label_get_text(GTK_LABEL(kv->key_label));
}

GtkWidget*
key_value_new(const char* key_label_str)
{
    Key_Value* kv_obj = g_object_new(KEY_VALUE_TYPE, NULL);

    gtk_label_set_text(GTK_LABEL(kv_obj->key_label), key_label_str);

    return GTK_WIDGET(kv_obj);
}

