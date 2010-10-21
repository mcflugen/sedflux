#if !defined( SED_TRIPOD_H )
#define SED_TRIPOD_H

#include <stdio.h>
#include <glib.h>
#include "utils/utils.h"

G_BEGIN_DECLS

new_handle( Sed_tripod );
new_handle( Sed_tripod_header );
new_handle( Sed_tripod_attr );
new_handle( Sed_measurement );

#include "sed_cube.h"
#include "sed_property_file.h"

typedef double (*Sed_tripod_func)( Sed_cube , gssize , gssize );

Sed_tripod      sed_tripod_new                 ( const char* file , Sed_measurement x , Sed_tripod_attr attr );
Sed_tripod      sed_tripod_destroy             ( Sed_tripod t );
double*         sed_tripod_measure             ( Sed_tripod t , Sed_cube c , Eh_pt_2* pos , double* data , gssize len );

Sed_tripod_attr sed_tripod_attr_new            ( );
Sed_tripod_attr sed_tripod_attr_copy           ( Sed_tripod_attr dest , Sed_tripod_attr src );
Sed_tripod_attr sed_tripod_attr_dup            ( Sed_tripod_attr src );
Sed_tripod_attr sed_tripod_attr_destroy        ( Sed_tripod_attr a );

Sed_measurement sed_measurement_new            ( const char* name );
Sed_measurement sed_measurement_copy           ( Sed_measurement dest , Sed_measurement src );
Sed_measurement sed_measurement_dup            ( Sed_measurement src );
Sed_measurement sed_measurement_destroy        ( Sed_measurement m );
char*           sed_measurement_name           ( Sed_measurement m );
gchar**         sed_measurement_all_names      (void);
gchar** sed_measurement_all_units (void);
gchar* sed_measurement_unit (gchar* name);

double          sed_measurement_make           ( Sed_measurement m , Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_slope         ( Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_water_depth   ( Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_elevation     ( Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_thickness     ( Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_basement      ( Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_grain_size    ( Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_age           ( Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_sand_fraction ( Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_silt_fraction ( Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_clay_fraction ( Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_mud_fraction  ( Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_density       ( Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_porosity      ( Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_permeability  ( Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_facies        ( Sed_cube p , gssize i , gssize j );
double          sed_measure_cube_river_mouth   ( Sed_cube p , gssize i , gssize j );

Sed_tripod_header sed_tripod_header_new          ( Sed_measurement x );
Sed_tripod_header sed_tripod_header_destroy      ( Sed_tripod_header h );
gssize            sed_tripod_header_fprint       ( FILE* fp , Sed_tripod_header h );
gssize            sed_tripod_write               ( Sed_tripod t , Sed_cube cube );

Sed_tripod sed_tripod_set_len( Sed_tripod t , gssize len );
Sed_tripod sed_tripod_set_n_x( Sed_tripod t , gssize n_x );
Sed_tripod sed_tripod_set_n_y( Sed_tripod t , gssize n_y );

G_END_DECLS

#endif
