#if !defined( SED_SIGNAL_H )
#define SED_SIGNAL_H

#include <glib.h>

G_BEGIN_DECLS

typedef gint32 Sed_sig_num;

#define SED_SIG_NONE (0)
#define SED_SIG_CONT (0)
#define SED_SIG_QUIT (1)
#define SED_SIG_DUMP (2)
#define SED_SIG_CPR  (4)
#define SED_SIG_USER (8)
#define SED_SIG_NEXT (16)
#define SED_SIG_EXIT (32)

gint      sed_signal_set_action( void );
gboolean  sed_signal_is_pending( Sed_sig_num sig );
void      sed_signal_reset     ( Sed_sig_num sig );
void      sed_signal_set       ( Sed_sig_num sig );

G_END_DECLS

#endif /* SED_SIGNAL_H */
