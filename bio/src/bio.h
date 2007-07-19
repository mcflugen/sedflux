#if !defined( BIO_H )
#define BIO_H

#include <glib.h>
#include "sed_sedflux.h"

#define BIO_PROGRAM_NAME     "bio"
#define BIO_MAJOR_VERSION_S  "0"
#define BIO_MINOR_VERSION_S  "1"
#define BIO_MICRO_VERSION_S  "0"
#define BIO_MAJOR_VERSION    (0)
#define BIO_MINOR_VERSION    (1)
#define BIO_MICRO_VERSION    (0)

void bioturbate           ( double** col , gint n_grains , gint n_layers , double dz , double k , double total_t );
void sed_column_bioturbate( Sed_column c , double depth , double k , double duration );

#endif /* bio.h */
