#ifndef __EH_GRID_H__
#define __EH_GRID_H__

#include <stdio.h>
#include <glib.h>
#include <utils/eh_types.h>

new_handle( Eh_grid );
derived_handle( Eh_grid , Eh_dbl_grid );
derived_handle( Eh_grid , Eh_int_grid );

typedef gboolean (*Populate_func)( double , double , gpointer );

/** A point defined by its indices, (i,j)
*/
typedef struct
{
   int i; //< i-index
   int j; //< j-index
}
Eh_ind_2;

typedef gssize Eh_grid_id;

Eh_ind_2    eh_ind_2_create          ( int i         , int j          );
gboolean    eh_ind_2_cmp             ( Eh_ind_2 a    , Eh_ind_2 b     );
Eh_ind_2*   eh_ind_2_dup             ( Eh_ind_2 *src , Eh_ind_2 *dest );

gssize      eh_grid_n_x              ( Eh_grid g );
gssize      eh_grid_n_y              ( Eh_grid g );
gssize      eh_grid_n_el             ( Eh_grid g );
gssize      eh_grid_el_size          ( Eh_grid g );
gssize      eh_grid_low_x            ( Eh_grid g );
gssize      eh_grid_low_y            ( Eh_grid g );
double*     eh_grid_x                ( Eh_grid g );
double*     eh_grid_y                ( Eh_grid g );
void*       eh_grid_row              ( Eh_grid g , gssize row );
void**      eh_grid_data             ( Eh_grid g );
double**    eh_dbl_grid_data         ( Eh_grid g );
double**    eh_dbl_grid_data_start   ( Eh_grid g );
void*       eh_grid_data_start       ( Eh_grid g );

Eh_grid     eh_grid_set_data         ( Eh_grid g , void** new_data );
Eh_grid     eh_grid_set_x_lin        ( Eh_grid g , double x_0 , double dx );
Eh_grid     eh_grid_set_y_lin        ( Eh_grid g , double y_0 , double dy );

Eh_grid     eh_grid_malloc           ( gssize n_x   , gssize n_y , gssize size );
Eh_grid     eh_grid_resize           ( Eh_grid g    , gssize n_x , gssize n_y );
Eh_grid     eh_grid_add_row          ( Eh_grid g    , void* new_row );
Eh_grid     eh_grid_add_column       ( Eh_grid g    , void* new_column );
void        eh_grid_free_data        ( Eh_grid g    , gboolean free_data );
Eh_grid     eh_grid_destroy          ( Eh_grid g    , gboolean free_data );

void        eh_grid_dump             ( FILE *fp     , Eh_grid g );
Eh_grid     eh_grid_load             ( FILE *fp );

gboolean    eh_grid_cmp_data         ( Eh_grid g_1  , Eh_grid g_2 );
gboolean    eh_grid_cmp_x_data       ( Eh_grid g_1  , Eh_grid g_2 );
gboolean    eh_grid_cmp_y_data       ( Eh_grid g_1  , Eh_grid g_2 );
gboolean    eh_grid_cmp              ( Eh_grid g_1  , Eh_grid g_2 );
gboolean    eh_dbl_grid_cmp          ( Eh_dbl_grid g_1 , Eh_dbl_grid g_2 , double eps );

Eh_grid     eh_grid_dup              ( Eh_grid g );
Eh_grid     eh_grid_copy             ( Eh_grid dest , Eh_grid src );
Eh_grid     eh_grid_copy_data        ( Eh_grid dest , Eh_grid src );

Eh_grid     eh_grid_reindex          ( Eh_grid g    , gssize low_x , gssize low_y );
gboolean    eh_grid_is_in_domain     ( Eh_grid g    , gssize i , gssize j );
gboolean    eh_grid_is_same_size     ( Eh_grid g_1  , Eh_grid g_2 );
Eh_grid_id  eh_grid_sub_to_id        ( gssize n_j   , gssize i , gssize j );
Eh_ind_2    eh_grid_id_to_sub        ( gssize n_i   , Eh_grid_id id );

