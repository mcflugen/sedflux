#if !defined( SED_RIVER_H )
#define SED_RIVER_H

#include <glib.h>
#include "utils/utils.h"
#include "sed_hydro.h"

G_BEGIN_DECLS

new_handle( Sed_riv );

Sed_riv sed_river_new (const gchar* name);
Sed_riv sed_river_copy( Sed_riv d , Sed_riv s );
Sed_riv sed_river_dup ( Sed_riv s );

Sed_riv sed_river_set_hydro( Sed_riv s , const Sed_hydro h );
Sed_riv sed_river_set_width( Sed_riv s , double width );
Sed_riv sed_river_set_depth( Sed_riv s , double depth );
Sed_riv sed_river_set_velocity( Sed_riv s , double velocity );
Sed_riv sed_river_set_bedload( Sed_riv s , double bedload );
Sed_riv sed_river_set_angle( Sed_riv s , double a );
Sed_riv sed_river_increment_angle( Sed_riv s , double da );
Sed_riv sed_river_set_angle_limit( Sed_riv s , double a_min , double a_max );
Sed_riv sed_river_set_hinge( Sed_riv r , gint i , gint j );
Sed_riv sed_river_set_mouth( Sed_riv r , gint i , gint j );

Sed_riv sed_river_adjust_mass( Sed_riv s , double f );

double    sed_river_water_flux( Sed_riv s );
double    sed_river_sediment_load    ( Sed_riv s );
double    sed_river_suspended_load   ( Sed_riv s );
Sed_hydro sed_river_hydro            ( Sed_riv s );
gboolean  sed_river_is_hyperpycnal   ( Sed_riv s );
double    sed_river_concentration    ( Sed_riv s );
gint      sed_river_n_branches       ( Sed_riv s );
double    sed_river_width            ( Sed_riv s );
double    sed_river_depth            ( Sed_riv s );
double    sed_river_velocity         ( Sed_riv s );
double    sed_river_bedload          ( Sed_riv s );
double    sed_river_angle            ( Sed_riv s );
double    sed_river_angle_to_deg     ( Sed_riv s );
double    sed_river_min_angle        ( Sed_riv s );
double    sed_river_max_angle        ( Sed_riv s );
Eh_ind_2  sed_river_hinge            ( Sed_riv s );
Eh_ind_2  sed_river_mouth            ( Sed_riv s );
gboolean  sed_river_mouth_is         ( Sed_riv s , gint i , gint j );
gchar*    sed_river_name             ( Sed_riv s );
gchar*    sed_river_name_loc         ( Sed_riv s );
gboolean  sed_river_name_is          ( Sed_riv s , gchar* name );
gint      sed_river_name_cmp         ( Sed_riv s , const gchar* name );


gboolean sed_river_has_children( Sed_riv s );
Sed_riv sed_river_left( Sed_riv s );
Sed_riv sed_river_right( Sed_riv s );
Sed_riv sed_river_split_discharge( Sed_riv s );
Sed_riv sed_river_split( Sed_riv s );
Sed_riv sed_river_longest_branch( Sed_riv s );

Sed_riv    sed_river_destroy ( Sed_riv s );
Sed_riv*   sed_river_leaves  ( Sed_riv s );
Sed_riv*   sed_river_branches( Sed_riv s );

gint    sed_river_fprint( FILE* fp , Sed_riv s );
Sed_riv sed_river_fread ( FILE* fp , Sed_riv d );
gint    sed_river_fwrite( FILE* fp , Sed_riv s );

G_END_DECLS

#endif
