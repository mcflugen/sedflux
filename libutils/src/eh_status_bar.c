#include <stdio.h>

#include <glib.h>
#include "utils.h"
#include "eh_status_bar.h"

gpointer print_status( gpointer data );

Eh_status_bar*
eh_status_bar_new( double* cur , double* end )
{
   Eh_status_bar* status_bar = eh_new( Eh_status_bar , 1 );

   status_bar->cur    = cur;
   status_bar->end    = end;
   status_bar->status = EH_STATUS_BAR_RUNNING;
   status_bar->timer  = g_timer_new();
   status_bar->mutex  = g_mutex_new();

   status_bar->t      = g_thread_create( print_status , status_bar , TRUE , NULL );

   return status_bar;
}

Eh_status_bar*
eh_status_bar_stop( Eh_status_bar* b )
{
   g_mutex_lock( b->mutex );
   b->status = EH_STATUS_BAR_STOPPED;
   g_mutex_unlock( b->mutex );

   return b;
}

Eh_status_bar*
eh_status_bar_pause( Eh_status_bar* b )
{
   g_mutex_lock( b->mutex );
   b->status = EH_STATUS_BAR_PAUSED;
   g_mutex_unlock( b->mutex );
   return b;
}

gboolean
eh_status_bar_is_stopped( Eh_status_bar* b )
{
   gboolean is_stopped;

   g_mutex_lock( b->mutex );
   is_stopped = (b->status == EH_STATUS_BAR_STOPPED);
   g_mutex_unlock( b->mutex );

   return is_stopped;
}

Eh_status_bar*
eh_status_bar_destroy( Eh_status_bar* b )
{
   eh_status_bar_stop( b );

   g_thread_join( b->t );

   g_timer_destroy( b->timer );
   g_mutex_free   ( b->mutex );
   eh_free( b );

   return NULL;
}

gpointer
print_status( gpointer data )
{
   Eh_status_bar* b = (Eh_status_bar*)data;
   double  t;
   double  eta;
   gchar*  t_str;
   gchar*  eta_str;
   gchar*  status_bar[] = { "." , "o" , "0" , "O" , NULL };
   gchar** p = status_bar;

   fprintf( stderr , "\n" );
   fprintf( stderr , " Current        |   Elapsed   |     ETA     \n" );

   for ( ; *(b->cur)<=0 && !eh_status_bar_is_stopped(b) ; );

   for ( ; !eh_status_bar_is_stopped(b) ; )
   {
      t   = g_timer_elapsed(b->timer,NULL);
      eta = t / *(b->cur) * ( *(b->end) - *(b->cur) );

      if ( *p==NULL )
         p = status_bar;

      if ( b->status==EH_STATUS_BAR_RUNNING )
      {
         t_str   = eh_render_time_str( t   );
         eta_str = eh_render_time_str( eta );

         fprintf( stderr , " %7g (%3.0f%%) | %s | %s" ,
                  *(b->cur) , *(b->cur) / *(b->end)*100. , t_str , eta_str );

         fprintf( stderr , "   (%s)" , *p );
         fprintf( stderr , "          \r" );

         eh_free( eta_str );
         eh_free( t_str   );
      }

      p++;

      g_usleep( 100000 );
   }

   t_str = eh_render_time_str( g_timer_elapsed(b->timer,NULL) );

   fprintf( stderr , "\n" );

   fprintf( stderr , "Elapsed time: %s\n" , t_str );

   eh_free( t_str );

   return data;
}

#define EH_SECONDS_PER_DAY    ( 86400. )
#define EH_SECONDS_PER_HOUR   ( 3600. )
#define EH_SECONDS_PER_MINUTE ( 60. )

gchar*
eh_render_time_str( double sec )
{
   gchar* str = NULL;

   {
      gint d = sec / (gint)EH_SECONDS_PER_DAY;
      gint h = sec / (gint)EH_SECONDS_PER_HOUR;
      gint m = sec / (gint)EH_SECONDS_PER_MINUTE;
      gint s = fmod( sec , 60. );

      str = g_strdup_printf( "%02d:%02d:%02d:%02d" , d , h , m , s );
   }

   return str;
}

