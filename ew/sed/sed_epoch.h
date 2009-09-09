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

#include "sed_cube.h"
#include "sed_process.h"

new_handle( Sed_epoch       );
new_handle( Sed_epoch_queue );

typedef enum
{
   SED_EPOCH_ERROR_OPEN_FILE ,
   SED_EPOCH_ERROR_MISSING_LABEL ,
   SED_EPOCH_ERROR_BAD_TIME_STEP ,
   SED_EPOCH_ERROR_NEGATIVE_TIME_STEP ,
   SED_EPOCH_ERROR_NEGATIVE_DURATION
}
Sed_epoch_error;

#define SED_EPOCH_ERROR sed_epoch_error_quark()

Sed_epoch     sed_epoch_new            ( void        );
Sed_epoch     sed_epoch_copy           ( Sed_epoch d , const Sed_epoch s );
Sed_epoch     sed_epoch_dup            ( const Sed_epoch s );
Sed_epoch_queue sed_epoch_new_from_table ( Eh_symbol_table t , GError** error );
Sed_epoch     sed_epoch_destroy        ( Sed_epoch e );

Sed_epoch     sed_epoch_set_name       ( Sed_epoch e , gchar* name      );
Sed_epoch     sed_epoch_set_number     ( Sed_epoch e , gssize name      );
Sed_epoch     sed_epoch_set_duration   ( Sed_epoch e , double duration  );
Sed_epoch     sed_epoch_set_time_step  ( Sed_epoch e , double time_step );
Sed_epoch     sed_epoch_set_filename   ( Sed_epoch e , const gchar* filename  );

Sed_epoch     sed_epoch_sscan_filename ( Sed_epoch e , const gchar* file_s     , GError** error );
Sed_epoch     sed_epoch_sscan_number   ( Sed_epoch e , const gchar* number_s   , GError** error );
Sed_epoch     sed_epoch_sscan_time     ( Sed_epoch e , const gchar* time_s     , GError** error );
Sed_epoch     sed_epoch_sscan_duration ( Sed_epoch e , const gchar* duration_s , GError** error );
Sed_epoch     sed_epoch_sscan_time_step( Sed_epoch e , const gchar* dt_s       , GError** error );
Sed_epoch     sed_epoch_set_active_time( Sed_epoch e , double* time );

const char*   sed_epoch_name           ( Sed_epoch e );
gssize        sed_epoch_number         ( Sed_epoch e );
double        sed_epoch_start          ( Sed_epoch e );
double        sed_epoch_end            ( Sed_epoch e );
double        sed_epoch_duration       ( Sed_epoch e );
double        sed_epoch_time_step      ( Sed_epoch e );
const char*   sed_epoch_filename       ( Sed_epoch e );
Sed_process_queue sed_epoch_proc_queue ( Sed_epoch e );

Sed_epoch_queue sed_epoch_queue_new_sscan_old( const gchar* number_s    ,
                                               const gchar* time_step_s ,
                                               const gchar* file_s      ,
                                               const gchar* duration_s  ,
                                               GError** error );
Sed_epoch_queue sed_epoch_queue_new_sscan    ( const gchar* time_s      ,
                                               const gchar* time_step_s ,
                                               const gchar* file_s      ,
                                               GError** error );

Sed_epoch_queue sed_epoch_queue_new    ( const gchar* file , GError** error );
Sed_epoch_queue sed_epoch_queue_new_full( const gchar*       file          ,
                                          Sed_process_init_t proc_defs[]   ,
                                          Sed_process_family proc_family[] ,
                                          Sed_process_check  proc_checks[] ,
                                          GError**           error );

Sed_epoch_queue sed_epoch_queue_dup    ( const Sed_epoch_queue s );
Sed_epoch_queue sed_epoch_queue_concat ( Sed_epoch_queue q_1 , Sed_epoch_queue q_2 );
Sed_epoch_queue sed_epoch_queue_destroy( Sed_epoch_queue q );

gssize          sed_epoch_queue_length ( Sed_epoch_queue q );
Sed_epoch       sed_epoch_queue_first  (Sed_epoch_queue q);
Sed_epoch       sed_epoch_queue_last   (Sed_epoch_queue q);
Sed_epoch_queue sed_epoch_queue_order  ( Sed_epoch_queue q );
Sed_epoch_queue sed_epoch_queue_push_tail( Sed_epoch_queue q , Sed_epoch e );
Sed_epoch       sed_epoch_queue_pop    ( Sed_epoch_queue q );
Sed_epoch       sed_epoch_queue_nth    ( Sed_epoch_queue q , gssize n );

Sed_epoch       sed_epoch_queue_find   ( Sed_epoch_queue q , double t );

gssize          sed_epoch_fprint      ( FILE* fp , Sed_epoch e );
gssize          sed_epoch_queue_fprint( FILE* fp , Sed_epoch_queue q );

Sed_epoch       sed_epoch_scan_proc_queue( Sed_epoch          e          ,
                                           Sed_process_init_t p_list[]   ,
                                           Sed_process_family p_family[] ,
                                           Sed_process_check  p_check[]  ,
                                           GError** error );
Sed_epoch_queue sed_epoch_queue_set_processes( Sed_epoch_queue    q          ,
                                               Sed_process_init_t p_list[]   ,
                                               Sed_process_family p_family[] ,
                                               Sed_process_check  p_check[]  ,
                                               GError**           error );
gboolean        sed_epoch_queue_test_run( const Sed_epoch_queue q          ,
                                          Sed_process_init_t    p_list[]   ,
                                          Sed_process_family    p_family[] ,
                                          Sed_process_check     p_check[]  ,
                                          GError**              error );
Sed_epoch_queue sed_epoch_queue_run     ( Sed_epoch_queue       q        ,
                                          Sed_cube              p        );

Sed_epoch_queue sed_epoch_queue_tic      ( Sed_epoch_queue epoch_q , Sed_cube p );
Sed_epoch_queue sed_epoch_queue_run_until( Sed_epoch_queue epoch_q , Sed_cube p , double t_in_years );

#endif /* sed_epoch.h */
