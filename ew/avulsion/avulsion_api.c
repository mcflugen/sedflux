#include <string.h>
#include <stdlib.h>

#include "utils/utils.h"

#include "avulsion.h"
#include "avulsion_api.h"

#ifdef SWIG
% include avulsion_api.h
#endif


typedef struct
{
  double variance;
  double last_angle;
  double now;
  double time_step;
  double sed_flux;
  int total_river_mouths;
  double bed_load_exponent;

  double* angles;
  int len;

  Sed_cube p;

  double* elevation;
  double* discharge;
  int nx;
  int ny;
  double dx;
  double dy;

  GRand* rand;
  guint seed;
}
State;

Avulsion_state* _avulsion_alloc (void);
Avulsion_state * _avulsion_free (Avulsion_state * self);
int _avulsion_initialize (State* s);
int _avulsion_run_until (State* s, int until);
int _avulsion_finalize (State* s);

void avulsion_init_state (State* s);
void avulsion_free_state (State* s);


Avulsion_state*
_avulsion_alloc (void)
{
  State *s = (State*)malloc (sizeof (State));

  avulsion_init_state (s);

  return (Avulsion_state *) s;
}

Avulsion_state*
_avulsion_free (Avulsion_state * self)
{
  if (self)
  {
    avulsion_free_state ((State *) self);
    g_free (self);
  }
  return NULL;
}

Avulsion_state *
avulsion_init (Avulsion_state * self)
{
  if (!self)
    self = _avulsion_alloc ();

  _avulsion_initialize ((State *) self);

  return self;
}

int
avulsion_run_until (Avulsion_state * self, double time_in_days)
{
  State *p = (State *) self;
  int until_time_step = time_in_days / p->time_step;
  return _avulsion_run_until (p, until_time_step);
}

Avulsion_state *
avulsion_finalize (Avulsion_state * self, int free)
{
  _avulsion_finalize ((State *) self);

  if (free)
    self = _avulsion_free (self);

  return self;
}

Avulsion_state *
avulsion_set_variance (Avulsion_state * self, double variance)
{
  State *p = (State *) self;
  p->variance = variance;

  {
    Sed_riv r = sed_cube_borrow_nth_river (p->p, 0);
    Avulsion_st* data;

    data = avulsion_new (
             (p->seed==0)?g_rand_new():g_rand_new_with_seed(p->seed),
             variance);
    eh_require (data);
    sed_river_set_avulsion_data (r, data);
    //sed_river_avulsion_data (r)->std_dev = variance;
  }
  return self;
}

Avulsion_state *
avulsion_set_dx (Avulsion_state * self, double dx)
{
  State *p = (State *) self;
  p->dx = dx;
  return self;
}

Avulsion_state *
avulsion_set_dy (Avulsion_state * self, double dy)
{
  State *p = (State *) self;
  p->dy = dy;
  return self;
}

void
_avulsion_update_elevation (Avulsion_state* self)
{
  State* s = (State*) self;

  {
    int i;
    const int len = sed_cube_size (s->p);
    for (i=0; i<len; i++)
    {
      s->elevation[i] = sed_cube_elevation (s->p, 0, i);
//eh_watch_dbl (s->elevation[i]);
    }
  }

  return;
}

Avulsion_state*
avulsion_set_grid (Avulsion_state* self, gint shape[2], double res[2])
{
  State* s = (State*)self;

  if (!s->p)
  {
    Sed_riv r = sed_river_new ("AvulsionRiver1");

    { /* Create sediment */
      Sed_sediment s = NULL;
      GError* error = NULL;
      gchar* buffer = sed_sediment_default_text ();

      s = sed_sediment_scan_text (buffer, &error);
      g_free (buffer);

      eh_print_on_error (error, "%s: Error creating sediment.", "Avulsion");
      sed_sediment_set_env (s);
    }

    s->p = sed_cube_new (shape[1], shape[0]);
    sed_cube_set_z_res (s->p, 1.);
    sed_cube_set_y_res (s->p, res[0]);
    sed_cube_set_x_res (s->p, res[1]);

    sed_cube_add_trunk (s->p, r);

    s->dy = res[0];
    s->dx = res[1];
    s->nx = sed_cube_n_x (s->p);
    s->ny = sed_cube_n_y (s->p);
    s->elevation = g_new (double, sed_cube_size (s->p));
    s->discharge = g_new (double, sed_cube_size (s->p));

    _avulsion_update_elevation (self);
  }
  return self;
}

