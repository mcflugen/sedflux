//---
//
// This file is part of sedflux.
//
// sedflux is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// sedflux is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with sedflux; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//---

#define SED_RIVER_PROC_NAME "river"
#define EH_LOG_DOMAIN SED_RIVER_PROC_NAME

#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "my_processes.h"
#include "sedflux.h"

gboolean
init_river_data(Sed_process proc, Sed_cube prof, GError** error);

Sed_process_info
run_river(Sed_process proc, Sed_cube prof)
{
    River_t*         data = (River_t*)sed_process_user_data(proc);
    Sed_process_info info = SED_EMPTY_INFO;
    Sed_hydro        river_data;

    if (sed_process_run_count(proc) == 0) {
        init_river_data(proc, prof, NULL);
    }

    if (data->type == SED_HYDRO_EXTERNAL) {
        river_data = sed_hydro_dup(sed_cube_external_river(prof));
    } else {
        river_data = sed_hydro_file_read_record(data->fp_river);
    }

    if (river_data) {
        gint i;
        double dt, volume, volume_to_remove, mass_removed;
        double susp_mass, bedload_mass, init_susp_mass, init_bedload_mass;

        //---
        // Set the cube time step to be that of the river.
        //---
        //      if ( data->type & (HYDRO_USE_BUFFER|HYDRO_INLINE) )
        if (data->type == SED_HYDRO_INLINE || data->buffer_is_on) {
            const double river_dt = sed_hydro_duration_in_seconds(river_data) / S_SECONDS_PER_YEAR;
            const double cube_dt = sed_cube_time_step(prof);

            if (river_dt > cube_dt) {
                sed_hydro_set_duration(river_data, cube_dt * S_DAYS_PER_YEAR);
            }

            sed_cube_set_time_step(prof,
                sed_hydro_duration_in_seconds(river_data) / S_SECONDS_PER_YEAR);

            // sed_cube_set_time_step(prof,
            //                        sed_hydro_duration_in_seconds(river_data)/S_SECONDS_PER_YEAR);
        }

        dt = sed_cube_time_step_in_seconds(prof);

        //---
        // Keep a running total of the sediment mass added from the river.
        // This exludes erosion/deposition within the river.
        //---
        data->total_mass_from_river += sed_hydro_total_load(river_data);

        init_susp_mass         = sed_hydro_suspended_flux(river_data) * dt;
        init_bedload_mass      = sed_hydro_bedload(river_data) * dt;

        //---
        // Add any eroded sediment to the river.
        //---
        volume = sed_cube_x_res(prof)
            * sed_cube_y_res(prof)
            * sed_cell_size(sed_cube_to_add(prof));

        sed_cell_resize(sed_cube_to_add(prof), volume);
        //eh_watch_dbl (dt);
        //eh_watch_dbl (sed_cell_size (sed_cube_to_add (prof)));

        sed_hydro_add_cell(river_data, sed_cube_to_add(prof));

        //      sed_cell_resize (sed_cube_to_add (prof), volume/(sed_cube_x_res (prof)*sed_cube_y_res (prof)));
        sed_cell_clear(sed_cube_to_add(prof));

        //eh_watch_dbl (sed_hydro_total_load( river_data ));
        //---
        // Remove any sediment that was deposited within the river.
        //---
        volume_to_remove = sed_cube_x_res(prof)
            * sed_cube_y_res(prof)
            * sed_cell_size(sed_cube_to_remove(prof));

        sed_cell_resize(sed_cube_to_remove(prof), volume_to_remove);
        sed_hydro_subtract_cell(river_data, sed_cube_to_remove(prof));
        sed_cell_clear(sed_cube_to_remove(prof));

        susp_mass         = sed_hydro_suspended_flux(river_data) * dt;
        bedload_mass      = sed_hydro_bedload(river_data) * dt;
        data->total_mass += susp_mass + bedload_mass;

        mass_removed     = susp_mass + bedload_mass - (init_susp_mass + init_bedload_mass);

        //sed_river_set_hydro( data->this_river , river_data );
        sed_cube_river_set_hydro(prof, data->this_river, river_data);
        //eh_message ("set river...");
        /*
              printf ( "time         : %f\n" , sed_cube_age_in_years(prof) );
              printf ( "duration     : %f\n" , sed_cube_time_step_in_years(prof) );
              printf ( "velocity     : %f\n" , sed_hydro_velocity(river_data) );
              printf ( "width        : %f\n" , sed_hydro_width   (river_data) );
              printf ( "depth        : %f\n" , sed_hydro_depth   (river_data) );
              printf ( "bedload      : %f\n" , sed_hydro_bedload (river_data) );
              for ( i=0 ; i<sed_hydro_size(river_data) ; i++ )
                 printf ( "conc[%d]      : %f\n" , i , sed_hydro_nth_concentration(river_data,i) );
              printf ( "eroded sediment added (m^3): %g\n"        , volume );
              printf ( "sediment removed (kg): %g\n"              , mass_removed );
              printf ( "suspended mass (kg): %g\n"                , susp_mass );
              printf ( "bedload mass (kg): %g\n"                  , bedload_mass );
              printf ( "total sediment added to basin (kg): %g\n" , data->total_mass );
              printf ( "total sediment added to river (kg): %g\n" , data->total_mass_from_river );
        */

        eh_message("time         : %f", sed_cube_age_in_years(prof));
        eh_message("duration     : %f", sed_cube_time_step_in_years(prof));
        eh_message("velocity     : %f", sed_hydro_velocity(river_data));
        eh_message("width        : %f", sed_hydro_width(river_data));
        eh_message("depth        : %f", sed_hydro_depth(river_data));
        eh_message("bedload      : %f", sed_hydro_bedload(river_data));

        for (i = 0 ; i < sed_hydro_size(river_data) ; i++) {
            eh_message("conc[%d]      : %f", i, sed_hydro_nth_concentration(river_data, i));
        }

        eh_message("eroded sediment added (m^3): %g", volume);
        eh_message("sediment removed (kg): %g", mass_removed);
        eh_message("suspended mass (kg): %g", susp_mass);
        eh_message("bedload mass (kg): %g", bedload_mass);
        eh_message("total sediment added to basin (kg): %g", data->total_mass);
        eh_message("total sediment added to river (kg): %g", data->total_mass_from_river);

        // NOTE: this will be freed when the river file is closed.
        //      hydro_destroy_hydro_record( river_data );
        //      river_data = NULL;

    } else {
        eh_require_not_reached();
    }

    sed_hydro_destroy(river_data);

    return info;
}

