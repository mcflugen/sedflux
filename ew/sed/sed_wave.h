#if !defined( SED_WAVE_H )
#define SED_WAVE_H

#include <glib.h>

#include "utils/eh_types.h"

new_handle( Sed_wave );
new_handle( Sed_ocean_storm );

Sed_wave sed_wave_new    ( double h , double k , double w );
Sed_wave sed_wave_copy   ( Sed_wave dest , Sed_wave src );
Sed_wave sed_wave_dup    ( Sed_wave src );
gboolean sed_wave_is_same( Sed_wave w_1 , Sed_wave w_2 );
Sed_wave sed_wave_destroy( Sed_wave w );

double   sed_wave_height         ( Sed_wave w );
double   sed_wave_number         ( Sed_wave w );
double   sed_wave_length         ( Sed_wave w );
double   sed_wave_frequency      ( Sed_wave w );
double   sed_wave_period         ( Sed_wave w );
double   sed_wave_phase_velocity ( Sed_wave w );
gboolean sed_wave_is_bad         ( Sed_wave w );

Sed_wave sed_gravity_wave_new                   ( Sed_wave w_infinity , double h , Sed_wave new_wave );
Sed_wave sed_gravity_wave_set_frequency         ( Sed_wave a , double w , double h);
Sed_wave sed_gravity_wave_set_number            ( Sed_wave w , double k , double h);
Sed_wave sed_gravity_wave_set_height            ( Sed_wave w , Sed_wave w_infinity , double h );
double   sed_gravity_wave_deep_water_height     ( Sed_wave w );
double   sed_gravity_wave_deep_water_wave_number( Sed_wave w );

gboolean sed_wave_is_breaking( Sed_wave w , double h );

double   sed_wave_break_depth( Sed_wave w );

double   sed_dispersion_relation_frequency  ( double water_depth , double wave_number );
double   sed_dispersion_relation_wave_number( double water_depth , double frequency   );

Sed_ocean_storm sed_ocean_storm_new    ( void );
Sed_ocean_storm sed_ocean_storm_destroy( Sed_ocean_storm s );

gssize    sed_ocean_storm_index              ( Sed_ocean_storm s );
double    sed_ocean_storm_val                ( Sed_ocean_storm s );
double    sed_ocean_storm_duration           ( Sed_ocean_storm s );
double    sed_ocean_storm_duration_in_seconds( Sed_ocean_storm s );
double    sed_ocean_storm_wave_height        ( Sed_ocean_storm s );
double    sed_ocean_storm_wave_number        ( Sed_ocean_storm s );
double    sed_ocean_storm_wave_length        ( Sed_ocean_storm s );
double    sed_ocean_storm_wave_freq          ( Sed_ocean_storm s );
double    sed_ocean_storm_wave_period        ( Sed_ocean_storm s );
double    sed_ocean_storm_phase_velocity     ( Sed_ocean_storm s );

Sed_ocean_storm sed_ocean_storm_set_wave     ( Sed_ocean_storm s , Sed_wave w        );
Sed_ocean_storm sed_ocean_storm_set_index    ( Sed_ocean_storm s , gssize ind        );
Sed_ocean_storm sed_ocean_storm_set_duration ( Sed_ocean_storm s , double dt_in_days );
Sed_ocean_storm sed_ocean_storm_set_val      ( Sed_ocean_storm s , double val        );

gssize sed_ocean_storm_fprint( FILE* fp , Sed_ocean_storm s );

#endif
