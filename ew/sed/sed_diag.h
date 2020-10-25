#ifndef __SED_DIAG_H__
#define __SED_DIAG_H__


#include <utils/eh_utils.h>
#include <sed/sed_sedflux.h>

G_BEGIN_DECLS

new_handle(Sed_diag);

Sed_diag
sed_diag_destroy(Sed_diag d);

Sed_diag
sed_diag_new_target_cube(Sed_cube   c);
Sed_diag
sed_diag_new_target_column(Sed_column c);
Sed_diag
sed_diag_new_target_cell(Sed_cell   c);

double
sed_diag_start(Sed_diag d);
double
sed_diag_stop(Sed_diag d);
double
sed_diag_elapsed(Sed_diag d);
double
sed_diag_continue(Sed_diag d);
double
sed_diag_reset(Sed_diag d);

gint
sed_diag_fprint(FILE* fp, Sed_diag d);

G_END_DECLS

#endif
