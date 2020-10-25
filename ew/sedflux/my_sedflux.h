#if !defined( MY_SEDFLUX_H )
#define MY_SEDFLUX_H

#include <glib.h>
#include <sed/sed_sedflux.h>
#include "my_processes.h"
#include "bio.h"

G_BEGIN_DECLS

/** Global variable that lists all of the process that sedflux will run

sedflux will cycle through the process in the order that they are listed here.
To add a new process to sedflux, you will need to add an entry to this list.
sedflux will will then automatically cycle through your process.
*/
/*
extern Sed_process_init_t my_proc_defs[];

extern Sed_process_family my_proc_family[];

extern Sed_process_check my_proc_checks[];
*/

G_END_DECLS

#endif /* MY_SEDFLUX_H */


