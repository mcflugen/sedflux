#ifndef __EH_POLYGON_H__
#define __EH_POLYGON_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <glib.h>

/** A point defined by an x-y pair.
*/
typedef struct
{
   double x; //< The x-coordinate
   double y; //< The y-coordinate
}
Eh_pt_2;

typedef GList* Eh_polygon_2;

Eh_polygon_2 eh_get_rectangle_polygon    ( Eh_pt_2 center , double dx , double dy );
Eh_polygon_2 eh_get_polygon_from_grid    ( Eh_grid g , gssize i , gssize j );
void         eh_destroy_polygon          ( Eh_polygon_2 p );
GList*       eh_find_polygon_crossings   ( Eh_pt_2 start , double angle  , Eh_polygon_2 area , int in_or_out );
gboolean     is_between_angles           ( double angle , double angle_1 , double angle_2 );
gboolean     is_inside_area              ( Eh_pt_2 x , Eh_polygon_2 area );
Eh_pt_2      eh_get_unit_vector          ( double angle );
Eh_pt_2      eh_get_dir_vector           ( Eh_pt_2 point_1 , Eh_pt_2 point_2 );
Eh_pt_2      eh_get_norm_vector          ( Eh_pt_2 u );
double       eh_get_vector_length        ( Eh_pt_2 u );
double       eh_get_vector_angle         ( Eh_pt_2 u );
Eh_pt_2      eh_normalize_vector         ( Eh_pt_2 u );
double       eh_dot_vectors              ( Eh_pt_2 u , Eh_pt_2 v );
double       eh_get_angle_between_vectors( Eh_pt_2 u , Eh_pt_2 v );

/** A series of (x,y) pairs

\deprecated Don't use this anymore.  Try Eh_dbl_grid instead.
*/
typedef struct
{
   double *x;
   double *y;
   int size;
} pos_t;

pos_t* createPosVec(int size) G_GNUC_DEPRECATED;
void destroyPosVec(pos_t *v) G_GNUC_DEPRECATED;
double *derivative(pos_t v) G_GNUC_DEPRECATED;

Eh_pt_2  eh_create_pt_2( double x , double y );
gboolean eh_cmp_pt_2   ( Eh_pt_2 a , Eh_pt_2 b , double eps );

#ifdef __cplusplus
}
#endif

#endif
