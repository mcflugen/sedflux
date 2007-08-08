#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <glib.h>
#include "utils.h"
#include "sed_signal.h"

G_GNUC_INTERNAL void _print_choices( int sig_num );

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

static gboolean __quit_signal = FALSE; //< signal to indicate that the user wishes to quit gracefully
static gboolean __exit_signal = FALSE; //< signal to indicate that the user wishes to quit ungracefully
static gboolean __dump_signal = FALSE; //< signal to indicate that the user wishes to dump output
static gboolean __user_signal = FALSE; //< signal to indicate that the user wishes to do something
static gboolean __next_signal = FALSE; //< signal to indicate that the user wishes to goto next epoch
static gboolean __cpr_signal  = FALSE; //< signal to indicate that the user wishes to create a checkpoint

gboolean
sed_signal_is_pending( Sed_sig_num sig )
{
   gboolean is_pending = FALSE;

   if ( sig==SED_SIG_QUIT && __quit_signal ) is_pending = TRUE;
   if ( sig==SED_SIG_EXIT && __exit_signal ) is_pending = TRUE;
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
   if ( sig&SED_SIG_EXIT && __exit_signal  ) __exit_signal = FALSE;
   if ( sig&SED_SIG_DUMP && __dump_signal  ) __dump_signal = FALSE;
   if ( sig&SED_SIG_CPR  && __cpr_signal   ) __cpr_signal  = FALSE;
   if ( sig&SED_SIG_NEXT && __next_signal  ) __next_signal = FALSE;
   if ( sig&SED_SIG_USER && __user_signal  ) __user_signal = FALSE;

   return;
}

void
sed_signal_set( Sed_sig_num sig )
{
   if ( sig&SED_SIG_QUIT ) __quit_signal = TRUE;
   if ( sig&SED_SIG_EXIT ) __exit_signal = TRUE;
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
   "  (2) Dump results and continue."                ,
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

static const gchar* pending_msg[] =
{
   [SED_SIG_QUIT] = "Quitting sedflux..." ,
   [SED_SIG_DUMP] = "Dump output files at end of current time step..." ,
   [SED_SIG_CONT] = "Continuing..." ,
   [SED_SIG_NEXT] = "Jumping to next epoch..." ,
   [SED_SIG_EXIT] = "Exiting sedflux..." ,
};

G_GNUC_INTERNAL
void
clear_to_eol( FILE* fp)
{
   char c = fgetc( fp );
   while ( c!='\n' ) c = fgetc( fp );
}

void
_print_choices( int sig_num )
{
   gchar**  msg;
   Sed_sig_num signal;
   gchar ch;

   for ( msg=head_msg ; *msg ; msg++ ) fprintf( stdout , "%s\n" , *msg );
   for ( msg=body_msg ; *msg ; msg++ ) fprintf( stdout , "%s\n" , *msg );
   for ( msg=tail_msg ; *msg ; msg++ ) fprintf( stdout , "%s\n" , *msg );

   fprintf( stdout , "   Your choice [3]? " );

   ch = fgetc( stdin );

   switch ( ch )
   {
      case '1': signal = SED_SIG_QUIT; break;
      case '2': signal = SED_SIG_DUMP; break;
      case '3': signal = SED_SIG_CONT; break;
      case '4': signal = SED_SIG_NEXT; break;
      case '5': signal = SED_SIG_EXIT; break;
      default : signal = SED_SIG_CONT;
   }

   if ( ch!='\n' ) clear_to_eol( stdin );

   sed_signal_set( signal );

   fprintf( stdout , "   %s\n" , pending_msg[signal] );

   if ( sed_signal_is_pending( SED_SIG_EXIT ) ) eh_exit( EXIT_SUCCESS );

   return;
}

