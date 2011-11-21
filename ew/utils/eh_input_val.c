#include <eh_utils.h>

/** A value that changes in a user-specified manner
*/
CLASS( Eh_input_val )
{
   Eh_input_val_type type;    //< The type of Eh_input_val.  This defines the way the value changes.
   FILE*             fp;      //< Pointer to a user-specified file, if necessary.  NULL, otherwise.
   gchar*            file;    //< The name of a user-specified file, if necessary.  NULL, otherwise.
   double*           x;       //< Array of x-values for a time series or a user-defined CDF
   double*           y;       //< Array of y-values for a time series or a user-defined CDF
   gint              len;     //< Length of \a x and \a y
   GRand*            rand;    //< A random number generator, if necessary.  NULL, otherwise.
   double            data[2]; //< Data used to calculate a new value
   double            val;     //< The current value of the Eh_input_val
};

GQuark
eh_input_val_error_quark( void )
{
   return g_quark_from_static_string( "eh-input-val-error-quark" );
}

/** Create an Eh_input_val

\return A new Eh_input_val.  Should be destroyed with eh_input_val_destroy.

\see eh_destroy_input_val.
*/
Eh_input_val
eh_input_val_new( )
{
   Eh_input_val val;

   NEW_OBJECT( Eh_input_val , val );

   if ( val )
   {
      val->fp        = NULL;
      val->file      = NULL;
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
Eh_input_val
eh_input_val_destroy( Eh_input_val val )
{
   if (val)
   {
      eh_free (val->x);
      eh_free (val->y);
      eh_free (val->file);
      g_rand_free (val->rand);
      eh_free (val);
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
\param err       Location of a GError to indicate and error (or NULL)

\return A new (initialized) Eh_input_val, or NULL if an error occured
*/
Eh_input_val
eh_input_val_set( const char *input_str , GError** err )
{
   Eh_input_val val = NULL;

   eh_return_val_if_fail( err==NULL || *err==NULL , NULL );

   eh_require( input_str );

   //---
   // Search input_str for '='.  If the string contains '=' then it is of
   // the form "FILE=filename", "UNIFORM=x,y", or "NORMAL=x,y".  Otherwise,
   // the string should be a string representation of a number.
   //---
   if ( strchr( input_str , '=' ) == NULL )
   {
      val = eh_input_val_new();

      val->type = EH_INPUT_VAL_SCALAR;
      val->val = g_ascii_strtod( input_str , NULL );
   }
   else
   {
      GError* tmp_error = NULL;
      char **equal_split = g_strsplit( input_str , "=" , 2 );

      if (    g_ascii_strcasecmp( equal_split[0] , "FILE" ) == 0 
           || g_ascii_strcasecmp( equal_split[0] , "USER" ) == 0  )
      {
         double** data;
         gint n_rows, n_cols;
         Eh_input_val_type type;
         gchar* file;

         if ( g_ascii_strcasecmp( equal_split[0] , "FILE" ) == 0 )
            type = EH_INPUT_VAL_FILE;
         else
            type = EH_INPUT_VAL_RAND_USER;

         file = g_strdup( equal_split[1] );

         data = eh_dlm_read_swap( file , ";," , &n_rows , &n_cols , &tmp_error );

         if ( !tmp_error )
         {
            if ( n_rows!=2 )
               g_set_error( &tmp_error ,
                            EH_INPUT_VAL_ERROR ,
                            EH_INPUT_VAL_ERROR_NOT_TWO_COLUMNS ,
                            "%s: Input file does not contain 2 columns (found %d)\n" ,
                            file , n_rows );
            else if ( !eh_dbl_array_is_monotonic_up( data[0] , n_cols ) )
               g_set_error( &tmp_error ,
                            EH_INPUT_VAL_ERROR ,
                            EH_INPUT_VAL_ERROR_X_NOT_MONOTONIC ,
                            "%s: The first column must be monotonically increasing\n" ,
                            file );
            else if (    type == EH_INPUT_VAL_RAND_USER
                      && !eh_dbl_array_is_monotonic_up( data[1] , n_cols ) )
               g_set_error( &tmp_error ,
                            EH_INPUT_VAL_ERROR ,
                            EH_INPUT_VAL_ERROR_F_NOT_MONOTONIC ,
                            "%s: The second column must be monotonically increasing\n" ,
                            file );
            else if (    type == EH_INPUT_VAL_RAND_USER
                      && !(data[1][0]<=0 && data[1][n_cols-1]>=1. ) )
               g_set_error( &tmp_error ,
                            EH_INPUT_VAL_ERROR ,
                            EH_INPUT_VAL_ERROR_BAD_F_RANGE ,
                            "%s: CDF data must range from 0 to 1 (found [%f,%f]).\n" ,
                            file , data[1][0] , data[1][n_cols-1] );
            else
            {
               val = eh_input_val_new();

               val->type = type;
               val->file = file;
               val->len = n_cols;
               val->x = (double*)g_memdup (data[0], sizeof(double)*n_cols);
               val->y = (double*)g_memdup (data[1], sizeof(double)*n_cols);
            }
         }

         eh_free_2( data );
      }
      else if (    g_ascii_strcasecmp( equal_split[0] , "UNIFORM" ) == 0 
                || g_ascii_strcasecmp( equal_split[0] , "NORMAL"  ) == 0 
                || g_ascii_strcasecmp( equal_split[0] , "WEIBULL" ) == 0 )
      {
         char** comma_split = g_strsplit( equal_split[1] , "," , -1 );
         gint   n_vals      = g_strv_length( comma_split );

         if ( n_vals != 2 )
            g_set_error( &tmp_error ,
                         EH_INPUT_VAL_ERROR ,
                         EH_INPUT_VAL_ERROR_NOT_TWO_DIST_VALS ,
                         "%s: Two values are required to define a distribution (found %d)." ,
                         equal_split[1], n_vals );
         else
         {
            val = eh_input_val_new();

            val->data[0] = g_ascii_strtod( comma_split[0] , NULL );
            val->data[1] = g_ascii_strtod( comma_split[1] , NULL );

            if      ( g_ascii_strcasecmp( equal_split[0] , "UNIFORM" ) == 0 )
               val->type = EH_INPUT_VAL_RAND_UNIFORM;
            else if ( g_ascii_strcasecmp( equal_split[0] , "NORMAL"  ) == 0 )
               val->type = EH_INPUT_VAL_RAND_NORMAL;
            else if ( g_ascii_strcasecmp( equal_split[0] , "WEIBULL" ) == 0 )
               val->type = EH_INPUT_VAL_RAND_WEIBULL;
            else
               eh_require_not_reached();

         }
         g_strfreev( comma_split );
      }
      else
      {
         g_set_error( &tmp_error ,
                      EH_INPUT_VAL_ERROR ,
                      EH_INPUT_VAL_ERROR_BAD_DIST_KEY ,
                      "%s: Bad input val key (valid keys are %s).\n" ,
                      equal_split[0] , "Uniform, Normal, Weibull, User, and File" );
      }

      g_strfreev( equal_split );

      if ( tmp_error )
         g_propagate_error( err , tmp_error );
   }
   
   return val;
}

/** Evaluate an Eh_input_val

\param val An initialized Eh_input_val
\param ... Extra parameter for a time value if \a val is set to read from
           a time series.

\return The new value
*/
double
eh_input_val_eval( Eh_input_val val , ... )
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


