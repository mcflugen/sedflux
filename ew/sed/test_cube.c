#include <glib.h>
#include <sed_sedflux.h>
#include <sed_cube.h>

#include "test_sed.h"

Sed_cube
new_test_cube ()
{
  const gint mult = g_test_quick ()?1:10;
  Sed_cube p = NULL;

  {
    const gint nx = g_test_rand_int_range (10*mult,20*mult);
    const gint ny = g_test_rand_int_range (10*mult,20*mult);

    p = sed_cube_new (nx, ny);
  }

  return p;
}
Sed_cube
new_land_ocean_cube (double i0, double i1, double j0, double j1)
{
  Sed_cube p = NULL;

  eh_require (i1>i0);
  eh_require (j1>j0);

  {
    p = new_test_cube ();

    {
      const gint nx = sed_cube_n_x (p);
      const gint ny = sed_cube_n_y (p);
      const gint i_start = nx*i0;
      const gint i_end = nx*i1;
      const gint j_start = ny*j0;
      const gint j_end = ny*j1;
      gint i, j;

      for (i=0; i<nx; i++)
        for (j=0; j<ny; j++)
          if (i>=i_start && i<i_end && j>=j_start && j<j_end)
            sed_cube_set_base_height (p, i, j, 1);
          else
            sed_cube_set_base_height (p, i, j, -1);
    }
  }

  return p;
}

void
test_sed_cube_new (void)
{
  {
    Sed_cube c = sed_cube_new( 2 , 5 );

    g_assert (c!=NULL);

    g_assert_cmpint (sed_cube_n_x (c), ==, 2);
    g_assert_cmpint (sed_cube_n_y (c), ==, 5);
    g_assert ( eh_compare_dbl (sed_cube_mass (c), 0., 1e-12) );

    sed_cube_destroy( c );
  }
}

void
test_sed_cube_destroy (void)
{
   Sed_cube c = sed_cube_new( 5 , 2 );

   c = sed_cube_destroy(c);

   g_assert (c==NULL);
}

gchar* test_seqfile[] =
{
   " # Begin the first record\n" ,
   "[ TiMe: 1 ] /* Label is case insensitive*/\n" ,
   "0, -1\n" ,
   "10, 1\n" ,
   "/* The second record.\n" ,
   "*/\n" ,
   "[ time : 3 ]\n" ,
   "0, -1\n" ,
   "5, -2\n" ,
   "9, 1// file ending without an empty line" ,
   NULL
};

void
test_sequence_2 (void)
{
  GError* error = NULL;
  gchar* name_used = NULL;
  gchar* tmpdir = NULL;
  int fd = -1;
  Eh_sequence* s = NULL;
  gint n_y = 10;
  double* y = eh_linspace( 0 , 9 , n_y );

  //fd = g_file_open_tmp ("sed_cube_test_XXXXXX", &name_used, &error);
  tmpdir = g_build_filename (g_get_tmp_dir (), "XXXXXX", NULL);
  mkdtemp (tmpdir);

//  g_assert (fd!=-1);
//  eh_print_on_error (error, "test_sequence_2");
//  g_assert (error==NULL);

  {
    //FILE* fp = fdopen (fd, "w");
    FILE* fp;
    gchar** line;

    name_used = g_build_filename (tmpdir, "sed_cube_test", NULL);

    fp = eh_fopen_error (name_used, "w" ,&error);
    eh_print_on_error (error, "test_sequence_2");

    for (line=test_seqfile; *line; line++ )
      fprintf (fp, "%s", *line);

    fclose (fp);
  }

  {
    GError* err = NULL;

    s = sed_get_floor_sequence_2 (name_used, y, n_y, &err);

    eh_print_on_error (err, "test_sequence_2");
  }

  g_assert (s!=NULL);
  g_assert_cmpint (s->len, ==, 2);

  eh_free (name_used);
  eh_free (tmpdir);
  //close (fd);
}

void
test_cube_to_cell (void)
{
  const int nx = g_test_rand_int_range (25, 50);
  const int ny = g_test_rand_int_range (25, 50);
  Sed_cube p = sed_cube_new (nx ,ny);
  Sed_cell dest;
  double cube_mass, cell_mass;

  sed_cube_set_x_res (p, 1.);
  sed_cube_set_y_res (p, 1.);
  sed_cube_set_z_res (p, 1.);

  {
    Sed_cell c = sed_cell_new_env ();
    gint i;

    sed_cell_set_equal_fraction (c);
    sed_cell_resize (c, 1.);

    for ( i=0 ; i<sed_cube_size (p) ; i++ )
      sed_column_add_cell (sed_cube_col(p,i), c);

    sed_cell_destroy (c);
  }

  dest = sed_cube_to_cell (p, NULL);

  cell_mass = sed_cell_mass (dest);
  cube_mass = sed_cube_mass (p);

  g_assert (eh_compare_dbl(cube_mass,cell_mass,1e-12));

  sed_cube_destroy( p );
}

