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

#ifndef __EH_LOGGING_H__
#define __EH_LOGGING_H__

#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdarg.h>

#define EH_LOG_LEVEL_DATA (1<<G_LOG_LEVEL_USER_SHIFT)

#define DEFAULT_LOG "stdout"
#define DEFAULT_ERROR_LOG "stderr"

// open a log.
// log_name     : the name of the log to open.
// return value : nothing.
FILE* eh_open_log(const char *log_name);

// close a log.
// log_name     : the name of the log to close.
// return value : nothing.
void eh_close_log(const char *log_name);

// print a message to a log.
// log_name     : the name of the log to print to.
// message      : a format string to print.
// ...          : arguments to substitute into message.
// return value : nothing.
void eh_print_log(const char *log_name, const char *message,...);

// open a file pointer to a log.
// log_name     : the name of the log to open.
// return value : a file pointer for the log.
FILE *eh_open_log_file(const char *log_name);

// reset a log to it's original state.
// log_file     : the name of a log.
// return value : nothing.
void eh_reset_log(const char *log_file);

// rediret a log to another file.
// log_file     : the name of a log.
// return value : nothing.
void eh_redirect_log(const char *log_file1,const char *log_file2);

#ifndef EH_LOG_DOMAIN
# define EH_LOG_DOMAIN ((gchar*)0)
#endif

void eh_set_ignore_log_level( GLogLevelFlags ignore );
GLogLevelFlags eh_set_verbosity_level( gint verbosity );
void eh_logger( const gchar *log_domain ,
                GLogLevelFlags log_level ,
                const gchar *message ,
                gpointer user_data );
#ifdef G_HAVE_ISO_VARARGS
#define eh_message(...)  g_log( EH_LOG_DOMAIN ,       \
                                G_LOG_LEVEL_MESSAGE , \
                                __VA_ARGS__ )
#define eh_info(...)     g_log( EH_LOG_DOMAIN ,    \
                                G_LOG_LEVEL_INFO , \
                                __VA_ARGS__ )
#define eh_warning(...)  g_log( EH_LOG_DOMAIN ,       \
                                G_LOG_LEVEL_WARNING , \
                                __VA_ARGS__ )
#define eh_error(...)    g_log( EH_LOG_DOMAIN ,     \
                                G_LOG_LEVEL_ERROR , \
                                __VA_ARGS__ )
#define eh_debug(...)    g_log( EH_LOG_DOMAIN ,     \
                                G_LOG_LEVEL_DEBUG , \
                                __VA_ARGS__ )
#define eh_data(...)     g_log( EH_LOG_DOMAIN ,     \
                                EH_LOG_LEVEL_DATA , \
                                __VA_ARGS__ )
#elif defined(G_HAVE_GNUC_VARARGS)
#define eh_message(format...)  g_log( EH_LOG_DOMAIN ,       \
                                      G_LOG_LEVEL_MESSAGE , \
                                      format )
#define eh_info(format...)     g_log( EH_LOG_DOMAIN ,    \
                                      G_LOG_LEVEL_INFO , \
                                      format )
#define eh_warning(format...)  g_log( EH_LOG_DOMAIN ,       \
                                      G_LOG_LEVEL_WARNING , \
                                      format )
#define eh_error(format...)    g_log( EH_LOG_DOMAIN ,     \
                                      G_LOG_LEVEL_ERROR , \
                                      format )
#define eh_debug(format...)    g_log( EH_LOG_DOMAIN ,     \
                                      G_LOG_LEVEL_DEBUG , \
                                      format )
#define eh_data(format...)     g_log( EH_LOG_DOMAIN ,     \
                                      EH_LOG_LEVEL_DATA , \
                                      format )
#else
static void eh_message( const char *format , ... )
{
   va_list args;
   va_start( args , format );
   g_logv( EH_LOG_DOMAIN , G_LOG_LEVEL_MESSAGE , format , args );
   va_end( args );
}
static void eh_info( const char *format , ... )
{
   va_list args;
   va_start( args , format );
   g_logv( EH_LOG_DOMAIN , G_LOG_LEVEL_INFO , format , args );
   va_end( args );
}

static void eh_warning( const char *format , ... )
{
   va_list args;
   va_start( args , format );
   g_logv( EH_LOG_DOMAIN , G_LOG_LEVEL_WARNING , format , args );
   va_end( args );
}

static void eh_error( const char *format , ... )
{
   va_list args;
   va_start( args , format );
   g_logv( EH_LOG_DOMAIN , G_LOG_LEVEL_ERROR , format , args );
   va_end( args );
}

static void eh_debug( const char *format , ... )
{
   va_list args;
   va_start( args , format );
   g_logv( EH_LOG_DOMAIN , G_LOG_LEVEL_DEBUG , format , args );
   va_end( args );
}

static void eh_data( const char* format , ... )
{
   va_list args;
   va_start( args , format );
   g_logv( EH_LOG_DOMAIN , EH_LOG_LEVEL_DATA , format , args );
   va_end( args );
}
#endif

#endif /* eh_logging.h is included */
