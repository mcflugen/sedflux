#if !defined( MY_PROCESSES_H )
#define MY_PROCESSES_H

#include <glib.h>
#include <utils/utils.h>
#include <sed/sed_sedflux.h>
#include "sedflux.h"

G_BEGIN_DECLS

/* All of the initialize functions for my processes
*/
gboolean         init_avulsion    ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_bbl         ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_bedload     ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_bioturbation( Sed_process , Eh_symbol_table , GError** );
gboolean         init_compaction  ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_constants   ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_cpr         ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_data_dump   ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_debris_flow ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_diffusion   ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_erosion     ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_failure     ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_flow        ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_isostasy    ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_met_station ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_new_process ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_plume       ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_plume_hypo  ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_inflow      ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_quake       ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_sea_level   ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_squall      ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_storm       ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_subsidence  ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_tide        ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_river       ( Sed_process , Eh_symbol_table , GError** );
gboolean         init_xshore      ( Sed_process , Eh_symbol_table , GError** );

/* All of the run functions for my processes
*/
Sed_process_info run_avulsion     ( Sed_process , Sed_cube );
Sed_process_info run_bbl          ( Sed_process , Sed_cube );
Sed_process_info run_bedload      ( Sed_process , Sed_cube );
Sed_process_info run_bioturbation ( Sed_process , Sed_cube );
Sed_process_info run_compaction   ( Sed_process , Sed_cube );
Sed_process_info run_constants    ( Sed_process , Sed_cube );
Sed_process_info run_cpr          ( Sed_process , Sed_cube );
Sed_process_info run_data_dump    ( Sed_process , Sed_cube );
Sed_process_info run_debris_flow  ( Sed_process , Sed_cube );
Sed_process_info run_diffusion    ( Sed_process , Sed_cube );
Sed_process_info run_erosion      ( Sed_process , Sed_cube );
Sed_process_info run_failure      ( Sed_process , Sed_cube );
Sed_process_info run_flow         ( Sed_process , Sed_cube );
Sed_process_info run_isostasy     ( Sed_process , Sed_cube );
Sed_process_info run_met_station  ( Sed_process , Sed_cube );
Sed_process_info run_new_process  ( Sed_process , Sed_cube );
Sed_process_info run_plume        ( Sed_process , Sed_cube );
Sed_process_info run_plume_hypo   ( Sed_process , Sed_cube );
Sed_process_info run_plume_hyper_inflow( Sed_process , Sed_cube );
Sed_process_info run_plume_hyper_sakura( Sed_process , Sed_cube );
Sed_process_info run_inflow       ( Sed_process , Sed_cube );
Sed_process_info run_sakura       ( Sed_process , Sed_cube );
Sed_process_info run_quake        ( Sed_process , Sed_cube );
Sed_process_info run_river        ( Sed_process , Sed_cube );
Sed_process_info run_sea_level    ( Sed_process , Sed_cube );
Sed_process_info run_squall       ( Sed_process , Sed_cube );
Sed_process_info run_slump        ( Sed_process , Sed_cube );
Sed_process_info run_storm        ( Sed_process , Sed_cube );
Sed_process_info run_subsidence   ( Sed_process , Sed_cube );
Sed_process_info run_tide         ( Sed_process , Sed_cube );
Sed_process_info run_xshore       ( Sed_process , Sed_cube );