void
test_cube_erode (void)
{
  const int nx = g_test_rand_int_range (25, 50);
  const int ny = g_test_rand_int_range (25, 50);
  const int len = nx*ny;
  Sed_cube p = sed_cube_new (nx ,ny);
  double *dz;

  g_assert (p);

  { /* Set erosion rates */
    int i;
    dz = eh_new (double, nx*ny);
    for (i=0; i<len; i++)
      dz[i] = g_test_rand_double_range (0, 10);
  }

  { /* Fill the cube with sediment */
    Sed_cell c = sed_cell_new_env ();
    gint i;

    g_assert (c);

    sed_cell_set_equal_fraction (c);

    for (i=0; i<len; i++)
    {
      sed_cell_resize (c, dz[i]*2.);
      sed_column_add_cell (sed_cube_col(p,i), c);
    }
  }

  { /* Erode the sediment */
    Sed_cube rtn;
    double mass_0 = sed_cube_mass (p);
    double mass_1;

    g_assert (mass_0>0);

    rtn = sed_cube_erode (p, dz);
    mass_1 = sed_cube_mass (p);

    g_assert (rtn==p);
    g_assert (eh_compare_dbl (mass_0, mass_1*2., 1e-12));
  }

  { /* NULL arguments */
    Sed_cube rtn;
    double mass_0 = sed_cube_mass (p);
    double mass_1;

    g_assert (mass_0>0);

    rtn = sed_cube_erode (p,NULL);
    mass_1 = sed_cube_mass (p);
    g_assert (eh_compare_dbl (mass_0, mass_1, 1e-12));
  }

  sed_cube_destroy (p);
}

void
test_cube_deposit (void)
{
  const int nx = g_test_rand_int_range (25, 50);
  const int ny = g_test_rand_int_range (25, 50);
  const int len = nx*ny;
  Sed_cube p = sed_cube_new (nx ,ny);
  Sed_cell *dz;

  g_assert (p);

  { /* Set deposit rates */
    int i;
    dz = eh_new (Sed_cell, nx*ny);
    for (i=0; i<len; i++)
    {
      dz[i] = sed_cell_new_env ();

      g_assert (dz[i]);
      sed_cell_set_equal_fraction (dz[i]);
      sed_cell_resize (dz[i], g_test_rand_double_range (0,10));
    }
  }

  { /* Fill the cube with sediment */
    Sed_cube rtn;
    double mass_0 = sed_cube_mass (p);
    double mass_1, mass_2;

    g_assert (eh_compare_dbl (mass_0, 0., 1e-12));

    rtn = sed_cube_deposit (p, dz);
    g_assert (rtn==p);

    mass_1 = sed_cube_mass (p);
    g_assert (mass_1>mass_0);

    sed_cube_deposit (p, dz);
    mass_2 = sed_cube_mass (p);
    g_assert (eh_compare_dbl (mass_2, 2.*mass_1, 1e-12));
  }

  { /* NULL arguments */
    Sed_cube rtn;
    double mass_0 = sed_cube_mass (p);
    double mass_1;

    g_assert (mass_0>0);

    rtn = sed_cube_deposit (p,NULL);
    mass_1 = sed_cube_mass (p);
    g_assert (eh_compare_dbl (mass_0, mass_1, 1e-12));
  }

  sed_cube_destroy (p);
}

void
test_cube_get_size (void)
{
  const int nx = g_test_rand_int_range (100, 200);
  const int ny = g_test_rand_int_range (100, 200);
  Sed_cube p = sed_cube_new (nx ,ny);

  g_assert (p);
  g_assert_cmpint (sed_cube_n_x (p), ==, nx);
  g_assert_cmpint (sed_cube_n_y (p), ==, ny);
  g_assert_cmpint (sed_cube_size (p), ==, nx*ny);

  p = sed_cube_destroy(p);

  g_assert (p==NULL);
}

void
test_cube_base_height (void)
{
  Sed_cube p = new_test_cube ();

  {
    int i, j;
    const int nx = sed_cube_n_x (p);
    const int ny = sed_cube_n_y (p);

    for (i=0; i<nx; i++)
      for (j=0; j<ny; j++)
        sed_cube_set_base_height (p, i, j, i*nx+j);

    for (i=0; i<nx; i++)
      for (j=0; j<ny; j++)
        g_assert (eh_compare_dbl (sed_cube_base_height(p, i, j),
                                  i*nx+j, 1e-12));
  }

  sed_cube_destroy (p);
}

