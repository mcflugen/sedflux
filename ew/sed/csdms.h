#if !defined(CSDMS_H)
# define CSDMS_H

#include <glib.h>

typedef struct _CSDMSComp CSDMSComp;

/** Contains the public fields of a CSDMS Component
*/
struct _CSDMSComp
{
   gpointer user_data; /**< A pointer to data user data to associate with component */
};

/** The initialization function for a CSDMSComp

\param[in,out]  c      A CSDMSComp
\param[in]      file   Name of an initialization file

\returns TRUE if initialization was successful.  FALSE otherwise
*/
typedef gboolean (CSDMSInitFunc)     ( CSDMSComp* c , const gchar* file );

/** The get values function for a CSDMSComp

\param[in,out]  c        A CSDMSComp
\param[in]      val_s    A String that indicates the type of value to obtain
\param[in]      where    -1-terminated array of indices that indicate where values should be found
\param[in]      when     Time (in years) when values are to be found
\param[out]     vals     Pointer to an arrya of obtained values

\returns TRUE if get values was successful.  FALSE otherwise
*/
typedef gboolean (CSDMSGetValFunc)   ( CSDMSComp* c , const gchar* val_s , gint* where , double when , double** vals );

typedef gboolean (CSDMSSetValFunc)   ( CSDMSComp* c , const gchar* val_s , gint* where , double* vals );

/** The finalize function for a CSDMSComp

\param[in,out]   c      A CSDMSComp

\returns TRUE if finalization was successful.  FALSE otherwise
*/
typedef gboolean (CSDMSFinalizeFunc) ( CSDMSComp* );

typedef struct _CSDMSRealComp CSDMSRealComp;

struct _CSDMSRealComp
{
   gpointer           user_data;

   CSDMSInitFunc*     init;
   CSDMSGetValFunc*   get_val;
   CSDMSSetValFunc*   set_val;
   CSDMSFinalizeFunc* finalize;
};

gboolean csdms_comp_init    ( CSDMSComp* c , const gchar* file );
gboolean csdms_comp_get_val ( CSDMSComp* c , const gchar* name , gint* here , double now , double** vals );
gboolean csdms_comp_set_val ( CSDMSComp* c , const gchar* name , const gint* here , const double* vals );
gboolean csdms_comp_finalize( CSDMSComp* c );

CSDMSComp* csdms_comp_new     ( void );
CSDMSComp* csdms_comp_destroy ( CSDMSComp* c );
CSDMSComp* csdms_comp_set_irf ( CSDMSComp* c , CSDMSInitFunc , CSDMSGetValFunc , CSDMSFinalizeFunc, CSDMSSetValFunc );

gpointer   csdms_comp_data( CSDMSComp* c );

#endif /* csdms.h is included */


