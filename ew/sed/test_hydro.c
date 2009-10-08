#include <glib.h>

#include "utils/utils.h"
#include "sed_hydro.h"
#include "sed_input_files.h"

void
test_sed_hydro_new (void)
{
   Sed_hydro h = sed_hydro_new( 5 );

   g_assert (h!=NULL);
   g_assert_cmpint (sed_hydro_size(h), ==, 5);

   {
      double qb = sed_hydro_bedload(h);
      double u  = sed_hydro_velocity(h);
      double w  = sed_hydro_width(h);
      double d  = sed_hydro_depth(h);
      double qs = sed_hydro_suspended_concentration(h);
      double dt = sed_hydro_duration(h);

      g_assert (eh_compare_dbl (qb ,0 ,1e-12));
      g_assert (eh_compare_dbl (u  ,0 ,1e-12));
      g_assert (eh_compare_dbl (w  ,0 ,1e-12));
      g_assert (eh_compare_dbl (d  ,0 ,1e-12));
      g_assert (eh_compare_dbl (qs ,0 ,1e-12));
      g_assert (eh_compare_dbl (dt ,1 ,1e-12));
   }

   sed_hydro_destroy( h );
}


void
test_sed_hydro_copy (void)
{
   Sed_hydro a   = sed_hydro_new( 5 );
   Sed_hydro a_0 = sed_hydro_new( 5 );
   Sed_hydro b   = sed_hydro_new( 5 );
   Sed_hydro b_0;

   sed_hydro_set_width   ( a , 500  );
   sed_hydro_set_depth   ( a , 7    );
   sed_hydro_set_velocity( a , 1.23 );
   sed_hydro_set_duration( a , 33   );

   sed_hydro_set_width   ( a_0 , 500  );
   sed_hydro_set_depth   ( a_0 , 7    );
   sed_hydro_set_velocity( a_0 , 1.23 );
   sed_hydro_set_duration( a_0 , 33   );

   b_0 = sed_hydro_copy( b , a );

   g_assert (b_0==b);
   g_assert (sed_hydro_is_same(b,a));
   g_assert (sed_hydro_is_same(a,a_0));

   sed_hydro_destroy( a   );
   sed_hydro_destroy( a_0 );
   sed_hydro_destroy( b   );
}


void
test_sed_hydro_add_cell (void)
{
   double f[5]   = { .2 , .2 , .2 , .2 , .2 };
   Sed_hydro a   = sed_hydro_new( 4 );
   Sed_cell  c   = sed_cell_new_sized( 5 , 1. , f );
   double volume = 12.;
   double load_0, load_1, cell_load;

   sed_hydro_set_width   ( a , 500  );
   sed_hydro_set_depth   ( a , 7    );
   sed_hydro_set_velocity( a , 1.23 );
   sed_hydro_set_duration( a , 33   );

   load_0 = sed_hydro_total_load( a );

   sed_cell_resize( c , volume );
   sed_hydro_add_cell( a , c );

   load_1 = sed_hydro_total_load( a );

   cell_load = sed_cell_density( c )*volume;

   g_assert (eh_compare_dbl (load_1, load_0+cell_load, 1e-12));

   sed_hydro_destroy( a );
   sed_cell_destroy ( c );
}


void
test_sed_hydro_subtract_cell (void)
{
   double f[5]   = { .2 , .2 , .2 , .2 , .2 };
   Sed_hydro a   = sed_hydro_new( 4 );
   Sed_cell  c   = sed_cell_new_sized( 5 , 1. , f );
   double volume = 12.;
   double load_0, load_1, cell_load;

   sed_hydro_set_width   ( a , 500  );
   sed_hydro_set_depth   ( a , 7    );
   sed_hydro_set_velocity( a , 1.23 );
   sed_hydro_set_duration( a , 33   );

   sed_cell_resize( c , volume );
   sed_hydro_add_cell( a , c );

   load_0 = sed_hydro_total_load( a );

   sed_cell_resize( c , volume/5. );
   sed_hydro_subtract_cell( a , c );

   load_1    = sed_hydro_total_load( a );
   cell_load = sed_cell_density( c )*volume/5.;

   g_assert (eh_compare_dbl(load_1, load_0-cell_load, 1e-12));

   sed_cell_resize( c , volume );
   sed_hydro_subtract_cell( a , c );

   load_1 = sed_hydro_total_load( a );

   g_assert (eh_compare_dbl (load_1, 0, 1e-12));

   sed_hydro_destroy( a );
   sed_cell_destroy ( c );
}


