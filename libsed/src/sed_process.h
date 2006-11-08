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

#ifndef PROCESS_H
# define PROCESS_H

#include <glib.h>
#include "utils.h"

#include "sed_cube.h"

#define PROCESS_ALWAYS    ( 1 << 3 )
#define PROCESS_REGULAR   ( 1 << 2 )
#define PROCESS_IRREGULAR ( 1 << 1 )
#define PROCESS_SPECIFIED ( 1 << 0 )

#define SED_MAX_LOG_FILES (2)

typedef struct
{
// Public
   double mass_added;
   double mass_lost;
   gboolean error;
}
Sed_process_info;

const static Sed_process_info __empty_info = { 0,0,FALSE };

#define SED_EMPTY_INFO __empty_info

typedef gboolean         (*init_func)  (Eh_symbol_table,gpointer);
typedef Sed_process_info (*run_func)   (gpointer,Sed_cube);
typedef gboolean         (*load_func)  (gpointer,FILE*);
typedef gboolean         (*dump_func)  (gpointer,FILE*);

new_handle( Sed_process );

Sed_process    sed_process_create         ( const char *name ,
                                            size_t data_size ,
                                            init_func f_init ,
                                            run_func f_run   ); 
Sed_process    sed_process_copy           ( Sed_process d    , Sed_process s );
Sed_process    sed_process_dup            ( Sed_process s );
Sed_process    sed_process_destroy        ( Sed_process p );
void           sed_process_clean          ( Sed_process p );
double         sed_process_next_event     ( Sed_process p );
Sed_process    sed_process_set_next_event ( Sed_process p    , double new_next_event );
gboolean       sed_process_is_on          ( Sed_process p    , double time );
gboolean       sed_process_array_run      ( GPtrArray *a     , Sed_cube );
gboolean       sed_process_run            ( Sed_process      , Sed_cube );
gboolean       sed_process_run_now        ( Sed_process      , Sed_cube );
void           sed_process_init           ( Sed_process a    , Eh_symbol_table symbol_table );
GSList*        sed_process_scan           ( Eh_key_file k    , Sed_process p );
gssize         sed_process_fprint         ( FILE *fp         , Sed_process p );

gpointer       sed_process_data           ( Sed_process p );
double         sed_process_interval       ( Sed_process p );
gchar*         sed_process_name           ( Sed_process p );
gboolean       sed_process_is_active      ( Sed_process p );

gssize         sed_process_fprint_info( FILE* fp , Sed_process p );
gboolean       sed_process_error( Sed_process p );

#define sed_process_new(name,type,f_init,f_run) ( \
   sed_process_create( name , sizeof(type) , f_init , f_run ) )
#define sed_process_data_val(p,member,type) ( ((type*)(sed_process_data(p)))->member )

//#define sed_new_process(name,type,f_init,f_run,f_load,f_dump) ( sed_process_create(name,sizeof(type),f_init,f_run,f_load,f_dump) )

//#define sed_process_data_val(p,member,type) ( ((type*)((p)->data))->member )

#endif /* process.h is included */