#define RIVER_KEY_FILE_TYPE  "river values"
#define RIVER_KEY_RIVER_FILE "river file"
#define RIVER_KEY_RIVER_NAME "river name"

static const gchar* river_req_labels[] = {
    RIVER_KEY_FILE_TYPE,
    RIVER_KEY_RIVER_FILE,
    RIVER_KEY_RIVER_NAME,
    NULL
};

gboolean
init_river(Sed_process p, Eh_symbol_table tab, GError** error)
{
    River_t* data    = sed_process_new_user_data(p, River_t);
    GError*  tmp_err = NULL;
    gboolean is_ok   = TRUE;
    gchar*   str;

    eh_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    data->total_mass            = 0;
    data->total_mass_from_river = 0;
    data->fp_river              = NULL;
    data->this_river            = NULL;

    if (eh_symbol_table_require_labels(tab, river_req_labels, &tmp_err)) {
        gchar* prefix = sed_process_prefix(p);
        gchar* file = NULL;

        if (!prefix) {
            prefix = g_strdup(".");
        }

        file = eh_symbol_table_value(tab, RIVER_KEY_RIVER_FILE);
        str = eh_symbol_table_lookup(tab, RIVER_KEY_FILE_TYPE);
        data->river_name = eh_symbol_table_value(tab, RIVER_KEY_RIVER_NAME);

        if (file[0] == '/') {
            data->filename = g_strdup(file);
        } else {
            data->filename = g_build_filename(prefix, file, NULL);
        }

        g_free(file);
        g_free(prefix);

        data->location     = 0;
        data->buffer_is_on = FALSE;

        data->type = sed_hydro_str_to_type(str);

        if (data->type != SED_HYDRO_EXTERNAL) {
            if ((g_ascii_strcasecmp(str, "EVENT") == 0
                    || g_ascii_strcasecmp(str, "BUFFER") == 0)
                && data->type == SED_HYDRO_HYDROTREND) {
                data->buffer_is_on = TRUE;
            }

            if (data->type == SED_HYDRO_UNKNOWN)
                g_set_error(&tmp_err, SEDFLUX_ERROR, SEDFLUX_ERROR_BAD_PARAM,
                    "Invalid river type key (season, hydrotrend, or event): %s", str);

            if (!tmp_err) {
                eh_touch_file(data->filename, O_RDONLY, &tmp_err);
            }

            if (!tmp_err) {
                Sed_hydro_file_type t = sed_hydro_file_guess_type(data->filename, &tmp_err);

                if (!tmp_err) {
                    if ((t == SED_HYDRO_HYDROTREND_BE && G_BYTE_ORDER != G_BIG_ENDIAN)
                        || (t == SED_HYDRO_HYDROTREND_LE && G_BYTE_ORDER != G_LITTLE_ENDIAN))
                        g_set_error(&tmp_err, SEDFLUX_ERROR, SEDFLUX_ERROR_BAD_FILE_TYPE,
                            "Byte-order of river file type doesn't match machine's: "
                            "Machine is %s but file is %s",
                            G_BYTE_ORDER == G_BIG_ENDIAN ? "big-endian" : "little-endian",
                            G_BYTE_ORDER == G_BIG_ENDIAN ? "little-endian" : "big-endian");
                }

                if (t == SED_HYDRO_HYDROTREND_BE || t == SED_HYDRO_HYDROTREND_LE) {
                    t = SED_HYDRO_HYDROTREND;
                }

                if (!tmp_err && t != data->type) {
                    if (t == SED_HYDRO_UNKNOWN)
                        g_set_error(&tmp_err, SEDFLUX_ERROR, SEDFLUX_ERROR_UNKNOWN_FILE_TYPE,
                            "Could not determine file type of river file: %s", data->filename);
                    else
                        g_set_error(&tmp_err, SEDFLUX_ERROR, SEDFLUX_ERROR_BAD_FILE_TYPE,
                            "River file type doesn't match the specified type: "
                            "Specified type is %s but file is type %s", sed_hydro_type_to_s(data->type),
                            sed_hydro_type_to_s(t));
                }
            }
        }
    }

    if (tmp_err) {
        g_propagate_error(error, tmp_err);
        is_ok = FALSE;
    }

    return is_ok;
}

