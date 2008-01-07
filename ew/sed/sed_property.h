#if !defined( SED_PROPERTY_H )
#define SED_PROPERTY_H

#include "utils/eh_types.h"

new_handle( Sed_property );

#include "sed_cell.h"
#include "sed_sediment.h"

Sed_property sed_property_new_full  ( char* name , char* ext , Sed_cell_property_func f , gssize n_args );
Sed_property sed_property_new       ( const char *name );
Sed_property sed_property_copy      ( Sed_property dest , Sed_property src );
Sed_property sed_property_dup       ( Sed_property src );
gboolean     sed_property_is_named  ( Sed_property p , const char* name );
char*        sed_property_extension ( Sed_property p );
char*        sed_property_name      ( Sed_property p );
gssize       sed_property_n_args    ( Sed_property p );
double       sed_property_measure   ( Sed_property p , Sed_cell c , ... );
Sed_property sed_property_destroy   ( Sed_property p );

#endif