void
test_cube_river_add (void)
{
  Sed_cube p = NULL;

  p = new_land_ocean_cube (0., 1.,  0., .25);
  g_assert (p);

  {
    const gint nx = sed_cube_n_x (p);
    const gint ny = sed_cube_n_y (p);
    const gint nland = ny*.25;
    Sed_riv r = sed_river_new ("Trunk 1");

    g_assert (r);

    { /* Find the river mouth of a river */
      Eh_ind_2 mouth;
  
      sed_river_set_hinge (r, nx/2, 0);
      sed_river_set_angle (r, G_PI*.5);
      sed_cube_find_river_mouth (p, r);
  
      mouth = sed_river_mouth (r);
  
      g_assert_cmpint (mouth.i, ==, nx/2);
      g_assert_cmpint (mouth.j, ==, nland);
  
      g_assert_cmpfloat (sed_cube_base_height(p, mouth.i, mouth.j), <, 0);
      g_assert_cmpfloat (sed_cube_base_height(p, mouth.i, mouth.j-1), >, 0);
    }
  
    { /* Add the river to a Sed_cube */
      Sed_riv* r_list = NULL;
      gpointer rtn = NULL;
  
      r_list = sed_cube_all_trunks (p);
      g_assert (r_list==NULL);
  
      rtn = sed_cube_add_trunk (p, r);
      g_assert (rtn);
  
      r_list = sed_cube_all_trunks (p);
      g_assert (r_list);
      g_assert_cmpint (g_strv_length ((gchar**)r_list), ==, 1);
  
      g_assert (r_list[0]!=r);
      g_assert (r_list[0]==rtn);
    }
  
    { /* Add another river to the Sed_cube */
      Sed_riv* r_list = NULL;
      Sed_riv r2 = sed_river_new ("Trunk 2");
      gpointer r2_id;
  
      sed_river_set_hinge (r2, nx/2, 0);
      sed_river_set_angle (r2, G_PI*.5);
      sed_cube_find_river_mouth (p, r2);
  
      r2_id = sed_cube_add_trunk (p, r2);
  
      r_list = sed_cube_all_trunks (p);
      g_assert (r_list);
      g_assert_cmpint (g_strv_length ((gchar**)r_list), ==, 2);
  
      g_assert (r_list[0]!=r2);
      g_assert (r_list[1]!=r);
  
      g_assert (sed_cube_nth_river (p, 0)!=r_list[0]);
      g_assert (sed_cube_nth_river (p, 1)!=r_list[1]);
  
      r2 = sed_river_destroy (r2);
    }
  
    { /* Move the river around in the Sed_cube */
      Eh_ind_2 mouth;
      Sed_riv r0 = sed_cube_nth_river (p, 0);
  
      sed_river_set_hinge (r0, nx/2, nland-1);
      sed_cube_find_river_mouth (p, r0);
      mouth = sed_river_mouth (r0);
  
      g_assert_cmpint (mouth.i, ==, nx/2);
      g_assert_cmpint (mouth.j, ==, nland);
  
      sed_river_set_hinge (r0, 0, nland-1);
      mouth = sed_river_mouth (r0);
  
      g_assert_cmpint (mouth.i, ==, nx/2);
      g_assert_cmpint (mouth.j, ==, nland);
  
      sed_cube_find_river_mouth (p, r0);
      mouth = sed_river_mouth (r0);
  
      g_assert_cmpint (mouth.i, ==, 0);
      g_assert_cmpint (mouth.j, ==, nland);
  
      sed_river_destroy (r0);
    }

    r = sed_river_destroy (r);
  }

  sed_cube_destroy (p);
}

