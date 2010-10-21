#ifndef __EH_STATUS_BAR_H__
#define __EH_STATUS_BAR_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <glib.h>

typedef enum
{
   EH_STATUS_BAR_RUNNING ,
   EH_STATUS_BAR_PAUSED  ,
   EH_STATUS_BAR_STOPPED
}
Eh_status_bar_status;

//typedef struct _Eh_status_bar Eh_Status_bar;
typedef struct
{
   double*  cur;
   double*  end;
   GThread* t;
   GTimer*  timer;
   GMutex*  mutex;
   Eh_status_bar_status status;
}
Eh_status_bar;

Eh_status_bar* eh_status_bar_new       ( double* cur , double* end );
Eh_status_bar* eh_status_bar_stop      ( Eh_status_bar* b );
gboolean       eh_status_bar_is_stopped( Eh_status_bar* b );
Eh_status_bar* eh_status_bar_pause     ( Eh_status_bar* b );
Eh_status_bar* eh_status_bar_destroy   ( Eh_status_bar* b );

gchar*   eh_render_time_str( double sec );

#ifdef __cplusplus
}
#endif

#endif /* eh_status_bar.h */


