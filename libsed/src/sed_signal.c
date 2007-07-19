#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <glib.h>
#include "utils.h"
#include "sed_signal.h"

void _print_choices( int sig_num );

int
sed_signal_set_action( void )
{
   gint rtn_val;
   struct sigaction new_action, old_action;

   /* Set the new action */
   new_action.sa_handler = _print_choices;
   sigemptyset( &new_action.sa_mask );
   new_action.sa_flags = 0;

   /* If the signal has already been set to be ignored, respect that */
   rtn_val = sigaction( SIGINT , NULL , &old_action );
   if ( old_action.sa_handler != SIG_IGN )
      rtn_val = sigaction( SIGINT , &new_action , NULL );

   return rtn_val;
}

static gboolean __quit_signal = FALSE; //< signal to indicate that the user wishes to quit
static gboolean __dump_signal = FALSE; //< signal to indicate that the user wishes to dump output
static gboolean __user_signal = FALSE; //< signal to indicate that the user wishes to do something
static gboolean __next_signal = FALSE; //< signal to indicate that the user wishes to goto next epoch
static gboolean __cpr_signal  = FALSE; //< signal to indicate that the user wishes to create a checkpoint

gboolean
sed_signal_is_pending( Sed_sig_num sig )
{
   gboolean is_pending = FALSE;

   if ( sig==SED_SIG_QUIT && __quit_signal ) is_pending = TRUE;
   if ( sig==SED_SIG_DUMP && __dump_signal ) is_pending = TRUE;
   if ( sig==SED_SIG_CPR  && __cpr_signal  ) is_pending = TRUE;
   if ( sig==SED_SIG_NEXT && __next_signal ) is_pending = TRUE;
   if ( sig==SED_SIG_USER && __user_signal ) is_pending = TRUE;
   
   return is_pending;
}

void
sed_signal_reset( Sed_sig_num sig )
{
   if ( sig&SED_SIG_QUIT && __quit_signal  ) __quit_signal = FALSE;
   if ( sig&SED_SIG_DUMP && __dump_signal  ) __dump_signal = FALSE;
   if ( sig&SED_SIG_CPR  && __cpr_signal   ) __cpr_signal  = FALSE;
   if ( sig&SED_SIG_NEXT  && __next_signal ) __next_signal = FALSE;
   if ( sig&SED_SIG_USER  && __user_signal ) __user_signal = FALSE;

   return;
}

void
sed_signal_set( Sed_sig_num sig )
{
   if ( sig&SED_SIG_QUIT ) __quit_signal = TRUE;
   if ( sig&SED_SIG_DUMP ) __dump_signal = TRUE;
   if ( sig&SED_SIG_CPR  ) __cpr_signal  = TRUE;
   if ( sig&SED_SIG_NEXT ) __next_signal = TRUE;
   if ( sig&SED_SIG_USER ) __user_signal = TRUE;

   return;
}

static gchar* head_msg[] =
{
   "-----------------------------------------------" ,
   NULL
};

static gchar* body_msg[] =
{
   "  (1) End run after this time step."             ,
   "  (2) Create a checkpoint."                      ,
   "  (3) Continue (do nothing)."                    ,
   "  (4) Jump to next epoch."                       ,
   "  (5) Quit immediatly (without saving)."         ,
   NULL
};

static gchar* tail_msg[] =
{
   "-----------------------------------------------" ,
   NULL
};

void
_print_choices( int sig_num )
{
   gboolean is_invalid = FALSE;
   char     ch;
   gchar**  msg;

   for ( msg=head_msg ; *msg ; msg++ ) fprintf( stdout , "%s\n" , *msg );
   for ( msg=body_msg ; *msg ; msg++ ) fprintf( stdout , "%s\n" , *msg );
   for ( msg=tail_msg ; *msg ; msg++ ) fprintf( stdout , "%s\n" , *msg );

   do
   {
      fprintf( stdout , "   Your choice [4]? " );

      is_invalid = FALSE;

      fscanf(stdin,"%s",&ch);

      if (      g_ascii_strcasecmp( &ch , "1" )==0 )
      {
         sed_signal_set( SED_SIG_QUIT );
         fprintf(stdout,"   You have opted to quit early...\n\n");
      }
      else if ( g_ascii_strcasecmp( &ch , "2" )==0 )
      {
         sed_signal_set( SED_SIG_DUMP );
         fprintf( stdout ,
                  "   Temporary output files will be dumped at the end of this time step...\n\n");
      }
      else if ( g_ascii_strcasecmp( &ch , "3" )==0 )
      {
         sed_signal_set( SED_SIG_CPR );
         fprintf(stdout,"   Creating a checkpoint/reset file...\n\n");
      }
      else if (    g_ascii_strcasecmp( &ch , "4" )==0 
                || g_ascii_strcasecmp( &ch , "" )==0 )
      {
         fprintf(stdout,"   Continuing...\n\n");
      }
      else if (    g_ascii_strcasecmp( &ch , "5" )==0 )
      {
         fprintf(stdout,"   Terminating run without saving...\n\n" );
         eh_exit( EXIT_SUCCESS );
      }
      else
         is_invalid = TRUE;
   }
   while ( is_invalid );

   return;
}

