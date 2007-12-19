#ifndef __KEY_VALUE_H__
#define __KEY_VALUE_H__

#include <gdk/gdk.h>

#include <gtk/gtk.h>

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkhbox.h>

G_BEGIN_DECLS

#define KEY_VALUE_TYPE            ( key_value_get_type() )
#define KEY_VALUE( obj )          ( G_TYPE_CHECK_INSTANCE_CAST((obj),KEY_VALUE_TYPE,Key_Value) )
#define KEY_VALUE_CLASS( klass ) ( G_TYPE_CHECK_CLASS_CAST( (klass) , KEY_VALUE_TYPE , Key_Value_Class ) )
#define IS_KEY_VALUE( obj )       ( G_TYPE_CHECK_INSTANCE_CAST( (obj) , KEY_VALUE_TYPE  )
#define IS_KEY_VALUE_CLASS( klass ) (G_TYPE_CHECK_CLASS_TYPE( (klass) , KEY_VALUE_TYPE ) )

typedef struct _Key_Value       Key_Value;
typedef struct _Key_Value_Class Key_Value_Class;

struct _Key_Value
{
   GtkHBox box;

   GtkWidget* key_label;
   GtkWidget* value_entry;
};

struct _Key_Value_Class
{
   GtkHBoxClass parent_class;

   void (*key_value) ( Key_Value* kv );
};

GType       key_value_get_type    ( void );
GtkWidget*  key_value_new         ( const char* key_label_str );

G_END_DECLS

#endif