gboolean
init_river_data(Sed_process proc, Sed_cube prof, GError** error)
{
    River_t* data = (River_t*)sed_process_user_data(proc);

    if (data) {
        Sed_riv new_river;
        data->total_mass            = 0;
        data->total_mass_from_river = 0;
        data->fp_river              = sed_hydro_file_new(data->filename, data->type,
                data->buffer_is_on, TRUE, error);
        data->prof = prof;

        //data->this_river            = sed_river_new     ( data->river_name );

        new_river = sed_river_new(data->river_name);
        eh_require(data->fp_river);

        data->this_river = sed_cube_add_trunk(prof, new_river);

        sed_river_destroy(new_river);
    }

    return TRUE;
}

gboolean
destroy_river(Sed_process p)
{
    if (p) {
        River_t* data = (River_t*)sed_process_user_data(p);

        if (data) {
            sed_cube_remove_trunk(data->prof, data->this_river);
            sed_hydro_file_destroy(data->fp_river);
            eh_free(data->filename);
            eh_free(data->river_name);
            eh_free(data);
        }
    }

    return TRUE;
}

gboolean
dump_river_data(gpointer ptr, FILE* fp)
{
    River_t* data = (River_t*)ptr;
    guint len;

    fwrite(data, sizeof(River_t), 1, fp);

    len = strlen(data->filename) + 1;
    fwrite(&len, sizeof(guint), 1, fp);
    fwrite(data->filename, sizeof(char), len, fp);

    return TRUE;
}

