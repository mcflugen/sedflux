#ifndef __ADD_REM_H__
#define __ADD_REM_H__

#include <gtk/gtk.h>
#include <gtk/gtkhbox.h>

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define ADD_REM_TYPE            ( add_rem_get_type() )
#define ADD_REM( obj )          ( G_TYPE_CHECK_INSTANCE_CAST((obj),ADD_REM_TYPE,Add_Rem) )
#define ADD_REM_CLASS( klass ) ( G_TYPE_CHECK_CLASS_CAST( (klass) , ADD_REM_TYPE , Add_Rem_Class ) )
#define IS_ADD_REM( obj )       ( G_TYPE_CHECK_INSTANCE_CAST( (obj) , ADD_REMTYPE ) )
#define IS_ADD_REM_CLASS( klass ) (G_TYPE_CHECK_CLASS_TYPE( (klass) , ADD_REM_TYPE ) )

typedef struct _Add_Rem   Add_Rem;
typedef struct _Add_Rem_Class Add_Rem_Class;

struct _Add_Rem
{
   GtkHBox box;

   GtkWidget* label;
   GtkWidget* add_button;
   GtkWidget* rem_button;
};

struct _Add_Rem_Class
{
   GtkHBoxClass parent_class;

   void (*add_line) ( Add_Rem* obj );
   void (*rem_line) ( Add_Rem* obj );
};

GType       add_rem_get_type ( void );
GtkWidget*   add_rem_new     ( const char* label_str );

G_END_DECLS

#endif

