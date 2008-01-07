#if !defined( BIO_H )
#define BIO_H

#include <glib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>

#define BIO_PROGRAM_NAME     "bio"
#define BIO_MAJOR_VERSION_S  "0"
#define BIO_MINOR_VERSION_S  "1"
#define BIO_MICRO_VERSION_S  "0"
#define BIO_MAJOR_VERSION    (0)
#define BIO_MINOR_VERSION    (1)
#define BIO_MICRO_VERSION    (0)

typedef enum
{
   BIO_ERROR_BAD_PARAM ,
   BIO_ERROR_BAD_ALGORITHM
}
Bio_error;

#define BIO_ERROR bio_error_quark()
GQuark bio_error_quark( void );

typedef enum
{
   BIO_METHOD_DIFFUSION ,
   BIO_METHOD_CONVEYOR  ,
   BIO_METHOD_UNKNOWN
}
Bio_method;

GOptionGroup* bio_get_option_group( void );
void     bioturbate           ( double** col , gint n_grains , gint n_layers , double dz , double k , double total_t );
//void     sed_column_bioturbate( Sed_column c , double depth , double k , double duration , Bio_method m );
double** bio_diffuse_layers ( double* t , gint n_layers , double dz , double k , double duration );
double** bio_conveyor_layers( double* t , gint n_layers , double dz , double r , double duration );

Sed_proc_init    bio_init;
Sed_proc_run     bio_run;
Sed_proc_destroy bio_destroy;

#endif /* bio.h */