Avulsion_state*
avulsion_set_river_hinge (Avulsion_state* self, gint ind[2])
{
  State* s = (State*)self;
  Sed_riv r = sed_cube_borrow_nth_river (s->p, 0);
  sed_river_set_hinge (r, ind[1], ind[0]);
  return self;
}

Avulsion_state*
avulsion_set_river_angle_limit (Avulsion_state* self, double limit[2])
{
  State* s = (State*)self;
  Sed_riv r = sed_cube_borrow_nth_river (s->p, 0);
  sed_river_set_angle_limit (r, limit[0], limit[1]);
  sed_river_set_angle (r, .5*(limit[0]+limit[1]));
  return self;
}

Avulsion_state*
avulsion_set_river_hydro (Avulsion_state* self, Sed_hydro hydro)
{
  State* s = (State*)self;
  Sed_riv r = sed_cube_borrow_nth_river (s->p, 0);
  sed_river_set_hydro (r , hydro);
  return self;
}

Avulsion_state*
avulsion_set_elevation_from_file (Avulsion_state* self, gchar* file)
{
  State *p = (State *) self;

  eh_require (p);

  {
    gint n_rows = 0;
    gint n_cols = 0;
    GError* error = NULL;
    double** elevation;

    elevation = eh_dlm_read (file, ";,", &n_rows, &n_cols, &error);
    p->elevation = elevation[0];
    g_free (elevation);

    p->nx = n_rows;
    p->ny = n_cols;

    p->discharge = g_new (double, p->nx*p->ny);

    {
      Sed_riv r = sed_river_new ("AvulsionRiver1");
      p->p = sed_cube_new (p->nx, p->ny);
      sed_cube_set_bathy_data (p->p, p->elevation);
      sed_cube_add_trunk (p->p, r);
    }

    if (error)
      self = NULL;
  }

  return self;
}

Avulsion_state*
avulsion_set_sed_flux (Avulsion_state* self, double flux)
{
  State *p = (State *) self;
  p->sed_flux = flux;
  return self;
}

Avulsion_state*
avulsion_set_bed_load_exponent (Avulsion_state* self, double exponent)
{
  State *p = (State *) self;
  p->bed_load_exponent = exponent;
  return self;
}

Avulsion_state*
avulsion_set_total_river_mouths (Avulsion_state* self, int n_branches)
{
  State *p = (State *) self;
  p->total_river_mouths = n_branches;
  return self;
}

Avulsion_state*
avulsion_set_elevation (Avulsion_state* self, double* val)
{
  State *p = (State *) self;

  eh_require (p);
  eh_require (val);

  {
    const int len = sed_cube_size (p->p);
    int i;

    for (i=0; i<len; i++)
      p->elevation[i] = val[i];

    sed_cube_set_bathy_data (p->p, val);
  }

  return self;
}

double
avulsion_get_variance (Avulsion_state * self)
{
  State *p = (State *) self;
  return p->variance;
}

double
avulsion_get_current_time (Avulsion_state* self)
{
  State *p = (State *) self;
  return p->now;
}

double
avulsion_get_end_time (Avulsion_state* self)
{
  return G_MAXDOUBLE;
}

double
avulsion_get_start_time (Avulsion_state* self)
{
  return 0.;
}

int
avulsion_get_nx (Avulsion_state* self)
{
  State *p = (State *) self;
  return p->nx;
}

int
avulsion_get_ny (Avulsion_state* self)
{
  State *p = (State *) self;
  return p->ny;
}

int*
avulsion_get_value_dimen (Avulsion_state* self, const gchar* val_s, int shape[3])
{
  shape[0] = avulsion_get_ny (self);
  shape[1] = avulsion_get_nx (self);
  shape[2] = 1;

  return shape;
}

double
avulsion_get_dx (Avulsion_state* self)
{
  State *p = (State *) self;
  return p->dx;
}