void
test_cube_add_river_mouth (void)
{
  Sed_cube p = NULL;

  p = new_land_ocean_cube (0., 1.,  0., .75);
  g_assert (p);

  {
    const gint nx = sed_cube_n_x (p);
    const gint ny = sed_cube_n_y (p);
    const double set_flux = g_test_rand_double ();
    gpointer r = NULL;

    { /* Set up a river and add it to a cube */
      Sed_hydro hydro = sed_hydro_new (1);
      gint id = sed_cube_id (p, nx/2, ny*3/4-1);

      g_assert (hydro);
      sed_hydro_set_bedload (hydro, set_flux);
      g_assert (eh_compare_dbl (sed_hydro_bedload (hydro), set_flux, 1e-12));

      r = sed_cube_add_river_mouth (p, id, hydro);
      g_assert (r!=NULL);

      sed_hydro_destroy (hydro);
    }

    { /* Test that the river was set up correctly */
      Eh_ind_2 ind;
      double angle;
      double get_flux;

      ind = sed_cube_river_mouth (p, r);
      g_assert_cmpint (ind.i, ==, nx*.5);
      g_assert_cmpint (ind.j, ==, ny*.75);

      angle = sed_cube_river_angle (p, r);
      g_assert (eh_compare_dbl (angle, M_PI/2., 1e-12));

      ind = sed_cube_river_hinge (p, r);
      g_assert_cmpint (ind.i, ==, nx*.5);
      g_assert_cmpint (ind.j, ==, ny*.75-1);

      get_flux = sed_cube_river_bedload (p, r);
      g_assert (eh_compare_dbl (get_flux, set_flux, 1e-12));
    }

  }

  sed_cube_destroy (p);

  return;
}

void
test_cube_add_river_mouth_grid (void)
{
  Sed_cube p = NULL;

  p = new_land_ocean_cube (0., .25,  0., 1.);
  g_assert (p);

  {
    const gint nx = sed_cube_n_x (p);
    const gint ny = sed_cube_n_y (p);
    double* set_flux = NULL;
    gpointer* rivers = NULL;
    gint n_rivers = 0;
    gint* shore = NULL;
    gint i;

    shore = sed_cube_shore_ids (p);
    g_assert (shore!=NULL);

    for (n_rivers=0; shore[n_rivers]>=0; n_rivers++);
    g_assert_cmpint (n_rivers, ==, ny);

    rivers = eh_new (gpointer, n_rivers+1);
    rivers[n_rivers] = NULL;

    set_flux = eh_new (double, n_rivers);
    for (i=0; i<n_rivers; i++)
      set_flux[i] = g_test_rand_double ();

    { /* Set up a rivers and add them to a cube */
      Sed_hydro hydro;
      gint i;

      for (i=0; i<n_rivers; i++)
      {
        hydro = sed_hydro_new (1);
        g_assert (hydro);

        sed_hydro_set_bedload (hydro, set_flux[i]);
        g_assert (eh_compare_dbl (sed_hydro_bedload (hydro),
                                  set_flux[i], 1e-12));

        rivers[i] = sed_cube_add_river_mouth (p, shore[i], hydro);
        g_assert (rivers[i]!=NULL);

        sed_hydro_destroy (hydro);
      }
    }

    { /* Test that the rivers were set up correctly */
      gint i;
      gpointer* river;
      Eh_ind_2 ind;
      Eh_ind_2 shore_ind;
      double angle;
      double get_flux;

      for (i=0; i<n_rivers; i++)
      {
        shore_ind = sed_cube_sub (p, shore[i]);

        ind = sed_cube_river_mouth (p, rivers[i]);
        g_assert_cmpint (ind.i, ==, shore_ind.i+1);
        g_assert_cmpint (ind.j, ==, shore_ind.j);

        angle = sed_cube_river_angle (p, rivers[i]);
        g_assert (eh_compare_dbl (angle, 0., 1e-12));

        ind = sed_cube_river_hinge (p, rivers[i]);
        g_assert_cmpint (ind.i, ==, shore_ind.i);
        g_assert_cmpint (ind.j, ==, shore_ind.j);

        get_flux = sed_cube_river_bedload (p, rivers[i]);
        g_assert (eh_compare_dbl (get_flux, set_flux[i], 1e-12));
      }
    }

    eh_free (set_flux);
    eh_free (rivers);
    eh_free (shore);
  }

  sed_cube_destroy (p);

  return;
}

void
test_is_land_ocean_cell ()
{
  gint nx;
  gint ny;
  gint nland;
  Sed_cube p = NULL;

  p = new_land_ocean_cube (0., 1., 0., .25);
  g_assert (p);

  nx = sed_cube_n_x (p);
  ny = sed_cube_n_y (p);
  nland = .25*ny;

  {
    gint i, j;
    const gint shore_j = nland-1;
    const gint coast_j = nland;

    for (i=0; i<nx; i++)
      for (j=0; j<ny; j++)
        if (j<shore_j)
          g_assert (is_land_cell (p, i, j));
        else if (j==shore_j)
          g_assert (is_land_cell (p, i, j) && is_shore_cell (p, i, j));
        else if (j==coast_j)
          g_assert (is_ocean_cell (p, i, j) && is_coast_cell (p, i, j));
        else
          g_assert (is_ocean_cell (p, i, j));

    for (i=0; i<nx; i++)
      for (j=0; j<ny; j++)
        if (j<shore_j)
          g_assert (!(is_shore_cell (p, i, j) || is_coast_cell (p, i, j) ||
                      is_ocean_cell (p, i, j)));
        else if (j==shore_j)
          g_assert (!(is_coast_cell (p, i, j) || is_ocean_cell (p, i, j)));
        else if (j==coast_j)
          g_assert (!(is_land_cell (p, i, j) || is_shore_cell (p, i, j)));
        else
          g_assert (!(is_land_cell (p, i, j) || is_shore_cell (p, i, j) ||
                      is_coast_cell (p, i, j)));
  }

  sed_cube_destroy (p);
}

