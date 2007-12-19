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

#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#include <sed/sed_sedflux.h>
#include <plume_types.h>
#include <utils/utils.h>

/** Create a queue of processes from a file

Create a new queue of process for sedflux to run.  The processes are initialized
using the process file, \p file.

If the string array, \p user_data is non-NULL, it is a NULL-terminated list
of strings that indicate which processes should be active.  This will override
the process file of the epochs.  Any process not listed in \p active
will \b NOT be active.

\param file      The file containing the process information
\param user_data A string array of process to activate (or NULL)
\param error     A GError

\return A newly-allocated (and initialized) Sed_process_queue.
*/
/*
Sed_process_queue
sedflux_create_process_queue( const gchar* file , gchar** user_data , GError** error )
{
   Sed_process_queue q = NULL;

   eh_require( file );
   eh_return_val_if_fail( error==NULL || *error==NULL , NULL );

   if ( file )
   {
      GError*     tmp_err  = NULL;
      Eh_key_file key_file = NULL;

      key_file    = eh_key_file_scan( file , &tmp_err );

      if ( key_file )
      {
         gint   n_processes = sizeof( process_list ) / sizeof(Sed_process_init_t);
         gssize i;

         q = sed_process_queue_new();
         for ( i=0 ; i<n_processes ; i++ )
            sed_process_queue_push( q , process_list[i] );

         sed_process_queue_scan( q , key_file , &tmp_err );

         if ( !tmp_err )
         {
            if ( user_data )
            {
               gchar** name;
               sed_process_queue_deactivate( q , "<all>" );

               for ( name=user_data ; *name ; name++ )
                  sed_process_queue_activate( q , *name );
            }


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
         }
      }

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         q = NULL;
      }

      eh_key_file_destroy( key_file );
   }

   return q;
}
*/

typedef struct
{
   const gchar* name;
   gint error;
} Process_check_t;

/** Individual process errors
*/
Process_check_t process_check[] =
{
   {"plume"           , SED_ERROR_MULTIPLE_PROCS|SED_ERROR_INACTIVE|SED_ERROR_NOT_ALWAYS } ,
   {"bedload dumping" , SED_ERROR_MULTIPLE_PROCS|SED_ERROR_INACTIVE|SED_ERROR_NOT_ALWAYS } ,
   {"bbl"             , SED_ERROR_MULTIPLE_PROCS|SED_ERROR_INACTIVE|SED_ERROR_NOT_ALWAYS } ,
   {"river"           ,                          SED_ERROR_INACTIVE|SED_ERROR_NOT_ALWAYS } ,
   {"earthquake"      , SED_ERROR_MULTIPLE_PROCS } ,
   {"storms"          , SED_ERROR_MULTIPLE_PROCS }
};

typedef struct
{
   const gchar* parent_name;
   const gchar* child_name;
   gint error;
} Family_check_t;

/** Process-family errors
*/
Family_check_t family_check[] =
{
   { "failure"   , "earthquake" , SED_ERROR_INACTIVE_PARENT|SED_ERROR_ABSENT_PARENT|SED_ERROR_DT_MISMATCH },
   { "squall"    , "storms" , SED_ERROR_INACTIVE_PARENT|SED_ERROR_ABSENT_PARENT|SED_ERROR_DT_MISMATCH },
   { "xshore"    , "storms" , SED_ERROR_INACTIVE_PARENT|SED_ERROR_ABSENT_PARENT|SED_ERROR_DT_MISMATCH },
   { "diffusion" , "storms" , SED_ERROR_INACTIVE_PARENT|SED_ERROR_ABSENT_PARENT|SED_ERROR_DT_MISMATCH }
};


/** Check a Sed_process_queue for potential errors

The Sed_process_queue is examined for potential errors.  The type of
errors are listed in the variables \p process_check and \p family_check.
\p process_check describes errors that occur in individual processes
while \p family_check describes errors in families of processes.

\param q     A Sed_process_queue
\param error A GError

\return TRUE if there were no erros, FALSE otherwise.
*/
/*
gboolean
check_process_list( Sed_process_queue q , GError** error )
{
   gboolean file_is_ok = TRUE;
   gchar** err_s_list = NULL;
   gint error_no = 0;
   gssize n_checks = sizeof( process_check ) / sizeof( Process_check_t );
   gssize n_families;
   gssize i;
   gchar* err_s = NULL;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   for ( i=0 ; i<n_checks ; i++ )
   {
      error_no = sed_process_queue_check( q , process_check[i].name );
      if ( error_no!=0 & process_check[i].error )
      {
         err_s = g_strdup_printf( "%s: Error in process input file" , process_check[i].name , error_no );
         eh_strv_append( &err_s_list , err_s );
      }
   }

   n_families = sizeof( family_check ) / sizeof( Family_check_t );

   for ( i=0 ; i<n_families ; i++ )
   {
      error_no = sed_process_queue_check_family( q                           ,
                                                 family_check[i].parent_name ,
                                                 family_check[i].child_name , NULL );
      if (   error_no!=0 & family_check[i].error )
      {
         eh_warning( "%s: Possible error (#%d) in process input file." ,
                     family_check[i].parent_name , error );

         err_s = g_strdup_printf( "%s: Error %d in process input file" , process_check[i].name , error_no );
         eh_strv_append( &err_s_list , err_s );
      }
   }

   if ( err_s_list!=NULL )
   {
      GError* tmp_err = NULL;
      gchar*  err_msg = g_strjoinv( "\n" , err_s_list );

      g_set_error( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_PROCESS_FILE_CHECK , err_msg );

      g_propagate_error( error , tmp_err );

      eh_free   ( err_msg    );
      g_strfreev( err_s_list );
      
      file_is_ok = FALSE;
   }
   else
      file_is_ok = TRUE;

   return file_is_ok;
}
*/


/** Check the input process files.

This function will read in all of the epoch files, and initialize the
processes.  This is intended to find any errors in the input files at 
the begining of the model run.

If the string array, \p active is non-NULL, it is a NULL-terminated list
of strings that indicate which process should be active.  This will override
the process file of the epochs.  Any process not listed in \p active
will \b NOT be active.

\param e_list  A singly liked list of pointers to Epoch's.
\param active  If non-NULL, a list of process to activate.  
\param error   A GError

\return TRUE if there were no errors, FALSE otherwise
*/
/*
gboolean
check_process_files( Sed_epoch_queue e_list , gchar** active , GError** error )
{
   gboolean no_errors = TRUE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   {
      GError*           tmp_err  = NULL;
      int               n_epochs = sed_epoch_queue_length( e_list );
      gssize            i;
      Sed_process_queue q;
      Sed_epoch         this_epoch;

      //---
      // For each epoch, we create each of the processes, initialize the 
      // processes from the appropriate input file, and destroy the processes.
      //---
      for ( i=0 ; i<n_epochs && !tmp_err ; i++ )
      {
         this_epoch = sed_epoch_queue_nth( e_list , i );

         q = sedflux_create_process_queue( sed_epoch_filename(this_epoch) , active , &tmp_err );

         if ( q )
         {
            no_errors = no_errors && check_process_list( q , &tmp_err );
            sed_process_queue_destroy( q );
         }
      } 

      if ( tmp_err )
      {
         g_propagate_error( error , tmp_err );
         no_errors = FALSE;
      }
   }

   return no_errors;
}
*/

