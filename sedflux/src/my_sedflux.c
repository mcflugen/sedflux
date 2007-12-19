#include <glib.h>
#include <sed/sed_sedflux.h>
#include "my_sedflux.h"

void
my_hook( Sed_process_queue q )
{
   if ( q )
   {
      gssize i;
      Failure_proc_t** data;
      Sed_process d = sed_process_queue_find_nth_obj( q , "debris flow"       , 0 );
      Sed_process t = sed_process_queue_find_nth_obj( q , "turbidity current" , 0 );
      Sed_process s = sed_process_queue_find_nth_obj( q , "slump"             , 0 );

      data = (Failure_proc_t**)sed_process_queue_obj_data( q , "failure" );
      for ( i=0 ; data && data[i] ; i++ )
      {
         data[i]->debris_flow       = d;
         data[i]->turbidity_current = t;
         data[i]->slump             = s;
      }
      eh_free( data );
   }

   return;
}