void
test_is_boundary_cell ()
{
  Sed_cube p = NULL;
  gint nx;
  gint ny;

  p = new_test_cube ();
  g_assert (p);

  nx = sed_cube_n_x (p);
  ny = sed_cube_n_y (p);

  g_assert (is_boundary_cell_id (p, 0));
  g_assert (is_boundary_cell_id (p, ny-1));
  g_assert (is_boundary_cell_id (p, ny));
  g_assert (is_boundary_cell_id (p, nx*ny-1));
  g_assert (is_boundary_cell_id (p, (nx-1)*ny));
  g_assert (is_boundary_cell_id (p, (nx-1)*ny-1));

  g_assert (!is_boundary_cell_id (p, ny+1));

  // Don't check for overrun.
  g_assert (is_boundary_cell_id (p, nx*ny));
}

void
test_shore_mask ()
{
  Sed_cube p = NULL;

  p = new_land_ocean_cube (0., 1., 0., .25);
  g_assert (p);

  {
    gint id;
    const gint nx = sed_cube_n_x (p);
    const gint ny = sed_cube_n_y (p);
    const gint nland = ny*.25;
    const gint len = nx*ny;
    const gint shore_j = nland-1;
    gboolean* mask = NULL;

    mask = sed_cube_shore_mask (p);
    g_assert (mask!=NULL);

    for (id=0; id<shore_j; id++)
      g_assert (!mask[id]);

    for (id=shore_j; id<len; id++)
      if ((id-shore_j)%ny == 0)
        g_assert (mask[id]);
      else
        g_assert (!mask[id]);

    eh_free (mask);
  }

  sed_cube_destroy (p);
}

void
test_shore_ids ()
{
  Sed_cube p = NULL;

  p = new_land_ocean_cube (0., 1., 0., .25);
  g_assert (p);

  {
    gint i, id;
    const int nx = sed_cube_n_x (p);
    const int ny = sed_cube_n_y (p);
    const int nland = ny*.25;
    const gint len = nx*ny;
    const gint shore_j = nland-1;
    gint* ids = NULL;

    ids = sed_cube_shore_ids (p);
    g_assert (ids!=NULL);

    for (i=0; ids[i]>=0; i++)
      g_assert ((ids[i]-shore_j)%ny==0);
    g_assert_cmpint (i, ==, nx);

    eh_free (ids);
  }

  sed_cube_destroy (p);
}

void
test_cube_river_north (void)
{
  Sed_cube p = NULL;
  Sed_riv r = sed_river_new ("North");

  p = new_land_ocean_cube (0., .25, 0., 1.);
  g_assert (p);
  g_assert (r);

  { /* Find the river mouth of a river */
    Eh_ind_2 mouth;
    const gint nland = sed_cube_n_x (p)*.25;

    sed_river_set_hinge (r, 0, sed_cube_n_y (p)/2);
    sed_river_set_angle (r, 0);
    sed_cube_find_river_mouth (p, r);

    mouth = sed_river_mouth (r);

    g_assert_cmpint (mouth.i, ==, nland);
    g_assert_cmpint (mouth.j, ==, sed_cube_n_y (p)/2);

    g_assert_cmpfloat (sed_cube_base_height(p, mouth.i, mouth.j), <, 0);
    g_assert_cmpfloat (sed_cube_base_height(p, mouth.i-1, mouth.j), >, 0);
  }

  { /* Add the river to a Sed_cube */
    Sed_riv* r_list = NULL;

    r_list = sed_cube_all_trunks (p);
    g_assert (r_list==NULL);

    sed_cube_add_trunk (p, r);
    r_list = sed_cube_all_trunks (p);
    g_assert_cmpint (g_strv_length ((gchar**)r_list), ==, 1);

    g_assert (r_list[0]!=r);
  }

  r = sed_river_destroy (r);
  sed_cube_destroy (p);
}

