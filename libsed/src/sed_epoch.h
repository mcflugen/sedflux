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

#if !defined(SED_EPOCH_H)
# define SED_EPOCH_H

new_handle( Sed_epoch       );
new_handle( Sed_epoch_queue );

Sed_epoch     sed_epoch_new            ( void        );
Sed_epoch     sed_epoch_new_from_table ( Eh_symbol_table t );
Sed_epoch     sed_epoch_destroy        ( Sed_epoch e );

Sed_epoch     sed_epoch_set_name       ( Sed_epoch e , gchar* name      );
Sed_epoch     sed_epoch_set_number     ( Sed_epoch e , gssize name      );
Sed_epoch     sed_epoch_set_duration   ( Sed_epoch e , double duration  );
Sed_epoch     sed_epoch_set_time_step  ( Sed_epoch e , double time_step );
Sed_epoch     sed_epoch_set_filename   ( Sed_epoch e , gchar* filename  );

const char*   sed_epoch_name           ( Sed_epoch e );
gssize        sed_epoch_number         ( Sed_epoch e );
double        sed_epoch_duration       ( Sed_epoch e );
double        sed_epoch_time_step      ( Sed_epoch e );
const char*   sed_epoch_filename       ( Sed_epoch e );

Sed_epoch_queue sed_epoch_queue_new    ( const gchar* file , GError** error );
Sed_epoch_queue sed_epoch_queue_destroy( Sed_epoch_queue q );
gssize          sed_epoch_queue_length ( Sed_epoch_queue q );
gint            sed_epoch_number_cmp   ( Sed_epoch a , Sed_epoch b );
Sed_epoch       sed_epoch_queue_pop    ( Sed_epoch_queue q );
Sed_epoch       sed_epoch_queue_nth    ( Sed_epoch_queue q , gssize n );


gssize          sed_epoch_fprint      ( FILE* fp , Sed_epoch e );
gssize          sed_epoch_queue_fprint( FILE* fp , Sed_epoch_queue q );


#endif /* sed_epoch.h */
