#include <string.h>
#include <stdlib.h>
#include <math.h>

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
  double init_discharge;
  int total_river_mouths;
  double bed_load_exponent;
  double discharge_exponent;

  double* qb;
  double* q;
  double* mouth_x;
  double* mouth_y;
  double* mouth_qb;

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
avulsion_set_sed_flux (Avulsion_state* self, const double flux)
{
  State *p = (State *) self;
  p->sed_flux = flux;
  return self;
}

Avulsion_state*
avulsion_set_discharge (Avulsion_state* self, const double q)
{
  State *p = (State *) self;
  p->init_discharge = q;
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
avulsion_set_discharge_exponent (Avulsion_state* self, double exponent)
{
  State *p = (State *) self;
  p->discharge_exponent = exponent;
  return self;
}

Avulsion_state*
avulsion_set_total_river_mouths (Avulsion_state* self, int n_branches)
{
  State *p = (State *) self;
  if (p->total_river_mouths != n_branches)
  {
    int i;

    p->total_river_mouths = n_branches;
    p->qb = g_renew (double, p->qb, n_branches);
    p->q = g_renew (double, p->q, n_branches);
    p->mouth_x = g_renew (double, p->mouth_x, n_branches);
    p->mouth_y = g_renew (double, p->mouth_y, n_branches);
    p->mouth_qb = g_renew (double, p->mouth_qb, n_branches);

    for (i=0; i<n_branches; i++)
    {
      p->qb[i] = 0.;
      p->q[i] = 0.;
      p->mouth_x[i] = 0.;
      p->mouth_y[i] = 0.;
      p->mouth_qb[i] = 0.;
    }
  }
  return self;
}

Avulsion_state*
avulsion_set_elevation (Avulsion_state* self, double* val)
{
  State *p = (State *) self;

  eh_require (p);
  eh_require (val);
  eh_require (p->p);

  {
    const int size = sed_cube_size (p->p);
    int id;
/*
    eh_watch_int (size);
    eh_watch_int (p->nx);
    eh_watch_int (p->ny);
    eh_watch_ptr (p->elevation);

    eh_message ("Init elevation");
    for (i=0; i<len; i++)
      p->elevation[i] = 0.;

    eh_message ("Init val");
    for (i=0; i<len; i++)
      val[i] = 0;
*/
    for (id=0; id<size; id++)
      p->elevation[id] = val[id];

    eh_message ("set bathymetry");
    sed_cube_set_bathy_data (p->p, p->elevation);
    eh_message ("done");
  }

  return self;
}

Avulsion_state*
avulsion_set_depth (Avulsion_state* self, double* val)
{
  State *p = (State *) self;

  {
    const int size = sed_cube_size (p->p);
    int id;

    for (id=0; id<size; id++)
      p->elevation[id] = -1.*val[id];

    sed_cube_set_bathy_data (p->p, p->elevation);
  }
  return self;
}

Avulsion_state*
avulsion_set_value (Avulsion_state* self, const gchar* val_s, double* data,
                    gint lower[3], gint upper[3], gint stride[3])
{
  State *p = (State *) self;

  {
    const int size = sed_cube_size (p->p);
    const int dimen[3] = {avulsion_get_nx (self), avulsion_get_ny (self), 0};
    const int len[3] = {upper[0]-lower[0], upper[1]-lower[1],
                        upper[2]-lower[2]};
    int i, j, k;
    int id;
    int id_0;
    double scale = 1.;

    if (g_ascii_strcasecmp (val_s, "DEPTH")==0)
      scale = -1.;

    for (i=0, k=0; i<dimen[0]; i++)
    {
      id_0 = (i-lower[0])*stride[0];
      for (j=0; j<dimen[1]; j++, k++)
      {
        id = id_0 + (j-lower[1])*stride[1];
        p->elevation[k] = scale*data[id];
        //p->elevation[i][j] = data[id];
      }
    }

    sed_cube_set_bathy_data (p->p, p->elevation);
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
  if (g_ascii_strcasecmp (val_s,
        "mean_bed_load_flux_from_river")==0 ||
      g_ascii_strcasecmp (val_s,
        "mean_water_discharge_from_river")==0 ||
      g_ascii_strcasecmp (val_s, "river_mouth_x_position")==0 ||
      g_ascii_strcasecmp (val_s, "river_mouth_y_position")==0)
  {
    State *p = (State *) self;
    const int len = p->total_river_mouths;

    shape[0] = len;
    shape[1] = 1;
    shape[2] = 1;
  }
  else
  {
    shape[0] = avulsion_get_ny (self);
    shape[1] = avulsion_get_nx (self);
    shape[2] = 1;
  }

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

const double*
avulsion_get_value (Avulsion_state* self, const gchar* val_string,
                    gint dimen[3])
{
  State *p = (State *) self;
  double* vals = NULL;
  
  {
    double* src = NULL;

    if (g_ascii_strcasecmp (val_string,
                            "mean_bed_load_flux_from_river")==0)
      src = p->mouth_qb;
    else if (g_ascii_strcasecmp (val_string,
                                "mean_water_discharge_from_river")==0)
      src = p->q;
    else if (g_ascii_strcasecmp (val_string, "elevation")==0)
      src = p->elevation;
    else if (g_ascii_strcasecmp (val_string, "discharge")==0 ||
             g_ascii_strcasecmp (val_string, "SedimentFlux")==0)
      src = p->discharge;
    else if (g_ascii_strcasecmp (val_string, "river_mouth_x_position")==0)
      src = p->mouth_x;
    else if (g_ascii_strcasecmp (val_string, "river_mouth_y_position")==0)
      src = p->mouth_y;

    if (src)
    {
      dimen[0] = 1;
      if (g_ascii_strcasecmp (val_string,
                              "mean_bed_load_flux_from_river")==0 ||
          g_ascii_strcasecmp (val_string,
                              "mean_water_discharge_from_river")==0 ||
          g_ascii_strcasecmp (val_string, "river_mouth_x_position")==0 ||
          g_ascii_strcasecmp (val_string, "river_mouth_y_position")==0)
      {
        //const int len = avulsion_get_nx (self)*avulsion_get_ny (self);
        const int len = p->total_river_mouths;
        //vals = (double*)g_memdup (src, sizeof (double)*len);
        vals = src;

        dimen[0] = len;
        dimen[1] = 1;
        dimen[2] = 1;
      }
      else
      {
        const int len = avulsion_get_nx (self)*avulsion_get_ny (self);
        //vals = (double*)g_memdup (src, sizeof (double)*len);
        vals = src;

        dimen[1] = avulsion_get_nx (self);
        dimen[2] = avulsion_get_ny (self);
      }
    }
    else
    {
      dimen[0] = 0;
      dimen[1] = 0;
      dimen[2] = 0;
    }
  }

  return (const double*)vals;
}

const double*
avulsion_get_value_data (Avulsion_state* self, const gchar* val_string,
                         gint lower[3], gint upper[3], gint stride[3])
{
  State* s = (State*)self;
  double* val = NULL;
  gint dimen[3];

  val = (double*)avulsion_get_value (self, val_string, dimen);

  if (g_ascii_strcasecmp (val_string,
                          "mean_bed_load_flux_from_river")==0 ||
      g_ascii_strcasecmp (val_string,
                          "mean_water_discharge_from_river")==0 ||
      g_ascii_strcasecmp (val_string, "river_mouth_x_position")==0 ||
      g_ascii_strcasecmp (val_string, "river_mouth_y_position")==0)
  {
    lower[0] = 0;
    upper[0] = s->total_river_mouths-1;
    stride[0] = 1;

    lower[1] = 0;
    upper[1] = 0;
    stride[1] = 0;
  }
  else
  {
    lower[0] = 0;
    lower[1] = 0;
    upper[0] = sed_cube_n_y (s->p)-1;
    upper[1] = sed_cube_n_x (s->p)-1;
    stride[0] = 1;
    stride[1] = sed_cube_n_y (s->p);
  }

  return (const double*)val;
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
_split_discharge (double* slope, const int len, const double n,
                  const double q_total, double* mem)
{
  double* q = NULL;

  {
    int i;
    double normalize;

    if (mem)
      q = mem;
    else
      q = g_new (double, len );

    for (i=0, normalize=0.; i<len; i++)
      normalize += pow (slope[i], n);
    normalize = q_total / normalize;

    for (i=0; i<len; i++)
      q[i] = pow (slope[i], n) * normalize;
  }

  return q;
}

double*
_split_bed_load (double* slope, double* q, const int len, const double m,
                 const double qb_total, double* mem)
{
  double* qb = NULL;

  {
    int i;
    double total = 0;

    if (mem)
      qb = mem;
    else
      qb = g_new (double, len);
//fprintf (stderr, "Split bed load\n");
//fprintf (stderr, "len=%d\n", len);
//fprintf (stderr, "q=%f\n", q[0]);
    for (i=0; i<len; i++)
      qb[i] = pow (q[i]*slope[i], m);
//fprintf (stderr, "qb=%f\n", qb[0]);

    for (i=0; i<len; i++)
      total += qb[i];
//fprintf (stderr, "total=%f\n", total);

    total = qb_total / total;
    for (i=0; i<len; i++)
      qb[i] *= total;
//fprintf (stderr, "qb=%f\n", qb[0]);
  }

  return qb;
}

double*
_avulsion_branch_length (State* s, int* len)
{
  double* l = NULL;

  *len = 0;

  { /* Calculate leaf lengths, and total length */
    int n;
    Eh_ind_2 hinge;
    Eh_ind_2 mouth;
    const double dx = sed_cube_x_res (s->p);
    const double dy = sed_cube_y_res (s->p);
    double di, dj;
    const Sed_riv r = sed_cube_borrow_nth_river (s->p, 0);
    Sed_riv* leaves = sed_river_leaves (r);
    Sed_riv* leaf;
    const int n_leaves = g_strv_length ((gchar**)leaves);

    l = g_new (double, n_leaves);

    for (leaf=leaves, n=0; *leaf; leaf++, n++)
    {
      hinge = sed_river_hinge (*leaf);
      mouth = sed_river_mouth (*leaf);
      di = (hinge.i - mouth.i)*dy;
      dj = (hinge.j - mouth.j)*dx;

      l[n] = sqrt (di*di + dj*dj);
    }

    *len = n_leaves;

    eh_free (leaves);
  }

  return l;
}
    
/** The discharge to each branch
*/
double*
_avulsion_branch_discharge (State* s, int* len)
{
  double* q = NULL;

  *len = 0;

  {
    int i;
    const Sed_riv r = sed_cube_borrow_nth_river (s->p, 0);
    const Sed_riv* leaves = sed_river_leaves (r);
    //const double q_total = sed_river_water_flux (r);
    //const double q_total = 10000;
    const double q_total = s->init_discharge;
    const double n = s->discharge_exponent;
    const int n_leaves = g_strv_length ((gchar**)leaves);
    double* length = NULL;
    double* slope = NULL;

    slope = g_new (double, n_leaves);

    length = _avulsion_branch_length (s, len);
    eh_require (length);
    eh_require (*len==n_leaves);

    for (i=0; i<n_leaves; i++)
      slope[i] = 1./length[i];

    q = _split_discharge (slope, n_leaves, n, q_total, s->q);

    g_free  (length);
    g_free  (slope);

    eh_free (leaves);

    *len = n_leaves;
  }

  return q;
}

/** The bed load flux to each branch
*/
double*
_avulsion_branch_bed_load (State* s, int* len)
{
  double* qb = NULL;

  *len = 0;

  {
    int i;
    const Sed_riv r = sed_cube_borrow_nth_river (s->p, 0);
    //const double qb_total = sed_river_bedload (r);
    const double qb_total = s->sed_flux;
    const Sed_riv* leaves = sed_river_leaves (r);
    const double m = s->bed_load_exponent;
    const int n_leaves = g_strv_length ((gchar**)leaves);
    double* length = NULL;
    double* slope = NULL;
    //double* q = NULL;

    slope = g_new (double, n_leaves);

    length = _avulsion_branch_length (s, len);

    for (i=0; i<n_leaves; i++)
      slope[i] = 1./length[i];

    //q = _avulsion_branch_discharge (s, len);
    _avulsion_branch_discharge (s, len);

    eh_require (*len==n_leaves);
    qb = _split_bed_load (slope, s->q, n_leaves, m, qb_total, s->qb);

    { /* Print leaf info */
      double* angle = g_new (double, n_leaves);

      for (i=0; i<n_leaves; i++)
      {
        angle[i] = sed_river_angle (leaves[i]);
      }
/*
      for (i=0; i<n_leaves; i++)
      {
        fprintf (stderr, "Avulsion: %d: %f: %f: %f: %f: %f: %f: %f\n",
                          i, length[i], s->q[i], s->qb[i], angle[i],
                          s->mouth_x[i], s->mouth_y[i], s->mouth_qb[i]);
      }
*/

      g_free (angle);
    }

    //g_free (q);
    g_free (length);
    g_free (slope);

    eh_free (leaves);
  }

  return qb;
}

#if 0
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
        f[n] = pow ((total_length/lengths[n]), a);
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
#endif

void
_avulsion_reset_discharge (State* s)
{
  State* p = (State*)s;

  {
    int i;

    { /* Reset discharge grid */
      const int len = sed_cube_size (p->p);
      for (i=0; i<len; i++)
        s->discharge[i] = 0.;
    }

    { /* Reset qb for each river mouth */
      const int len = p->total_river_mouths;
      for (i=0; i<len; i++)
        s->mouth_qb[i] = 0.;
    }
  }

  return;
}

void
_avulsion_update_discharge (State* s, const double f)
{
  State* p = (State*)s;
  const Sed_riv r = sed_cube_borrow_nth_river (s->p, 0);
  gint* path;
  gint* id;
  Sed_riv* leaves;
  int n_leaves;
  int n;
  Eh_ind_2 mouth_ind;
  //double* qb = NULL;
  const double dx = sed_cube_x_res (s->p);
  const double dy = sed_cube_y_res (s->p);
  double angle;

  leaves = sed_river_leaves (r);

  _avulsion_branch_bed_load (s, &n_leaves);

  //f = avulsion_branch_load_fraction (s, leaves, &n_leaves);

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

    //s->discharge[*path] += p->sed_flux*f[n];

    s->discharge[*path] += s->qb[n]*f;

    mouth_ind = sed_river_mouth (leaves[n]);

    s->mouth_x[n] = mouth_ind.i*dx;
    s->mouth_y[n] = mouth_ind.j*dy;
    s->mouth_qb[n] += s->qb[n]*f;

    angle = sed_river_angle (leaves[n]);
    fprintf (stderr, "Avulsion: %d: %f: %f: %f: %f: %f: %f\n",
                     n, s->q[n], s->qb[n], angle,
                     s->mouth_x[n], s->mouth_y[n], s->mouth_qb[n]);

  }

  //g_free (qb);

  eh_free (leaves);
  //eh_free (f);

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
      const double dt_frac = 1./(double)len;

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

        _avulsion_update_discharge (s, dt_frac);

        //s->angles[i] = avulsion (s->rand, s->last_angle, s->variance);
      }
      s->now = until;
      status = TRUE;
    }
    else
      status = FALSE;

    //_avulsion_update_discharge (s);
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

    s->qb = g_new (double, s->total_river_mouths);
    s->q = g_new (double, s->total_river_mouths);
    s->mouth_x = g_new (double, s->total_river_mouths);
    s->mouth_y = g_new (double, s->total_river_mouths);
    s->mouth_qb = g_new (double, s->total_river_mouths);

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

    g_free (s->q);
    s->q = NULL;

    g_free (s->qb);
    s->qb = NULL;

    g_free (s->mouth_x);
    s->mouth_x = NULL;

    g_free (s->mouth_y);
    s->mouth_y = NULL;

    g_free (s->mouth_qb);
    s->mouth_qb = NULL;

    s->total_river_mouths = 0;
  }
  return;
}