void
test_cube_river_path_ray (void)
{
  Sed_cube p = NULL;
  Sed_riv r = sed_river_new ("North");

  p = new_land_ocean_cube (0., .25, 0., 1.);
  g_assert (p);
  g_assert (r);

  { /* Find the river mouth of a river */
    const int nx = sed_cube_n_x (p);
    const int ny = sed_cube_n_y (p);
    const int len = nx*ny;
    const int nland = nx*.25;
    Eh_ind_2 mouth;
    const gint hinge[2] = {0, ny/2};

    sed_cube_set_river_path_ray (r, p, hinge, 0.);

    mouth = sed_river_mouth (r);

    g_assert_cmpint (mouth.i, ==, nland);
    g_assert_cmpint (mouth.j, ==, ny/2);

    g_assert_cmpfloat (sed_cube_base_height(p, mouth.i, mouth.j), <, 0);
    g_assert_cmpfloat (sed_cube_base_height(p, mouth.i-1, mouth.j), >, 0);
  }

  r = sed_river_destroy (r);
  sed_cube_destroy (p);
}

void
test_cube_river_path_ends (void)
{
  Sed_cube p = NULL;
  Sed_riv r = sed_river_new ("North");

  p = new_land_ocean_cube (0., .25, 0., 1.);
  g_assert (p);
  g_assert (r);

  { /* Find the river mouth of a river */
    Eh_ind_2 mouth;
    double angle;
    const int nx = sed_cube_n_x (p);
    const int ny = sed_cube_n_y (p);
    const int nland = nx*.25;
    const gint start[2] = {0, ny/2};
    const gint end[2] = {nland, ny/2};

    sed_cube_set_river_path_ends (r, p, start, end);

    mouth = sed_river_mouth (r);
    angle = sed_river_angle (r);

    g_assert (eh_compare_dbl (angle, 0., 1e-12));
    g_assert_cmpint (mouth.i, ==, nland);
    g_assert_cmpint (mouth.j, ==, ny/2);

    g_assert_cmpfloat (sed_cube_base_height(p, mouth.i, mouth.j), <, 0);
    g_assert_cmpfloat (sed_cube_base_height(p, mouth.i-1, mouth.j), >, 0);
  }

  r = sed_river_destroy (r);
  sed_cube_destroy (p);
}

void
test_river_new (void)
{
  gchar *name = g_strdup ("Test River");
  Sed_riv r = sed_river_new (name);

  g_assert (r);

  g_assert (name!=sed_river_name (r));
  g_assert (sed_river_name_is (r,name));
  g_assert (sed_river_name_cmp (r,name)==0);
  g_assert_cmpstr (sed_river_name (r), ==, name);

  g_assert (sed_river_mouth_is (r, 0, 0));

  g_assert (sed_river_hydro (r)==NULL);
  g_assert (!sed_river_has_children (r));
  g_assert (sed_river_right(r)==NULL);
  g_assert (sed_river_left(r)==NULL);

  g_assert (eh_compare_dbl (sed_river_angle (r), 0., 1e-12));
  g_assert (eh_compare_dbl (sed_river_min_angle (r), -G_PI, 1e-12));
  g_assert (eh_compare_dbl (sed_river_max_angle (r), G_PI, 1e-12));

  r = sed_river_destroy (r);

  g_assert (r==NULL);
}

void
test_river_dup (void)
{
  gchar *name = g_strdup ("Test River Dup");
  Sed_riv r = sed_river_new (name);
  Sed_riv r_copy = NULL;
  double min_angle = g_test_rand_double_range (-G_PI, G_PI);
  double max_angle = g_test_rand_double_range (min_angle, G_PI);
  double angle = g_test_rand_double_range (min_angle, max_angle);
  gint hinge_i = g_test_rand_int ();
  gint hinge_j = g_test_rand_int ();

  g_assert (r);

  sed_river_set_angle (r, angle);
  sed_river_set_angle_limit (r, min_angle, max_angle);
  sed_river_set_hinge (r, hinge_i, hinge_j);

  r_copy = sed_river_dup (r);

  g_assert (r_copy);
  g_assert (r_copy!=r);

  g_assert (eh_compare_dbl (sed_river_angle (r_copy),
                            sed_river_angle (r), 1e-12));
  g_assert (eh_compare_dbl (sed_river_min_angle (r_copy),
                            sed_river_min_angle (r), 1e-12));
  g_assert (eh_compare_dbl (sed_river_max_angle (r_copy),
                            sed_river_max_angle (r), 1e-12));

  sed_river_destroy (r_copy);
  sed_river_destroy (r);
}

