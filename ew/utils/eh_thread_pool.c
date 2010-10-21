/* Thread pools */
#include <eh_utils.h>
#include <pthread.h>

void
tpool_init(tpool_t *tpoolp, int num_worker_threads, int max_queue_size, int do_not_block_when_full)
{
   int i, rtn;
   tpool_t tpool;
   pthread_attr_t attr;

   pthread_attr_init( &attr );
   pthread_attr_setscope( &attr , PTHREAD_SCOPE_SYSTEM );

//   if ( (tpool = (tpool_t)malloc(sizeof(struct tpool))) == NULL )
//      perror("malloc"), eh_exit(-1);
//   tpool = (tpool_t)eh_malloc( sizeof(tpool_t) , NULL , __FILE__ , __LINE__ );
   tpool = eh_new( struct tpool , 1 );

   tpool->num_threads = num_worker_threads;
   tpool->max_queue_size = max_queue_size;
   tpool->do_not_block_when_full = do_not_block_when_full;
//   if ( (tpool->threads = (pthread_t*)malloc(sizeof(pthread_t)*num_worker_threads)) == NULL )
//      perror("malloc"), eh_exit(-1);
   tpool->threads = eh_new( pthread_t , num_worker_threads );
   
   tpool->cur_queue_size = 0;
   tpool->queue_head = NULL;
   tpool->queue_tail = NULL;
   tpool->queue_closed = 0;
   tpool->shutdown = 0;
   if ( (rtn=pthread_mutex_init(&(tpool->queue_lock),NULL)) != 0 )
      fprintf(stderr,"pthread_mutex_init %s",strerror(rtn)), eh_exit(-1);
   if ( (rtn=pthread_cond_init(&(tpool->queue_not_empty),NULL)) != 0 )
      fprintf(stderr,"pthread_cond_init %s",strerror(rtn)), eh_exit(-1);
   if ( (rtn=pthread_cond_init(&(tpool->queue_not_full),NULL)) != 0 )
      fprintf(stderr,"pthread_cond_init %s",strerror(rtn)), eh_exit(-1);
   if ( (rtn=pthread_cond_init(&(tpool->queue_empty),NULL)) != 0 )
      fprintf(stderr,"pthread_cond_init %s",strerror(rtn)), eh_exit(-1);

   for ( i=0 ; i!=num_worker_threads ; i++ )
      if ( (rtn=pthread_create( &(tpool->threads[i]) , &attr , (void *(*)(void*))tpool_thread , (void*)tpool )) != 0 )
         fprintf(stderr,"pthread_create %d",rtn), eh_exit(-1);

   pthread_attr_destroy(&attr);

   *tpoolp = tpool;
}

void
tpool_thread( tpool_t tpool )
{
   tpool_work_t *my_workp;

   for (;;)
   {
      pthread_mutex_lock( &(tpool->queue_lock) );
      while ( (tpool->cur_queue_size==0) && (!tpool->shutdown) )
      {
         pthread_cond_wait( &(tpool->queue_not_empty) , &(tpool->queue_lock) );
      }

      if ( tpool->shutdown )
      {
         pthread_mutex_unlock( &(tpool->queue_lock) );
         pthread_exit( NULL );
      }

      my_workp = tpool->queue_head;
      tpool->cur_queue_size--;
      if ( tpool->cur_queue_size == 0 )
         tpool->queue_head = tpool->queue_tail = NULL;
      else
         tpool->queue_head = my_workp->next;

      if ( (!tpool->do_not_block_when_full) && (tpool->cur_queue_size==(tpool->max_queue_size-1)) )
         pthread_cond_broadcast( &(tpool->queue_not_full) );

      if ( tpool->cur_queue_size == 0 )
         pthread_cond_signal( &(tpool->queue_empty) );

      pthread_mutex_unlock( &(tpool->queue_lock) );
      //(*(my_workp->routine))(my_workp->arg);
      (*(my_workp->routine))();
      eh_free(my_workp);
   }
}

int
tpool_add_work( tpool_t tpool , void *routine , void *arg )
{
   tpool_work_t *workp;
   pthread_mutex_lock( &(tpool->queue_lock) );

   if (    ( tpool->cur_queue_size==tpool->max_queue_size )
        && tpool->do_not_block_when_full )
   {
      pthread_mutex_unlock( &(tpool->queue_lock) );
      return -1;
   }

   while (    ( tpool->cur_queue_size==tpool->max_queue_size )
           && ( !(tpool->shutdown || tpool->queue_closed) ) )
   {
      pthread_cond_wait( &(tpool->queue_not_full) , &(tpool->queue_lock) );
   }

   if ( tpool->shutdown || tpool->queue_closed )
   {
      pthread_mutex_unlock( &(tpool->queue_lock) );
      return -1;
   }

//   workp = (tpool_work_t*)malloc(sizeof(tpool_work_t));
   workp = eh_new( tpool_work_t , 1 );
   workp->routine = (void(*)())routine;
   workp->arg = arg;
   workp->next = NULL;
   if ( tpool->cur_queue_size == 0 )
   {
      tpool->queue_tail = tpool->queue_head = workp;
      pthread_cond_broadcast( &(tpool->queue_not_empty) );
   }
   else
   {
      (tpool->queue_tail)->next = workp;
      tpool->queue_tail = workp;
   }
   tpool->cur_queue_size++;

   pthread_mutex_unlock( &(tpool->queue_lock) );
   return 1;
}

int
tpool_destroy( tpool_t tpool , int finish )
{
   int i, rtn;
   tpool_work_t *cur_nodep;

   if ( (rtn=pthread_mutex_lock( &(tpool->queue_lock) )) != 0 )
      fprintf(stderr,"pthread_mutex_lock %d",rtn), eh_exit(-1);

   if ( tpool->queue_closed || tpool->shutdown )
   {
      if ( (rtn=pthread_mutex_unlock( &(tpool->queue_lock) )) != 0 )
         fprintf(stderr,"pthread_mutex_unlock %d",rtn), eh_exit(-1);
      return 0;
   }

   tpool->queue_closed = 1;

   if ( finish==1 )
   {
      while ( tpool->cur_queue_size!=0 )
         if ( (rtn=pthread_cond_wait( &(tpool->queue_empty) , &(tpool->queue_lock) )) != 0 )
            fprintf(stderr,"pthread_cond_wait %d",rtn), eh_exit(-1);
   }

   tpool->shutdown = 1;

   if ( (rtn=pthread_mutex_unlock( &(tpool->queue_lock) ))!=0 )
      fprintf(stderr,"pthread_mutex_unlock %d",rtn), eh_exit(-1);

   if ( (rtn=pthread_cond_broadcast( &(tpool->queue_not_empty) ))!=0 )
      fprintf(stderr,"pthread_cond_broadcast %d",rtn), eh_exit(-1);
   if ( (rtn=pthread_cond_broadcast( &(tpool->queue_not_full) ))!=0 )
     fprintf(stderr,"pthread_cond_broadcast %d",rtn), eh_exit(-1);

   for ( i=0 ; i<tpool->num_threads ; i++ )
   {
      if ( (rtn=pthread_join( tpool->threads[i] , NULL ))!=0 )
         fprintf(stderr,"pthread_join %d",rtn), eh_exit(-1);
   }

   eh_free(tpool->threads);
   while ( tpool->queue_head != NULL )
   {
      cur_nodep = tpool->queue_head->next;
      tpool->queue_head = tpool->queue_head->next;
      eh_free(cur_nodep);
   }
   eh_free(tpool);
   return 0 ;
}

