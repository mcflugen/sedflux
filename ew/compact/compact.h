#ifndef __COMPACT_H__
#define __COMPACT_H__

#include <glib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>

G_BEGIN_DECLS

typedef enum {
    COMPACT_ERROR_INPUT_FILE
}
Compact_error;

#define COMPACT_ERROR compact_error_quark()
GQuark
compact_error_quark(void);
int
compact(Sed_column col);
gboolean
compact_cube(Sed_cube cube);

#define COMPACTION_PROGRAM_NAME     "compact"
#define COMPACTION_MAJOR_VERSION    1
#define COMPACTION_MINOR_VERSION    0
#define COMPACTION_MICRO_VERSION    0

G_END_DECLS

#endif