void
test_river_angle (void)
{
  gchar *name = g_strdup ("Test River Angle");
  Sed_riv r = sed_river_new (name);

  g_assert (eh_compare_dbl (sed_river_angle (r), 0., 1e-12));
  g_assert (eh_compare_dbl (sed_river_min_angle (r), -G_PI, 1e-12));
  g_assert (eh_compare_dbl (sed_river_max_angle (r), G_PI, 1e-12));

  sed_river_set_angle (r, G_PI*.5);
  g_assert (eh_compare_dbl (sed_river_angle (r), G_PI*.5, 1e-12));

  sed_river_set_angle (r, G_PI*1.5);
  g_assert (eh_compare_dbl (sed_river_angle (r), -G_PI*.5, 1e-12));

  sed_river_set_angle (r, -G_PI);
  g_assert (eh_compare_dbl (sed_river_angle (r), -G_PI, 1e-12));

  sed_river_set_angle (r, G_PI);
  g_assert (eh_compare_dbl (sed_river_angle (r), G_PI-1e-12, 1e-12));

  sed_river_set_angle_limit (r, G_PI*.25, G_PI*.75);
  g_assert (eh_compare_dbl (sed_river_min_angle (r), G_PI*.25, 1e-12));
  g_assert (eh_compare_dbl (sed_river_max_angle (r), G_PI*.75, 1e-12));
  g_assert (eh_compare_dbl (sed_river_angle (r), G_PI*.75, 1e-12));

  sed_river_set_angle (r, 0.);
  g_assert (eh_compare_dbl (sed_river_angle (r), G_PI*.25, 1e-12));

  sed_river_increment_angle (r, G_PI*.25);
  g_assert (eh_compare_dbl (sed_river_angle (r), G_PI*.5, 1e-12));

  sed_river_destroy (r);
}

void
test_river_name (void)
{
  gchar *name = g_strdup_printf ("Test River %d", g_test_rand_int ());
  Sed_riv r = sed_river_new (name);
  gchar *name_ptr = NULL;

  g_assert (r);

  g_assert (name!=sed_river_name (r));
  g_assert (sed_river_name_is (r,name));
  g_assert (sed_river_name_cmp (r,name)==0);
  g_assert_cmpstr (sed_river_name (r), ==, name);

  name[0] = 't';
  g_assert (sed_river_name_is (r,name));

  name[0] = 'x';
  g_assert (!sed_river_name_is (r,name));

  name_ptr = sed_river_name_loc (r);
  g_assert (name_ptr!=name);
  g_assert (name_ptr!=sed_river_name (r));

  name_ptr[0] = 't';
  g_assert (sed_river_name_is (r, name_ptr));

  name_ptr[0] = 'X';
  g_assert (sed_river_name_is (r, name_ptr));

  sed_river_destroy (r);

}

void
test_river_hinge (void)
{
  Sed_riv r = sed_river_new ("Hinge Test");
  Sed_riv rtn = NULL;
  Eh_ind_2 hinge;
  gint i = g_test_rand_int ();
  gint j = g_test_rand_int ();

  g_assert (r);

  hinge = sed_river_hinge (r);

  g_assert_cmpint (hinge.i, ==, 0);
  g_assert_cmpint (hinge.j, ==, 0);

  rtn = sed_river_set_hinge (r, i, j);

  g_assert (rtn==r);
  hinge = sed_river_hinge (r);

  g_assert_cmpint (hinge.i, ==, i);
  g_assert_cmpint (hinge.j, ==, j);

  sed_river_destroy (r);
}

void
test_cube_grid_elevation_all (void)
{
  Sed_cube p = new_test_cube ();

  g_assert (p);

  {
    gint i;
    const gint nx = sed_cube_n_x (p);
    const gint ny = sed_cube_n_y (p);
    const gint len = nx*ny;
    double* z_save = eh_new (double, len);

    for (i=0; i<len; i++)
      z_save[i] = g_test_rand_double ();

    for (i=0; i<len; i++)
      sed_cube_set_base_height (p, 0, i, z_save[i]);

    { /* Get elevations everywhere */
      Eh_dbl_grid z_grid = NULL;
      double* z = NULL;

      z_grid = sed_cube_elevation_grid (p, NULL);
      g_assert (z_grid!=NULL);
      g_assert_cmpint (eh_grid_n_x (z_grid), ==, nx);
      g_assert_cmpint (eh_grid_n_y (z_grid), ==, ny);

      z = eh_dbl_grid_data_start (z_grid);
      g_assert (z!=NULL);

      for (i=0; i<len; i++)
        g_assert (eh_compare_dbl (z[i], z_save[i], 1e-12));

      eh_grid_destroy (z_grid, TRUE);
    }

    eh_free (z_save);
  }

  sed_cube_destroy (p);
}