double
avulsion_get_dy (Avulsion_state* self)
{
  State *p = (State *) self;
  return p->dy;
}

double*
avulsion_get_value_res (Avulsion_state* self, const gchar* val_s, double res[3])
{
  res[0] = avulsion_get_dy (self);
  res[1] = avulsion_get_dx (self);
  res[2] = 1;

  return res;
}

double
avulsion_get_angle (Avulsion_state* self)
{
  State *s = (State *) self;
  return s->angles[s->len-1];
}

double*
avulsion_get_value (Avulsion_state* self, const gchar* val_string,
                    gint dimen[3])
{
  State *p = (State *) self;
  double* vals = NULL;
  
  {
    double* src = NULL;
    const int len = avulsion_get_nx (self)*avulsion_get_ny (self);

    if (g_ascii_strcasecmp (val_string, "elevation")==0)
      src = p->elevation;
    else if (g_ascii_strcasecmp (val_string, "discharge")==0 ||
             g_ascii_strcasecmp (val_string, "SedimentFlux")==0)
      src = p->discharge;

    if (src)
    {
      vals = (double*)g_memdup (src, sizeof (double)*len);

      dimen[0] = 1;
      dimen[1] = avulsion_get_nx (self);
      dimen[2] = avulsion_get_ny (self);
    }
    else
    {
      dimen[0] = 0;
      dimen[1] = 0;
      dimen[2] = 0;
    }
  }

  return vals;
}

double*
avulsion_get_value_data (Avulsion_state* self, const gchar* val_string,
                         gint lower[3], gint upper[3], gint stride[3])
{
  State* s = (State*)self;
  double* val = NULL;
  gint dimen[3];

  val = avulsion_get_value (self, val_string, dimen);

  lower[0] = 0;
  lower[1] = 0;
  upper[0] = sed_cube_n_y (s->p)-1;
  upper[1] = sed_cube_n_x (s->p)-1;
  stride[0] = 1;
  stride[1] = sed_cube_n_y (s->p);

  return val;
}

int
_avulsion_initialize (State* s)
{
  if (s)
    return TRUE;
  else
    return FALSE;
}


double*
avulsion_branch_load_fraction (State* s, Sed_riv* leaves, int* len)
{
  double* f = NULL;
  
  *len = 0;

  { /* Calculate fraction to each branch */
    const int n_leaves = g_strv_length ((gchar**)leaves);
    double* lengths = g_new (double, n_leaves);

    f = g_new (double, n_leaves);

    { /* Calculate leaf lengths, and total length */
      int n;
      Eh_ind_2 hinge;
      Eh_ind_2 mouth;
      const double dx = sed_cube_x_res (s->p);
      const double dy = sed_cube_y_res (s->p);
      double di, dj;
      Sed_riv* leaf;

      for (leaf=leaves, n=0; *leaf; leaf++, n++)
      {
        hinge = sed_river_hinge (*leaf);
        mouth = sed_river_mouth (*leaf);
        di = (hinge.i - mouth.i)*dy;
        dj = (hinge.j - mouth.j)*dx;

        lengths[n] = sqrt (di*di + dj*dj);
      }
    }
    
    { /* Calculate fraction to each branch */
      int n;
      double total = 0;
      const double a = s->bed_load_exponent;
      double total_length = 0;

      for (n=0; n<n_leaves; n++)
        total_length += lengths[n];

      for (n=0; n<n_leaves; n++)
      {
        f[n] = powf ((total_length/lengths[n]), a);
        total += f[n];
      }
      for (n=0; n<n_leaves; n++)
        f[n] /= total;
    }

    g_free (lengths);

    *len = n_leaves;
  }

  return f;
}

void
_avulsion_reset_discharge (State* s)
{
  State* p = (State*)s;

  {
    int i;
    const int len = sed_cube_size (p->p);
    for (i=0; i<len; i++)
      s->discharge[i] = 0.;
  }

  return;
}