void        eh_dbl_grid_set_val      ( Eh_grid g , gssize i , gssize j , double val );
void        eh_int_grid_set_val      ( Eh_grid g , gssize i , gssize j , int val    );
double      eh_dbl_grid_val          ( Eh_dbl_grid g   , gssize i     , gssize j            );
int         eh_int_grid_val          ( Eh_int_grid g   , gssize i     , gssize j            );
void*       eh_grid_loc              ( Eh_grid     g   , gssize i     , gssize j            );

int         eh_dbl_grid_write        ( FILE* fp        , Eh_dbl_grid g                      );
gboolean    eh_grid_is_compatible    ( Eh_grid g_1     , Eh_grid g_2                        );
void        eh_grid_foreach          ( Eh_grid g       , GFunc func    , gpointer user_data );

Eh_dbl_grid eh_dbl_grid_add          ( Eh_dbl_grid g_1 , Eh_dbl_grid g_2                    );
Eh_dbl_grid eh_dbl_grid_subtract     ( Eh_dbl_grid g_1 , Eh_dbl_grid g_2                    );
double      eh_dbl_grid_sum          ( Eh_dbl_grid g                                        );
double      eh_dbl_grid_sum_bad_val  ( Eh_dbl_grid g   , double bad_val                     );
Eh_dbl_grid eh_dbl_grid_set          ( Eh_dbl_grid g   , double val                         );
Eh_dbl_grid eh_dbl_grid_randomize    ( Eh_dbl_grid g );
void        eh_dbl_grid_scalar_mult  ( Eh_dbl_grid g   , double val                         );
Eh_dbl_grid eh_dbl_grid_rotate       ( Eh_dbl_grid g   , double angle  ,
                                       gssize i_0      , gssize j_0    , double* err        );
Eh_dbl_grid eh_dbl_grid_reduce       ( Eh_dbl_grid g   , gssize new_nx , gssize new_ny      );
Eh_dbl_grid eh_dbl_grid_expand       ( Eh_dbl_grid g   , gssize new_nx , gssize new_ny      );
Eh_dbl_grid eh_dbl_grid_remesh       ( Eh_dbl_grid g   , gssize new_nx , gssize new_ny      );

void        interpolate_2            ( Eh_dbl_grid source , Eh_dbl_grid dest ) G_GNUC_DEPRECATED;
void        interpolate_2_bad_val    ( Eh_dbl_grid source , Eh_dbl_grid dest , double bad_val ) G_GNUC_DEPRECATED;

Eh_grid_id* eh_dbl_grid_line_ids     ( Eh_dbl_grid g   , gssize i_0 , gssize j_0 ,
                                                         gssize i_1 , gssize j_1 );
gssize      eh_grid_path_len         ( gssize* p );
gboolean    eh_grid_path_is_same     ( gssize* p_1 , gssize* p_2 );
Eh_grid     eh_grid_sub              ( Eh_grid g       , gssize i_0    , gssize j_0 ,

                                                         gssize n_x    , gssize n_y         );
void        eh_dbl_grid_rebin        ( Eh_dbl_grid src , Eh_dbl_grid dest                   );
void        eh_dbl_grid_rebin_bad_val( Eh_dbl_grid src , Eh_dbl_grid dest , double val      );

Eh_dbl_grid eh_dbl_grid_populate     ( Eh_dbl_grid g   , Populate_func f , gpointer user_data );

void        eh_dbl_grid_fprintf      ( FILE* fp        , const gchar* format , Eh_dbl_grid g );
Eh_grid     eh_grid_transpose        ( Eh_grid g );
Eh_dbl_grid eh_dbl_grid_diff         ( Eh_dbl_grid g , gssize n , gssize dim );

Eh_dbl_grid eh_dbl_grid_new_set      ( gint n_x , gint n_y , double** d );

#define eh_grid_val( g , t , i , j ) ( *((t*)eh_grid_loc(g,i,j)) )
#define eh_grid_new( t , n_x , n_y ) eh_grid_malloc( (n_x) , (n_y) , sizeof(t) )
#define eh_grid_new_uniform( t , n_x , n_y , dx , dy ) eh_grid_malloc_uniform( (n_x) , (n_y) , sizeof(t) , dx , dy )

#endif