void
test_cube_grid_elevation_some (void)
{
  Sed_cube p = new_test_cube ();

  {
    gint i;
    const gint nx = sed_cube_n_x (p);
    const gint ny = sed_cube_n_y (p);
    const gint len = nx*ny;
    double* z_save = eh_new (double, len);

    for (i=0; i<len; i++)
      z_save[i] = g_test_rand_double ();

    for (i=0; i<len; i++)
      sed_cube_set_base_height (p, 0, i, z_save[i]);

    { /* Get elevations are certain columns */
      Eh_dbl_grid z_grid = NULL;
      double* z = NULL;
      gint n_id = g_test_rand_int_range (1, len);
      gint* id = eh_new (gint, n_id+1);
      gboolean* is_set = eh_new (gboolean, len);

      for (i=0; i<n_id; i++)
        id[i] = g_test_rand_int_range (0, len);
      id[n_id] = -1;

      for (i=0; i<len; i++)
        is_set[i] = FALSE;
      for (i=0; i<n_id; i++)
        is_set[id[i]] = TRUE;

      z_grid = sed_cube_elevation_grid (p, id);
      g_assert (z_grid!=NULL);
      g_assert_cmpint (eh_grid_n_x (z_grid), ==, nx);
      g_assert_cmpint (eh_grid_n_y (z_grid), ==, ny);

      z = eh_dbl_grid_data_start (z_grid);
      g_assert (z!=NULL);
    
      for (i=0; i<n_id; i++)
        g_assert (eh_compare_dbl (z[id[i]], z_save[id[i]], 1e-12));

      for (i=0; i<len; i++)
        if (is_set[i] == FALSE)
          g_assert (eh_compare_dbl (z[i], 0., 1e-12));

      eh_free (is_set);
      eh_free (id);
      eh_grid_destroy (z_grid, TRUE);
    }

    eh_free (z_save);
  }

  sed_cube_destroy (p);
}

int
main (int argc, char* argv[])
{
  eh_init_glib ();

  if (!sed_test_setup_sediment ("sediment"))
    eh_exit (EXIT_FAILURE);

  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/libsed/sed_cube/new",&test_sed_cube_new);
  g_test_add_func ("/libsed/sed_cube/destroy",&test_sed_cube_destroy);
  g_test_add_func ("/libsed/sed_cube/sequence_2",&test_sequence_2);
  g_test_add_func ("/libsed/sed_cube/to_cell",&test_cube_to_cell);
  g_test_add_func ("/libsed/sed_cube/get_size",&test_cube_get_size);
  g_test_add_func ("/libsed/sed_cube/erode",&test_cube_erode);
  g_test_add_func ("/libsed/sed_cube/deposit",&test_cube_deposit);
  g_test_add_func ("/libsed/sed_cube/base_height",&test_cube_base_height);
  g_test_add_func ("/libsed/sed_cube/add_river",&test_cube_river_add);
  g_test_add_func ("/libsed/sed_cube/add_river_mouth",
                   &test_cube_add_river_mouth);
  g_test_add_func ("/libsed/sed_cube/add_river_mouth_grid",
                   &test_cube_add_river_mouth_grid);
  g_test_add_func ("/libsed/sed_cube/river_north",&test_cube_river_north);
  g_test_add_func ("/libsed/sed_cube/is_land_ocean_cell",
                   &test_is_land_ocean_cell);
  g_test_add_func ("/libsed/sed_cube/is_boundary_cell",
                   &test_is_boundary_cell);
  g_test_add_func ("/libsed/sed_cube/shore_mask",&test_shore_mask);
  g_test_add_func ("/libsed/sed_cube/shore_ids",&test_shore_ids);

  g_test_add_func ("/libsed/sed_cube/river_path/ray",
                   &test_cube_river_path_ray);
  g_test_add_func ("/libsed/sed_cube/river_path/ends",
                   &test_cube_river_path_ends);

  g_test_add_func ("/libsed/sed_river/new",&test_river_new);
  g_test_add_func ("/libsed/sed_river/dup",&test_river_dup);
  g_test_add_func ("/libsed/sed_river/angle",&test_river_angle);
  g_test_add_func ("/libsed/sed_river/name",&test_river_name);
  g_test_add_func ("/libsed/sed_river/hinge",&test_river_hinge);

  g_test_add_func ("/libsed/sed_cube/grid/elevation_all",
                   &test_cube_grid_elevation_all);
  g_test_add_func ("/libsed/sed_cube/grid/elevation_some",
                   &test_cube_grid_elevation_some);

  g_test_run ();
}