void
_avulsion_update_discharge (State* s)
{
  State* p = (State*)s;
  const Sed_riv r = sed_cube_borrow_nth_river (s->p, 0);
  gint* path;
  gint* id;
  Sed_riv* leaves;
  int n_leaves;
  int n;
  double* f;

  leaves = sed_river_leaves (r);

  f = avulsion_branch_load_fraction (s, leaves, &n_leaves);

  for (n=0; n<n_leaves; n++)
  {
  //path = sed_cube_river_path_id (s->p, r, TRUE);
    path = sed_cube_river_path_id (s->p, leaves[n], FALSE);

//  fprintf (stderr, "Avulsion:\n");
//  fprintf (stderr, "  angle = %f\n", s->angles[s->len-1]);
/*
  for (id=path; *id>=0; id++)
  {
    //s->discharge[*id] = p->sed_flux;
//    fprintf (stderr, "  river index = %d\n", *id);
//    eh_watch_int (*id);
//    eh_watch_dbl (p->elevation[*id]);
//    eh_watch_dbl (sed_cube_elevation (p->p, 0, *id));
  }
*/

    s->discharge[*path] += p->sed_flux*f[n];
//eh_watch_dbl (f[n]);
  }

  eh_free (leaves);
  eh_free (f);

//  eh_watch_int (*path);
//  eh_watch_dbl (s->discharge[*path]);

  return;
}

int
_avulsion_run_until (State* s, int until)
{
  int status = FALSE;

  g_assert (s);
  if (s)
  {
    const gint len = until - s->now;

    if (len>0)
    {
      gint i;
      const Sed_riv r = sed_cube_borrow_nth_river (s->p, 0);
      double f = 10e6;
      double area = sed_cube_area_above (s->p, sed_cube_sea_level(s->p));  
      gint n_branches = sed_cube_n_branches (s->p);

      if (s->angles)
        s->angles = g_renew (double, s->angles, len);
      else
        s->angles = g_new (double, len);

      _avulsion_reset_discharge (s);

      s->len = len;
      for (i=0; i<len; i++)
      {
        if (FALSE)
        {
          while ( area/n_branches > f )
          {
            sed_cube_split_river (s->p, sed_river_name_loc(r));
            sed_river_impart_avulsion_data(r);
            n_branches += 2;
          }
        }

        if ((n_branches+1)/2<s->total_river_mouths)
        {
          sed_cube_split_river (s->p, sed_river_name_loc(r));
          sed_river_impart_avulsion_data(r);
          n_branches += 2;
        }

        sed_cube_avulse_river (s->p , r);
        s->angles[i] = sed_river_angle (r);
        s->last_angle = s->angles[i];

        _avulsion_update_discharge (s);

        //s->angles[i] = avulsion (s->rand, s->last_angle, s->variance);
      }
      s->now = until;
      status = TRUE;
    }
    else
      status = FALSE;

    _avulsion_update_discharge (s);
  }
  else
    status = FALSE;

  return status;
}

int
_avulsion_finalize (State* s)
{
  return TRUE;
}

#define DEFAULT_VARIANCE (1.)
#define DEFAULT_LAST_ANGLE (0.)
#define DEFAULT_SEED (1945)

void
avulsion_init_state (State* s)
{
  g_assert (s);

  if (s)
  {
    s->variance = DEFAULT_VARIANCE;
    s->last_angle = DEFAULT_LAST_ANGLE;
    s->sed_flux = 0;
    s->total_river_mouths = 1;
    s->bed_load_exponent = 5.;

    s->now = 0;
    s->time_step = 1.;

    s->seed = DEFAULT_SEED;
    s->rand = g_rand_new_with_seed (s->seed);

    s->p = NULL;

    s->elevation = NULL;
    s->discharge = NULL;
    s->nx = 0;
    s->ny = 0;
    s->dx = 1.;
    s->dy = 1.;

    s->angles = NULL;
    s->len = 0;
  }

  return;
}

void
avulsion_free_state (State* s)
{
  if (s)
  {
    g_rand_free (s->rand);
    s->rand = NULL;

    if (s->discharge)
    {
      g_free (s->discharge);
    }
    s->discharge = NULL;
    if (s->elevation)
    {
      g_free (s->elevation);
    }
    s->elevation = NULL;
    s->nx = 0;
    s->ny = 0;

    g_free (s->angles);
    s->angles = NULL;
    s->len = 0;
  }
  return;
}