void
test_sed_hydro_copy_null (void)
{
   Sed_hydro a   = sed_hydro_new( 5 );
   Sed_hydro b;

   sed_hydro_set_width   ( a , 500  );
   sed_hydro_set_depth   ( a , 7    );
   sed_hydro_set_velocity( a , 1.23 );
   sed_hydro_set_duration( a , 33   );

   b = sed_hydro_copy( NULL , a );

   g_assert (b!=NULL);
   g_assert (b!=a);
   g_assert (sed_hydro_is_same(b,a));

   sed_hydro_destroy( a   );
   sed_hydro_destroy( b   );
}


void
test_sed_hydro_init (void)
{
  GError* error = NULL;
  gchar* buffer = g_strjoinv ("\n",_default_hydro_inline_file);
  Sed_hydro* a = sed_hydro_scan_text (buffer, &error);
  Sed_hydro  b = sed_hydro_new( 4 );
  const double dt_set = 365./4.;
  const double qb_set = 100.;
  const double u_set = 2.;
  const double w_set = 125.;
  const double d_set = 2.;

  g_assert (a||error!=NULL);
  if (error)
    eh_print_on_error (error, "test_sed_hydro_init");

  g_assert (error==NULL);
  g_assert (a);
  g_assert (a[0]);
  g_assert (b);

  sed_hydro_set_duration (b, dt_set);
  sed_hydro_set_bedload  (b, qb_set);
  sed_hydro_set_velocity (b, u_set);
  sed_hydro_set_width    (b, w_set);
  sed_hydro_set_depth    (b, d_set);

  {
    double qb = sed_hydro_bedload (a[0]);
    double u  = sed_hydro_velocity(a[0]);
    double w  = sed_hydro_width   (a[0]);
    double d  = sed_hydro_depth   (a[0]);
    double dt = sed_hydro_duration(a[0]);

    g_assert (eh_compare_dbl (qb, qb_set, 1e-12));
    g_assert (eh_compare_dbl (u, u_set, 1e-12));
    g_assert (eh_compare_dbl (w, w_set, 1e-12));
    g_assert (eh_compare_dbl (d, d_set, 1e-12));
    g_assert (eh_compare_dbl (dt, dt_set, 1e-12));
  }

  g_assert (sed_hydro_is_same(b,a[0]));

  sed_hydro_array_destroy( a );
  sed_hydro_destroy( b );
}


void
test_sed_hydro_is_same (void)
{
  GError* error = NULL;
  gchar* buffer = g_strjoinv ("\n",_default_hydro_inline_file);
  Sed_hydro* a = NULL;
  Sed_hydro* b = NULL;

  a = sed_hydro_scan_text (buffer, &error);

  g_assert (a||error!=NULL);
  if (error)
    eh_print_on_error (error, "test_hydro_is_same");
  g_assert (error==NULL);

  b = sed_hydro_scan_text (buffer, &error);
  g_assert (b||error!=NULL);
  if (error)
    eh_print_on_error (error, "test_hydro_is_same");
  g_assert (error==NULL);

  {
    const int len_a = g_strv_length ((gchar**)a);
    const int len_b = g_strv_length ((gchar**)b);
    int i;

    g_assert_cmpint (len_a, >, 0);
    g_assert_cmpint (len_b, >, 0);
    g_assert_cmpint (len_a, ==, len_b);

    for (i=0; i<len_a; i++)
      g_assert (sed_hydro_is_same (b[i],a[i]));
  }

  sed_hydro_array_destroy( a );
  sed_hydro_array_destroy( b );
}


void
test_sed_hydro_file_new_inline (void)
{
  gchar* name_used = NULL;
  Sed_hydro_file f;
  Sed_hydro a;

  {
    GError* error = NULL;
    int fd = g_file_open_tmp ("sed_hydro_test_XXXXXX", &name_used, &error);

    g_assert (fd!=-1);
    eh_print_on_error (error, "test_sed_hydro_file_new_inline");
    g_assert (error==NULL);

    {
      FILE* fp = fdopen (fd, "w");
      gchar** line = _default_hydro_inline_file;

      for (; *line; line++ )
        fprintf (fp, "%s\n", *line);

      fclose (fp);
    }
  }

  f = sed_hydro_file_new (name_used, SED_HYDRO_INLINE, FALSE, TRUE, NULL);
  a = sed_hydro_file_read_record (f);

  {
    const double dt_set = 365./4.;
    const double qb_set = 100.;
    const double u_set = 2.;
    const double w_set = 125.;
    const double d_set = 2.;

    g_assert (eh_compare_dbl (sed_hydro_bedload (a), qb_set, 1e-12));
    g_assert (eh_compare_dbl (sed_hydro_velocity (a), u_set, 1e-12));
    g_assert (eh_compare_dbl (sed_hydro_width (a) , w_set, 1e-12));
    g_assert (eh_compare_dbl (sed_hydro_depth (a), d_set, 1e-12));
    g_assert (eh_compare_dbl (sed_hydro_duration (a), dt_set, 1e-12));
  }

  {
    Sed_hydro b;
    int i;

    for (i=0; i<3; i++)
    {
      b = sed_hydro_file_read_record (f);
      g_assert (!sed_hydro_is_same (b,a));
      sed_hydro_destroy(b);
    }

    b = sed_hydro_file_read_record (f);
    g_assert (sed_hydro_is_same(b,a));
    sed_hydro_destroy(b);
  }

  sed_hydro_destroy( a );
  sed_hydro_file_destroy( f );
}


