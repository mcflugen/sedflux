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

#if !defined(EH_OPT_CONTEXT_H)
# define EH_OPT_CONTEXT_H

#include <stdio.h>
#include <glib.h>
#include "eh_types.h"

new_handle( Eh_opt_context );

typedef struct
{
   const gchar* long_name;
   gchar short_name;

   const gchar* description;
   const gchar* arg_description;

   const gchar* default_val;
}
Eh_opt_entry;

//---
// Public member function.
//---

Eh_opt_context eh_opt_create_context   ( const gchar* name        ,
                                         const gchar* description ,
                                         const gchar* help_description );
Eh_opt_context eh_destroy_context      ( Eh_opt_context context );
Eh_opt_context eh_opt_set_context      ( Eh_opt_context context ,
                                         Eh_opt_entry* entries );
gboolean       eh_opt_parse_context( Eh_opt_context context ,
                                         gint* argc             ,
                                         gchar*** argv          , 
                                         GError** error );
void           eh_opt_print_label_value( Eh_opt_context context , char *label );

char*          eh_opt_value            ( Eh_opt_context context , char* label );
char*          eh_opt_str_value        ( Eh_opt_context c , char* label );
gboolean       eh_opt_bool_value       ( Eh_opt_context c , char *label );
int            eh_opt_key_value        ( Eh_opt_context c , char *label ,
                                         char *keys[] );
gint           eh_opt_int_value        ( Eh_opt_context c , char *label );
double         eh_opt_dbl_value        ( Eh_opt_context c , char *label );

void           eh_opt_print_key_file   ( Eh_opt_context c , FILE *fp );
void           eh_opt_print_all_opts   ( Eh_opt_context c , FILE *fp );


#endif

