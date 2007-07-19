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

#define SED_COMPACTION_PROC_NAME "compaction"
#define EH_LOG_DOMAIN SED_COMPACTION_PROC_NAME

#include <stdio.h>
#include <pthread.h>
#include "sed_sedflux.h"
#include "my_processes.h"

#undef WITH_THREADS

#define N_THREADS 5

void thread_compact( void *data , void *user_data );
int compact(Sed_column,double);

Sed_process_info
run_compaction( Sed_process proc , Sed_cube p )
{
   Sed_process_info info = SED_EMPTY_INFO;

#if !defined(WITH_THREADS)

   {
      gint i;
      gint len = sed_cube_size(p);
      for ( i=0 ; i<len ; i++ )
         compact( sed_cube_col(p,i) , sed_cube_age_in_years(p) );
   }

#else

   if (!g_thread_supported ()) g_thread_init(NULL);

   {
      GError *error=NULL;
      GThreadPool *compact_pool;

      compact_pool = g_thread_pool_new( (GFunc)&thread_compact ,
                                        (gpointer)p            ,
                                        N_THREADS              ,
                                        TRUE                   ,
                                        &error );

      eh_require( error==NULL );
      {
         gint   i;
         gint*  queue;
         gssize len = sed_cube_size(p);

         queue = eh_new( int , len );
         for ( i=0 ; i<len ; i++ )
         {
            queue[i] = i;
            g_thread_pool_push( compact_pool , &(queue[i]) , &error );
            eh_require( error==NULL );
         }
         g_thread_pool_free( compact_pool , FALSE , TRUE );
         eh_free( queue );
      }
   }

#endif

   return info;
}

int pthread_mutex_spinlock(pthread_mutex_t *mutex);

void thread_compact( void *data , void *user_data )
{
   Sed_cube p = (Sed_cube)user_data;
   int i = *((int*)data);

   compact( sed_cube_col(p,i) , sed_cube_age_in_years(p) );

   return;
}

#define S_NTRIES 5

int pthread_mutex_spinlock(pthread_mutex_t *mutex)
{
   int i=0;
   while ( i++<S_NTRIES )
   {
      if ( pthread_mutex_trylock(mutex)==0 )
         return 0; // got the lock, return.
   }
   return pthread_mutex_lock(mutex); // give up.
}