void
test_sed_hydro_file_new_binary (void)
{
   Sed_hydro_file f = sed_hydro_file_new( SED_HYDRO_TEST_FILE , SED_HYDRO_HYDROTREND , FALSE , TRUE , NULL );
   Sed_hydro a = sed_hydro_file_read_record( f );

   {
      double qb = sed_hydro_bedload (a);
      double u  = sed_hydro_velocity(a);
      double w  = sed_hydro_width   (a);
      double d  = sed_hydro_depth   (a);
      double dt = sed_hydro_duration(a);

      g_assert (eh_compare_dbl (qb, 4.467378, 1e-6));
      g_assert (eh_compare_dbl (u, .901985, 1e-6));
      g_assert (eh_compare_dbl (w, 95.524864, 1e-6));
      g_assert (eh_compare_dbl (d, 4.236207, 1e-6));
      g_assert (eh_compare_dbl (dt, 1, 1e-6));
   }

   sed_hydro_destroy( a );
   sed_hydro_file_destroy( f );
}


void
test_sed_hydro_file_new_buffer (void)
{
   double a_load, b_load;

   {
      Sed_hydro a;
      Sed_hydro_file f = sed_hydro_file_new( SED_HYDRO_TEST_FILE ,
                                             SED_HYDRO_HYDROTREND ,
                                             TRUE , TRUE , NULL );
      double dt;

      for ( dt=0 ; dt<365 ; )
      {
         a   = sed_hydro_file_read_record( f );
         dt += sed_hydro_duration( a );
         if (dt <= 365)
            a_load += sed_hydro_total_load( a );
         sed_hydro_destroy( a );
      }

      sed_hydro_file_destroy( f );
   }

   {
      gssize i;
      Sed_hydro b;
      Sed_hydro_file f = sed_hydro_file_new( SED_HYDRO_TEST_FILE ,
                                             SED_HYDRO_HYDROTREND ,
                                             FALSE , TRUE , NULL );

      for (i=0, b_load=0; i<365; i++);
      {
         b       = sed_hydro_file_read_record( f );
         b_load += sed_hydro_total_load( b );
         sed_hydro_destroy( b );
      }

      sed_hydro_file_destroy( f );
   }

   g_assert (eh_compare_dbl (a_load, b_load, 1e-12));
}


int
main (int argc, char* argv[])
{
  Sed_sediment s     = NULL;
  GError*      error = NULL;

  eh_init_glib ();

  s = sed_sediment_scan (SED_SEDIMENT_TEST_FILE, &error);

  if ( s )
    sed_sediment_set_env (s);
  else
    eh_error ("%s: Unable to read sediment file: %s",
              SED_SEDIMENT_TEST_FILE, error->message);

  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/libsed/sed_hydro/new", &test_sed_hydro_new);
  g_test_add_func ("/libsed/sed_hydro/copy", &test_sed_hydro_copy);
  g_test_add_func ("/libsed/sed_hydro/add_cell", &test_sed_hydro_add_cell);
  g_test_add_func ("/libsed/sed_hydro/subtract_cell", &test_sed_hydro_subtract_cell);
  g_test_add_func ("/libsed/sed_hydro/copy_null", &test_sed_hydro_copy_null);
  g_test_add_func ("/libsed/sed_hydro/init", &test_sed_hydro_init);
  g_test_add_func ("/libsed/sed_hydro/is_same", &test_sed_hydro_is_same);
  g_test_add_func ("/libsed/sed_hydro/new_inline", &test_sed_hydro_file_new_inline);
  //g_test_add_func ("/libsed/sed_hydro/new_binary", &test_sed_hydro_file_new_binary);
  //g_test_add_func ("/libsed/sed_hydro/new_buffer", &test_sed_hydro_file_new_buffer);

  g_test_run ();
}

