#if !defined( SEDFLUX_H )
#define SEDFLUX_H

#include <glib.h>

typedef struct
{
   gboolean mode_3d;
   gboolean mode_2d;
   gchar*   init_file;
   gchar*   out_file;
   gchar*   working_dir;
   gchar*   run_desc;
   gboolean just_plume;
   gboolean just_rng;
   gboolean summary;
   gboolean warn;
   gint     verbosity;
   gboolean verbose;
   gboolean version;
   char**   active_procs;
}
Sedflux_param_st;

typedef enum
{
   SEDFLUX_ERROR_BAD_PARAM ,
   SEDFLUX_ERROR_UNKNOWN_FILE_TYPE ,
   SEDFLUX_ERROR_BAD_FILE_TYPE ,
   SEDFLUX_ERROR_BAD_ALGORITHM ,
   SEDFLUX_ERROR_BAD_DIR ,
   SEDFLUX_ERROR_BAD_INIT_FILE ,
   SEDFLUX_ERROR_MULTIPLE_MODES ,
   SEDFLUX_ERROR_PROCESS_FILE_CHECK
}
Sedflux_error;

#define SEDFLUX_ERROR sedflux_error_quark()

typedef gint32 Sedflux_run_flag;

#define SEDFLUX_RUN_FLAG_SUMMARY (1)
#define SEDFLUX_RUN_FLAG_WARN    (2)

GQuark sedflux_error_quark( void );

//gboolean          sedflux                    ( const gchar* init_file , Sedflux_run_flag flag );
gboolean          sedflux                    ( const gchar* init_file );

Sedflux_param_st* sedflux_parse_command_line ( int argc , char *argv[] , GError** error );
gboolean          sedflux_setup_project_dir  ( gchar** init_file , gchar** working_dir , GError** error );
gint              sedflux_print_info_file    ( const gchar* init_file , const gchar* wd ,
                                               const gchar* cmd_str   , const gchar* desc );

#endif /* SEDFLUX_H */