Sed_proc_destroy destroy_avulsion;
Sed_proc_destroy destroy_bbl;
Sed_proc_destroy destroy_bedload;
Sed_proc_destroy destroy_bioturbation;
Sed_proc_destroy destroy_compaction;
Sed_proc_destroy destroy_cpr;
Sed_proc_destroy destroy_constants;
Sed_proc_destroy destroy_data_dump;
Sed_proc_destroy destroy_debris_flow;
Sed_proc_destroy destroy_diffusion;
Sed_proc_destroy destroy_erosion;
Sed_proc_destroy destroy_failure;
Sed_proc_destroy destroy_flow;
Sed_proc_destroy destroy_inflow;
Sed_proc_destroy destroy_isostasy;
Sed_proc_destroy destroy_met_station;
Sed_proc_destroy destroy_new_process;
Sed_proc_destroy destroy_plume;
Sed_proc_destroy destroy_plume_hypo;
Sed_proc_destroy destroy_quake;
Sed_proc_destroy destroy_river;
Sed_proc_destroy destroy_sea_level;
Sed_proc_destroy destroy_squall;
Sed_proc_destroy destroy_storm;
Sed_proc_destroy destroy_subsidence;
Sed_proc_destroy destroy_tide;
Sed_proc_destroy destroy_xshore;

#define BBL_PROCESS_NAME_S    "bbl"

typedef struct
{
   int          algorithm;
   gchar*       src_file;
   Eh_sequence* src_seq;
   double       last_year;
}
Bbl_t;

typedef struct
{
   double bed_load_dump_length;
   double bedload_ratio;
   double f_retained;
   gchar* river_name;
}
Bedload_dump_t;

typedef struct
{
   Eh_input_val k;
   Eh_input_val depth;
}
Bioturbation_t;

typedef struct
{
   Eh_file_list* file_list;
   gchar*        output_dir;
}
Cpr_t;

typedef struct
{
   double  vertical_resolution;
   double  horizontal_resolution;
   double  y_lim_min, y_lim_max;
   double  x_lim_min, x_lim_max;
   gchar*  output_dir;
   GArray* property;
   int     count;
}
Data_dump_t;

typedef struct
{
   double   yield_strength;
   double   viscosity;
   double   numerical_viscosity;
   double   dt;
   double   max_time;
   Sed_cube failure;
}
Debris_flow_t;

#define EROSION_PROCESS_NAME_S      "erosion"

typedef struct
{
   gint   method;
   double k_max;
   double stream_reach;
   double stream_relief;
   double slope;
}
Erosion_t;

#include <failure.h>

GQuark failure_profile_data_quark( void );
#define FAILURE_PROFILE_DATA failure_profile_data_quark()

typedef struct
{
   double       decider_clay_fraction;
   double       consolidation;
   double       cohesion;
   double       friction_angle;
   double       gravity;
   double       density_sea_water;
   Sed_process  turbidity_current;
   Sed_process  debris_flow;
   Sed_process  slump;
   Sed_process  flow;
   Fail_profile* fail_prof;
}
Failure_proc_t;

typedef struct
{
   double      relaxation_time; /* relaxation time of the Earth's crust (years) */
   double      eet;
   double      youngs_modulus;
   double      last_time; /* the last time (in years) that the Earth was subsided */
   guint       len;
   double      last_half_load;
   Eh_dbl_grid last_dw_iso;
   Eh_dbl_grid last_load;
}
Isostasy_t;

typedef struct
{
   FILE*           fp;
   gchar*          parameter_str;
   Sed_measurement parameter;
   gboolean        from_river_mouth;
   GArray*         pos;
   gchar*          filename;
   Sed_tripod      met_fp;
}
Met_station_t;

typedef struct
{
   double   last_time;
   double   mean_quake;
   double   var_quake;
   long     rand_seed;
   GRand*   rand;
}
Quake_t;

typedef struct
{
   Sed_hydro_file   fp_river;
   char*            filename;
   Sed_hydro_file_type type;
   gboolean         buffer_is_on;
   int              location;
   double           total_mass;
   double           total_mass_from_river;
   //Sed_riv          this_river;
   gpointer         this_river;
   char*            river_name;
}
River_t;

typedef enum
{
  BOUNDARY_NONE,
  BOUNDARY_NORTH,
  BOUNDARY_SOUTH,
  BOUNDARY_EAST,
  BOUNDARY_WEST,
}
Boundary;

