#include "eh_input_val.h"

#include <glib.h>
#include "eh_data_record.h"
#include "utils.h"

/** A value that changes in a user-specified manner
*/
CLASS( Eh_input_val )
{
/// The type of Eh_input_val.  This defines the way the value changes.
   Eh_input_val_type type;
/// Pointer to a user-specified file, if necessary.  NULL, otherwise.
   FILE *fp;
/// The name of a user-specified file, if necessary.  NULL, otherwise.
   char *file_name;
/// Array of x-values for a time series or a user-defined CDF
   double *x;
/// Array of y-values for a time series or a user-defined CDF
   double *y;
/// Length of \a x and \a y
   double len;
/// A random number generator, if necessary.  NULL, otherwise.
   GRand* rand;
/// Data used to calculate a new value
   double data[2];
/// The current value of the Eh_input_val
   double val;
};

/** Create an Eh_input_val

\return A new Eh_input_val.  Should be destroyed with eh_input_val_destroy.

\see eh_destroy_input_val.
*/
Eh_input_val eh_input_val_new( )
{
   Eh_input_val val;

   NEW_OBJECT( Eh_input_val , val );

   if ( val )
   {
      val->fp        = NULL;
      val->file_name = NULL;
      val->x         = NULL;
      val->y         = NULL;
      val->len       = 0;
      val->val       = G_MINDOUBLE;
      val->rand      = g_rand_new();
   }

   return val;
}

/** Destroy an Eh_input_val

\param val An Eh_input_val that was created with eh_create_input_val.

\see eh_create_input_val.
*/
Eh_input_val eh_input_val_destroy( Eh_input_val val )
{
   if ( val )
   {
      eh_free( val->x         );
      eh_free( val->y         );
      eh_free( val->file_name );
      g_rand_free( val->rand );
      eh_free( val );
   }
   return NULL;
}

/** Create an Eh_input_val with a value

The Eh_input_val can be initialized to be one of:
   - A scalar.  In this case, \a input_str should be a string representation
     of the scalar value.
   - A time series from a file.  In this case, \a input_str should be of the
     form FILE=FILENAME, where FILENAME is the name of a file that contains
     a time series of values for the Eh_input_val.
   - A random variable.  In this case, the values for the Eh_input_val will
     be drawn from a distribution.
        - UNIFORM=MIN,MAX draws the values from a uniform distribution
          between MIN and MAX.
        - NORMAL=MU,SIGMA draws the values from a normal distribution
          with mean MU and standard deviation SIGMA.
        - WEIBUL=ETA,BETA draws the values from a weibul distribution
          with scale parameter ETA and shape parameter BETA.
        - USER=FILENAME draws values from a user specified distribution
          function defined in file, FILENAME.

\param input_str An initialization string for the Eh_input_val

\return A new (initialized) Eh_input_val
*/
Eh_input_val eh_input_val_set( const char *input_str )
{
   Eh_input_val val = eh_input_val_new();

   eh_require( input_str );

   //---
   // Search input_str for '='.  If the string contains '=' then it is of
   // the form "FILE=filename", "UNIFORM=x,y", or "NORMAL=x,y".  Otherwise,
   // the string should be a string representation of a number.
   //---
   if ( strchr( input_str , '=' ) == NULL )
   {
      val->type = EH_INPUT_VAL_SCALAR;
      val->val = g_ascii_strtod( input_str , NULL );
   }
   else
   {
      char **equal_split = g_strsplit( input_str , "=" , 2 );

      if (    g_ascii_strcasecmp( equal_split[0] , "FILE" ) == 0 
           || g_ascii_strcasecmp( equal_split[0] , "USER" ) == 0  )
      {
         Eh_data_record* data_rec;

         if ( g_ascii_strcasecmp( equal_split[0] , "FILE" ) == 0 )
            val->type = EH_INPUT_VAL_FILE;
         else
            val->type = EH_INPUT_VAL_RAND_USER;

         val->file_name = g_strdup( equal_split[1] );

         data_rec = eh_data_record_scan_file( val->file_name , "," , EH_FAST_DIM_COL , FALSE );

         if ( data_rec )
         {
            gssize i;

            if ( eh_data_record_size( data_rec[0] , 0 )>=2 )
            {
               val->len = eh_data_record_size   ( data_rec[0] , 1 );
               val->x   = eh_data_record_dup_row( data_rec[0] , 0 );
               val->y   = eh_data_record_dup_row( data_rec[0] , 1 );
            }
            else
               eh_error( "Only one column of data found (expecting two)" );

            for ( i=0 ; data_rec[i] ; i++ )
               eh_data_record_destroy( data_rec[i] );
            eh_free( data_rec );
         }
      }
      else if (    g_ascii_strcasecmp( equal_split[0] , "UNIFORM" ) == 0 
                || g_ascii_strcasecmp( equal_split[0] , "NORMAL"  ) == 0 
                || g_ascii_strcasecmp( equal_split[0] , "WEIBULL" ) == 0 )
      {
         char **comma_split = g_strsplit( equal_split[1] , "," , 2 );

         val->data[0] = g_ascii_strtod( comma_split[0] , NULL );
         val->data[1] = g_ascii_strtod( comma_split[1] , NULL );

         g_strfreev( comma_split );

         if      ( g_ascii_strcasecmp( equal_split[0] , "UNIFORM" ) == 0 )
            val->type = EH_INPUT_VAL_RAND_UNIFORM;
         else if ( g_ascii_strcasecmp( equal_split[0] , "NORMAL"  ) == 0 )
            val->type = EH_INPUT_VAL_RAND_NORMAL;
         else if ( g_ascii_strcasecmp( equal_split[0] , "WEIBULL" ) == 0 )
            val->type = EH_INPUT_VAL_RAND_WEIBULL;
         else
            eh_require_not_reached();
      }
      else
      {
         eh_error( "Bad key value.  Must be one of "
                   "UNIFORM, "
                   "NORMAL, "
                   "WEIBULL, "
                   "USER, "
                   "FILE" );
         eh_require_not_reached();
      }

      g_strfreev( equal_split );
   }
   
   return val;
}

/** Evaluate an Eh_input_val

\param val An initialized Eh_input_val
\param ... Extra parameter for a time value if \a val is set to read from
           a time series.

\return The new value
*/
double eh_input_val_eval( Eh_input_val val , ... )
{
   va_list args;
   double data;

   if ( val->type == EH_INPUT_VAL_RAND_NORMAL )
      val->val = eh_rand_normal( val->rand , val->data[0] , val->data[1] );
   else if ( val->type == EH_INPUT_VAL_RAND_UNIFORM )
      val->val = g_rand_double_range( val->rand , val->data[0] , val->data[1] );
   else if ( val->type == EH_INPUT_VAL_RAND_WEIBULL )
      val->val = eh_rand_weibull( val->rand , val->data[0] , val->data[1] );
   else if ( val->type == EH_INPUT_VAL_RAND_USER )
      val->val = eh_rand_user( val->rand , val->x , val->y , val->len );
   else if ( val->type == EH_INPUT_VAL_FILE )
   {

      eh_require( val->x );
      eh_require( val->y );

      va_start( args , val );
      data = va_arg( args , double );

      interpolate( val->x , val->y , val->len , &data , &(val->val) , 1 );

      va_end( args );
   }

   return val->val;
}


