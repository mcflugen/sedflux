#if !defined( SED_PROPERTY_FILE_H )
#define SED_PROPERTY_FILE_H

#include "utils/eh_types.h"
#include "sed_property.h"
#include "sed_cube.h"

new_handle( Sed_property_file_attr );
new_handle( Sed_property_file_header );
new_handle( Sed_property_file );

typedef enum
{
   SED_TYPE_UINT8  = 0,
   SED_TYPE_UINT16 = 1,
   SED_TYPE_UINT32 = 2,
   SED_TYPE_FLOAT  = 3,
   SED_TYPE_DOUBLE = 4
}
Sed_data_type;

typedef double (*Sed_get_val_func) ( Sed_cell , double , gpointer );

Sed_property_file sed_property_file_new( const char* file , Sed_property p , Sed_property_file_attr a );
Sed_property_file sed_property_file_destroy( Sed_property_file f );
gssize sed_property_file_write( Sed_property_file sed_fp , Sed_cube p );

Sed_property_file_attr sed_property_file_attr_new( );
Sed_property_file_attr sed_property_file_attr_copy( Sed_property_file_attr dest , Sed_property_file_attr src );
Sed_property_file_attr sed_property_file_attr_dup( Sed_property_file_attr src );
Sed_property_file_attr sed_property_file_attr_destroy( Sed_property_file_attr a );
Sed_property_file_header sed_property_file_header_destroy( Sed_property_file_header h );

#endif