typedef struct
{
   Boundary     left_bound;
   Boundary     right_bound;
   Eh_input_val std_dev;
   Eh_input_val min_angle;
   Eh_input_val max_angle;
   Eh_input_val f_remain;
   gboolean     branching_is_on;
   GRand*       rand;
   guint32      rand_seed;
   gboolean     reset_angle;
   gint         hinge_i;
   gint         hinge_j;
   char*        river_name;
}
Avulsion_t;

typedef struct
{
   Eh_input_val gravity;
   Eh_input_val rho_sea_h2o;
   Eh_input_val rho_h2o;
   Eh_input_val salinity;
   Eh_input_val rho_quartz;
   Eh_input_val rho_mantle;
}
Constants_t;

typedef struct
{
   Eh_input_val k_max;
   Eh_input_val k_long_max;
   Eh_input_val k_cross_max;
   double       skin_depth;
}
Diffusion_t;

#define FLOW_PROCESS_NAME_S        "FLOW"

#define FLOW_ALGORITHM_EXPONENTIAL (1)
#define FLOW_ALGORITHM_TERZAGHI    (2)
#define FLOW_ALGORITHM_DARCY       (3)

#define FLOW_KEY_METHOD            "method"

typedef struct
{
   gint    method;
   double  last_time; // the last time (in years) that excess porewater pressure was calculated
   guint   len;
   double* old_load;
}
Flow_t;

typedef struct
{
   double param;
}
New_process_t;

#define PLUME_PROCESS_NAME_S      "plume"

typedef struct
{
   Sed_process plume_proc_hyper;
   Sed_process plume_proc_hypo;

   gchar*      hyper_name;
   gchar*      hypo_name;
}
Plume_t;

GQuark plume_hydro_data_quark( void );
#define PLUME_HYDRO_DATA plume_hydro_data_quark()

#define PLUME_HYPO_PROCESS_NAME_S "HYPOPYCNAL PLUME"

#include "plume_types.h"
#include "plumeinput.h"

typedef struct
{
   Eh_input_val  current_velocity;
   double        ocean_concentration;
   double        plume_width;
   int           ndx;
   int           ndy;

   int           deposit_size;
   Sed_cell**    deposit;
   Sed_cell**    last_deposit;
   double**      plume_deposit;
   Plume_river   last_river_data;
   Plume_data*   plume_data;

   Sed_cell_grid deposit_grid;
   Sed_cell_grid last_deposit_grid;
}
Plume_hypo_t;

typedef struct
{
   gboolean initialized;
   double   time_fraction;
   double   squall_duration;
   double   dt;
}
Squall_t;

typedef struct
{
   double   sua;
   double   sub;
   double   E_a;
   double   E_b;
   double   C_d;
   double   tan_phi;
   double   mu;
   double   rhoSW;
   double   channel_width;
   double   channel_length;

   Sed_cube failure;

//   guint    n_x;
//   guint    n_y;
//   double** deposit;
}
Inflow_t;

typedef struct
{
   gboolean     initialized;
   double       last_time;
   int          sediment_type;
   Eh_input_val xshore_current;
}
Xshore_t;

typedef struct
{
   gchar*    filename;
   double    start_year;
   double**  sea_level;
   gint      len;
}
Sea_level_t;

typedef struct
{
   gboolean initialized;
   Sed_cube failure;
}
Slump_t;

typedef struct
{
   double       last_k;
   double       last_time;
   double       alpha;
   double       beta;
   double       fraction;   
   gboolean     average_non_events;
   GRand*       rand;
   Eh_input_val wave_height;
   guint32      rand_seed;
}
Storm_t;

typedef struct
{
   gchar*       filename;
   double       last_year;
   GArray*      tectonic_curve;
   Eh_sequence* subsidence_seq;
}
Subsidence_t;

typedef struct
{
   double tidal_range;
   double tidal_period;
}
Tide_t;

G_END_DECLS

#endif /* MY_PROCESSES_H */