gboolean
load_river_data(gpointer ptr, FILE* fp)
{
    River_t* data = (River_t*)ptr;
    guint len;

    fread(data, sizeof(River_t), 1, fp);

    fread(&len, sizeof(guint), 1, fp);
    fread(data->filename, sizeof(char), len, fp);

    return TRUE;
}


/***********************************************************************
*                                                                      *
* Function :                                                           *
*                                                                      *
*  READRIVER                                                           *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Description :                                                        *
*                                                                      *
*  Read the river data for the next time step.  The data file in this  *
*  case is the binary output file from HYDROTREND.                     *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Formal parameters :                                                  *
*                                                                      *
*  runPrefix - The file name prefix for the river data file.           *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Global Variables :                                                   *
*                                                                      *
*  NONE                                                                *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Local Variables :                                                    *
*                                                                      *
*  hydroData   - Structure containing the river data for this time     *
*                step.                                                 *
*  hydroHeader - Structure containing the header for the data file.    *
*  first_time  - Flag indicating if this is the first time through the *
*                function.                                             *
*  prefix      - The file name prefix for the data file.               *
*  extension   - The file name extension for the data file.            *
*  fpRiver     - Pointer to the data file.                             *
*                                                                      *
***********************************************************************/

#if defined(IGNORE)

#include <stdio.h>
#include <string.h>
//#include <values.h>
#include "hydro.h"

Hydro_record*
read_river(FILE* fpRiver)
{
    Hydro_record* hydro_data;
    static Hydro_header hydro_header;

    if (ftell(fpRiver) == 0) {
        hydro_header = readHydroHeader(fpRiver);
    }

    hydro_data = hydro_create_hydro_record(hydro_header.nGrain);

    if (readHydroRecord(fpRiver, hydro_data, hydro_header.nGrain)) {
        return hydro_data;
    } else {
        hydro_destroy_hydro_record(hydro_data);
        return NULL;
    }
}

/***********************************************************************
*                                                                      *
* Function :                                                           *
*                                                                      *
*  READRIVERINLINE                                                     *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Description :                                                        *
*                                                                      *
*  Read the river data for the next time step.  The data file in this  *
*  case is an ascii file.  An example file follows.                    *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Formal parameters :                                                  *
*                                                                      *
*  fpRiver - Pointer to the input file.                                *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Global Variables :                                                   *
*                                                                      *
*  NONE                                                                *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Local Variables :                                                    *
*                                                                      *
*  line      - The next line read from the input file.                 *
*  riverData - Structure of the river data for the next time step.     *
*  val       - The next number value read from the input file.         *
*  nGrains   - The number of grain types provided by the river (both   *
*              suspended load and bedload.                             *
*                                                                      *
***********************************************************************/

/***********************************************************************
*                                                                      *
* Example input file:                                                  *
*                                                                      *
*  Number of Grains: 5                                                 *
*  -- Grain #1 (bedload) ---                                           *
*  Bulk Density (kg/m^3):      1850                                    *
*  Grain Size (microns):       200                                     *
*  Removal Rate (1/day):       50                                      *
*  Diffusion coefficient (-):  .25                                     *
*  -- Grain #2 ---*                                                    *
*  Bulk Density (kg/m^3):      1750                                    *
*  Grain Size (microns):       75                                      *
*  Removal Rate (1/day):       16.8                                    *
*  Diffusion coefficient (-):  .25                                     *
*                                                                      *
***********************************************************************/

#include <stdio.h>
#include <string.h>
//#include <values.h>
#include "hydro.h"

Hydro_record*
read_river_inline(FILE* fpRiver)
{
    char line[S_LINEMAX];
    Hydro_record* river_data;
    double val;
    int nGrains;

    if (read_int_vector(fpRiver, &nGrains, 1) != 1) {
        return NULL;
    }

    river_data = hydro_create_hydro_record(nGrains - 1);

    read_double_vector(fpRiver, &river_data->bedload, 1);
    read_double_vector(fpRiver, river_data->conc, (int)(nGrains - 1));
    read_double_vector(fpRiver, &river_data->velocity, 1);
    read_double_vector(fpRiver, &river_data->width, 1);
    read_double_vector(fpRiver, &river_data->depth, 1);

    return river_data;

}

#endif
