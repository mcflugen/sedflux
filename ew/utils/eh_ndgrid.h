#ifndef __EH_NDGRID_H__
#define __EH_NDGRID_H__

#include <glib.h>
#include <utils/eh_types.h>
#include <utils/eh_grid.h>

#if !defined( OLD_NDGRID )

new_handle( Eh_ndgrid );

double*     eh_ndgrid_x           ( Eh_ndgrid g      , gssize dim           );
gssize      eh_ndgrid_n           ( Eh_ndgrid g      , gssize dim           );

Eh_ndgrid   eh_ndgrid_malloc      ( gssize n_dim     , gssize el_size , ... );
void        eh_ndgrid_free_data   ( Eh_ndgrid g                             );
double      eh_ndgrid_ind         ( Eh_ndgrid g      , ...                  );
Eh_ndgrid   eh_ndgrid_reshape     ( Eh_ndgrid g      ,
                                    gssize *new_size ,
                                    gssize new_n_dim                        );
gssize      eh_ndgrid_sub_to_id   ( gssize *size     ,
                                    gssize *sub      ,
                                    gssize n_dim                            );
gssize*     eh_ndgrid_id_to_sub   ( gssize *size     ,
                                    gssize id        ,
                                    gssize n_dim                            );
void        eh_ndgrid_destroy     ( Eh_ndgrid g, gboolean free_data );
Eh_dbl_grid eh_ndgrid_to_grid     ( Eh_ndgrid g                             );
Eh_ndgrid   eh_grid_to_ndgrid     ( Eh_grid g                               );
gssize      eh_ndgrid_write       ( FILE *fp         , Eh_ndgrid g          );

double* eh_ndgrid_start (Eh_ndgrid g);

#endif

#endif

