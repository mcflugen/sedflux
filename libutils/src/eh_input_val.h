#if !defined( EH_INPUT_VAL_H )
#define EH_INPUT_VAL_H

#include "eh_types.h"

new_handle( Eh_input_val );

/** The ways an Eh_input_val is obtained
*/
typedef enum
{
/// The value is time-invarient
   EH_INPUT_VAL_SCALAR,
/// The value is given as a function of time from a user-specified file
   EH_INPUT_VAL_FILE,
/// The value is drawn from a uniform distribution
   EH_INPUT_VAL_RAND_UNIFORM,
/// The value is drawn from a normal distribution
   EH_INPUT_VAL_RAND_NORMAL,
/// The value is drawn from a weibul distribution
   EH_INPUT_VAL_RAND_WEIBULL,
/// The value is drawn from a user-defined distribution
   EH_INPUT_VAL_RAND_USER
}
Eh_input_val_type;

Eh_input_val    eh_input_val_new      (                             );
Eh_input_val    eh_input_val_destroy  ( Eh_input_val val            );
Eh_input_val    eh_input_val_set      ( const char *input_str       );
double          eh_input_val_eval     ( Eh_input_val val      , ... );

#endif

