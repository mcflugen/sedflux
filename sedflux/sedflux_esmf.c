#include <glib.h>
#include <sed/sed_sedflux.h>
#include "sedflux.h"

Sedflux_param_st* sedflux_setup        ( gchar* command_s );
gboolean sedflux_init         ( Sed_epoch_queue* q , Sed_cube* p , const gchar* init_file );
gboolean sedflux_run_time_step( Sed_epoch_queue  q , Sed_cube  p );
gboolean sedflux_run          ( Sed_epoch_queue  q , Sed_cube  p );
gboolean sedflux_finalize     ( Sed_epoch_queue  q , Sed_cube  p );

void     esmf_sedflux_init    ( void );
void     esmf_sedflux_run     ( void );
void     esmf_sedflux_finalize( void );

static Sedflux_param_st* param     = NULL;
static gchar*            command_s = NULL;
static Sed_epoch_queue*  q         = NULL;
static Sed_cube*         p         = NULL;
static gchar*            init_file = NULL;

void esmf_sedflux_setup   ( void ) { param = sedflux_setup( command_s ); }
void esmf_sedflux_init    ( void ) { sedflux_init         (  q ,  p , init_file ); }
void esmf_sedflux_run     ( void ) { sedflux_run_time_step( *q , *p );             }
void esmf_sedflux_finalize( void ) { sedflux_finalize     ( *q , *p );             }

