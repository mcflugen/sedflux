#include <glib.h>
#include <utils/utils.h>
#include <csdms.h>

/**
   \defgroup csdms_group CSDMS Component Interface
*/

/*@{*/

/** Initialize a CSDMSComp

\param[in,out]   c   A CSDMSComp
\param[in]       file   Name of an initialization file

\return TRUE if the component is initialized successfully.  FALSE otherwise.
*/
gboolean
csdms_comp_init(CSDMSComp* c, const gchar* file)
{
    gboolean success = FALSE;

    eh_require(c);

    if (c && file) {
        CSDMSRealComp* rc = (CSDMSRealComp*)c;

        if (rc->init) {
            success = rc->init(c, file);
        } else {
            success = TRUE;
        }
    }

    eh_require(success == TRUE || success == FALSE);

    return success;
}

/** Get values of a CSDMSComp

\param[in,out]   c      A CSDMSComp
\param[in]       name   String that indicates the value to obtain
\param[in]       here   -1-terminated array of indices where values will be found (or NULL for everywhere)
\param[in]       now    Time in years when values are found
\param[out]      vals   Pointer to an array where values will be stored

\returns TRUE if values were found successfully, FALSE otherwise.
*/
gboolean
csdms_comp_get_val(CSDMSComp* c, const gchar* name, gint* here, double now,
    double** vals)
{
    gboolean success = FALSE;

    eh_require(c);
    eh_return_val_if_fail(*vals == NULL, FALSE);

    if (c) {
        CSDMSRealComp* rc = (CSDMSRealComp*)c;

        if (rc->get_val) {
            success = rc->get_val(c, name, here, now, vals);
        } else {
            success = TRUE;
        }
    }

    eh_require(success == TRUE || success == FALSE);

    return success;
}

/** Run finalize methods for a CSDMSComp

\param[in,out]   c   A CSDMSComp

\returns TRUE if finalization was successful, FALSE otherwise.
*/
gboolean
csdms_comp_finalize(CSDMSComp* c)
{
    gboolean success = FALSE;

    if (c) {
        CSDMSRealComp* rc = (CSDMSRealComp*)c;

        if (rc->finalize) {
            success = rc->finalize(c);
        } else {
            success = TRUE;
        }

    }

    eh_require(success == TRUE || success == FALSE);

    return success;
}

/** Create a new CSDMS component

\return A newly allocated CSDMSComp that should be freed using csdms_comp_destroy
*/
CSDMSComp*
csdms_comp_new(void)
{
    CSDMSRealComp* c = NULL;

    c = eh_new(CSDMSRealComp, 1);

    if (c) {
        c->init      = NULL;
        c->get_val   = NULL;
        c->finalize  = NULL;
        c->user_data = NULL;
    }

    eh_require(c);

    return (CSDMSComp*)c;
}

/** Destroy a CSDMSComp

\param[in,out]   c   A CSDMSComp

\return NULL
*/
CSDMSComp*
csdms_comp_destroy(CSDMSComp* c)
{
    if (c) {
        eh_free(c);
    }

    return NULL;
}

/** Set Initialize, Run, and Finalization functions of a CSDMSComp

\param[in,out]   c              A CSDMSComp
\param[in]       init_func      Initialization function
\param[in]       get_val_func   Get values function
\param[in]       finalize_func  Finalize function

\return The CSDMSComp
*/
CSDMSComp*
csdms_comp_set_irf(CSDMSComp* c, CSDMSInitFunc init_func, CSDMSGetValFunc get_val_func,
    CSDMSFinalizeFunc finalize_func)
{
    eh_require(c);

    if (c) {
        CSDMSRealComp* rc = (CSDMSRealComp*)c;

        rc->init     = init_func;
        rc->get_val  = get_val_func;
        rc->finalize = finalize_func;
    }

    return c;
}

/** Get the user data of a CSDMSComp

\param[in]   c   A CSDMSComp

\return The user data
*/
gpointer
csdms_comp_data(CSDMSComp* c)
{
    gpointer p = NULL;

    if (c) {
        p = ((CSDMSRealComp*)c)->user_data;
    }

    return p;
}

/*@}*/
