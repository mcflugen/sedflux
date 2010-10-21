#include "sed_river.h"
#include "sed_cube.h"

/**
   A structure to describe the hinge point of a stream
*/
typedef struct
{
   double angle;      ///< The angle the stream leaves the hinge point from
   gint   i;          ///< The x-index of the hinge point
   gint   j;          ///< The y-index of the hinge point
   double min_angle;  ///< The minimum angle the steam can make
   double max_angle;  ///< The maximum angle the stream can make
   double std_dev;    ///< The standard deviation of stream avulsions
}
Sed_riv_hinge;

/**
   Describe a river.
*/
CLASS( Sed_riv )
{
   Sed_hydro      data;    ///< The hydrological characteristics of the river
   Sed_riv_hinge* hinge;   ///< The hinge point of the river
   gint           x_ind;   ///< The x-index of the river mouth
   gint           y_ind;   ///< The y-index of the river mouth
   gchar*         name;    ///< The name of the river
   Sed_riv        l;
   Sed_riv        r;
};

Sed_riv_hinge* sed_river_hinge_new( );
Sed_riv_hinge* sed_river_hinge_copy( Sed_riv_hinge* d , Sed_riv_hinge* s );
Sed_riv_hinge* sed_river_hinge_destroy( Sed_riv_hinge* h );

Sed_riv_hinge*
sed_river_hinge_new( )
{
   Sed_riv_hinge* h = eh_new( Sed_riv_hinge , 1 );

   h->angle     = 0.;
   h->i         = 0;
   h->j         = 0;
   h->min_angle = -G_PI;
   h->max_angle =  G_PI;
   h->std_dev   = 0.;

   return h;
}

Sed_riv_hinge*
sed_river_hinge_copy( Sed_riv_hinge* d , Sed_riv_hinge* s )
{
   eh_return_val_if_fail( s , NULL );

   if ( !d )
      d = sed_river_hinge_new();

   d->angle      = s->angle;
   d->i          = s->i;
   d->j          = s->j;
   d->min_angle  = s->min_angle;
   d->max_angle  = s->max_angle;
   d->std_dev    = s->std_dev;

   return d;
}

Sed_riv_hinge*
sed_river_hinge_destroy( Sed_riv_hinge* h )
{
   if ( h )
      eh_free( h );
   return NULL;
}

Sed_riv
sed_river_new (const gchar* name)
{
   Sed_riv r;

   NEW_OBJECT( Sed_riv , r );

   r->data  = NULL;
   r->hinge = sed_river_hinge_new();
   r->x_ind = 0;
   r->y_ind = 0;
   r->name  = g_strdup( name );
   r->l     = NULL;
   r->r     = NULL;

   return r;
}

Sed_riv
sed_river_copy( Sed_riv d , Sed_riv s )
{
   eh_return_val_if_fail( s , NULL );

   {
      if ( !d )
         d = sed_river_new( s->name );

      d->x_ind = s->x_ind;
      d->y_ind = s->y_ind;

      d->data  = sed_hydro_copy      ( d->data  , s->data  );
      d->hinge = sed_river_hinge_copy( d->hinge , s->hinge );
      sed_river_copy      ( d->l     , s->l     );
      sed_river_copy      ( d->r     , s->r     );

      eh_free( d->name );
      d->name = g_strdup( s->name );
   }

   return d;
}

Sed_riv
sed_river_dup ( Sed_riv s )
{
   return sed_river_copy( NULL , s );
}

Sed_riv
sed_river_set_hydro( Sed_riv s , const Sed_hydro h )
{
   if ( s && h )
      s->data = sed_hydro_copy( s->data , h );
   return s;
}

Sed_riv
sed_river_set_width( Sed_riv s , double width )
{
   if ( s && s->data )
   {
      sed_hydro_set_width( s->data , width );
   }
   return s;
}

Sed_riv
sed_river_set_depth( Sed_riv s , double depth )
{
   if ( s && s->data )
   {
      sed_hydro_set_depth( s->data , depth );
   }
   return s;
}

Sed_riv
sed_river_set_velocity( Sed_riv s , double velocity )
{
   if ( s && s->data )
   {
      sed_hydro_set_velocity( s->data , velocity );
   }
   return s;
}

Sed_riv
sed_river_set_bedload( Sed_riv s , double bedload )
{
   if ( s && s->data )
   {
      sed_hydro_set_bedload( s->data , bedload );
   }
   return s;
}

Sed_riv
sed_river_set_angle( Sed_riv s , double a )
{
   if ( s && s->hinge )
   {
      if (eh_compare_dbl (a, G_PI, 1e-12))
         a -= 1e-12;

      a = eh_reduce_angle (a);

      if (sed_river_max_angle (s)<G_PI)
        s->hinge->angle = a;
      else
      {
        if (a<sed_river_min_angle (s))
          s->hinge->angle = a + 2.*G_PI;
        else
          s->hinge->angle = a;
      }

      eh_clamp (s->hinge->angle, s->hinge->min_angle, s->hinge->max_angle);
   }
   return s;
}

Sed_riv
sed_river_increment_angle( Sed_riv s , double da )
{
   if ( s && s->hinge )
      sed_river_set_angle( s , s->hinge->angle + da );
   return s;
}

Sed_riv
sed_river_set_angle_limit( Sed_riv s , double a_min , double a_max )
{
   if ( s && s->hinge )
   {
      if ( eh_compare_dbl( a_max , G_PI , 1e-12 ) )
         a_max -= 1e-12;

      eh_require (a_min<=a_max);
      s->hinge->min_angle = a_min;
      s->hinge->max_angle = a_max;

      a_min = eh_reduce_angle( a_min );
      a_max = eh_reduce_angle( a_max );

      if (a_max<a_min)
        a_max += 2.*G_PI;

      if ( a_min<=a_max )
      {
         s->hinge->min_angle = a_min;
         s->hinge->max_angle = a_max;
      }
      else
        eh_require_not_reached ();

      sed_river_set_angle (s, s->hinge->angle);
   }
   return s;
}

Sed_riv
sed_river_set_hinge( Sed_riv r , gint i , gint j )
{
   if ( r )
   {
      r->hinge->i = i;
      r->hinge->j = j;
   }
   return r;
}

Sed_riv
sed_river_set_mouth( Sed_riv r , gint i , gint j )
{
   if ( r )
   {
      r->x_ind = i;
      r->y_ind = j;
   }
   return r;
}

Sed_riv
sed_river_adjust_mass( Sed_riv s , double f )
{
   eh_return_val_if_fail( s , NULL );

   sed_hydro_adjust_mass( s->data , f );
   return s;
}

double
sed_river_water_flux( Sed_riv s )
{
   eh_return_val_if_fail( s       , 0. );
   eh_return_val_if_fail( s->data , 0. );

   return sed_hydro_water_flux( s->data );
}

double
sed_river_sediment_load( Sed_riv s )
{
   eh_return_val_if_fail( s       , 0. );
   eh_return_val_if_fail( s->data , 0. );

   return sed_hydro_total_load( s->data );
}

double
sed_river_suspended_load( Sed_riv s )
{
   eh_return_val_if_fail( s       , 0. );
   eh_return_val_if_fail( s->data , 0. );

   return sed_hydro_suspended_load( s->data );
}

Sed_hydro
sed_river_hydro   ( Sed_riv s )
{
   eh_return_val_if_fail( s       , NULL );
   eh_return_val_if_fail( s->data , NULL );

   return sed_hydro_dup( s->data );
}

gboolean
sed_river_is_hyperpycnal( Sed_riv s )
{
   eh_require( s );

   return sed_hydro_is_hyperpycnal( s->data );
}

double
sed_river_concentration( Sed_riv s )
{
   eh_require( s );
   return sed_hydro_flow_density( s->data , sed_rho_fresh_water() );
}

gint*
sed_river_n_branches_helper( Sed_riv s , gint* n )
{
   if ( s )
   {
      sed_river_n_branches_helper( s->l , n );
      sed_river_n_branches_helper( s->r , n );

      (*n) += 1;
   }

   return n;
}

/** Count the number of branches of a river

Count all of the branches of a river, including the river trunk.  Thus,
if the river does not branch, sed_river_n_branches will return 1.  If the
trunk branches only once, the size is 3.

\param s   A Sed_river

\return The total number of branches of the river
*/
gint
sed_river_n_branches( Sed_riv s )
{
   gint n = 0;

   if ( s )
      sed_river_n_branches_helper( s , &n );

   return n;
}

double
sed_river_width   ( Sed_riv s )
{
   eh_return_val_if_fail( s       , 0. );
   eh_return_val_if_fail( s->data , 0. );

   return sed_hydro_width( s->data );
}

double
sed_river_depth   ( Sed_riv s )
{
   eh_return_val_if_fail( s       , 0. );
   eh_return_val_if_fail( s->data , 0. );

   return sed_hydro_depth( s->data );
}

double
sed_river_velocity( Sed_riv s )
{
   eh_return_val_if_fail( s       , 0. );
   eh_return_val_if_fail( s->data , 0. );

   return sed_hydro_velocity( s->data );
}

double
sed_river_bedload( Sed_riv s )
{
   eh_return_val_if_fail( s       , 0. );
   eh_return_val_if_fail( s->data , 0. );

   return sed_hydro_bedload( s->data );
}

double
sed_river_angle   ( Sed_riv s )
{
   eh_return_val_if_fail( s        , 0. );
   eh_return_val_if_fail( s->hinge , 0. );

   return s->hinge->angle;
}

double
sed_river_angle_to_deg( Sed_riv s )
{
   return sed_river_angle(s)*S_DEGREES_PER_RAD;
}

double
sed_river_min_angle( Sed_riv s )
{
   eh_return_val_if_fail( s        , 0. );
   eh_return_val_if_fail( s->hinge , 0. );

   return s->hinge->min_angle;
}

double
sed_river_max_angle( Sed_riv s )
{
   eh_return_val_if_fail( s        , 0. );
   eh_return_val_if_fail( s->hinge , 0. );

   return s->hinge->max_angle;
}

Eh_ind_2
sed_river_hinge( Sed_riv s )
{
   Eh_ind_2 pos = {-1,-1};
   if ( s )
   {
      pos.i = s->hinge->i;
      pos.j = s->hinge->j;
   }
   return pos;
}

Eh_ind_2
sed_river_mouth( Sed_riv s )
{
   Eh_ind_2 pos = {-1,-1};
   if ( s )
   {
      pos.i = s->x_ind;
      pos.j = s->y_ind;
   }
   return pos;
}

gboolean
sed_river_mouth_is( Sed_riv s , gint i , gint j )
{
   eh_return_val_if_fail( s , FALSE );
   return s->x_ind==i && s->y_ind==j;
}

gchar*
sed_river_name( Sed_riv s )
{
   eh_return_val_if_fail( s , NULL );

   return g_strdup( s->name );
}

gchar*
sed_river_name_loc( Sed_riv s )
{
   eh_return_val_if_fail( s , NULL );

   return s->name;
}

gboolean
sed_river_name_is( Sed_riv s , gchar* name )
{
   eh_return_val_if_fail( s && name , FALSE );

   return g_ascii_strcasecmp( s->name , name )==0;
}

gint
sed_river_name_cmp( Sed_riv s , const gchar* name )
{
   eh_return_val_if_fail( s && s->name , -1 );
   return g_ascii_strcasecmp( s->name , name );
}

gboolean
sed_river_has_children( Sed_riv s )
{
   eh_return_val_if_fail( s , FALSE );

   return s->l && s->r;
}

Sed_riv
sed_river_left( Sed_riv s )
{
   eh_return_val_if_fail( s , NULL );
   return s->l;
}

Sed_riv
sed_river_right( Sed_riv s )
{
   eh_return_val_if_fail( s , NULL );
   return s->r;
}

Sed_riv
sed_river_split_discharge( Sed_riv s )
{
   eh_return_val_if_fail( s       , NULL );
   eh_return_val_if_fail( s->data , s    );

   sed_hydro_set_width  ( s->data , sed_hydro_width(s->data)/2.   );
   sed_hydro_set_bedload( s->data , sed_hydro_bedload(s->data)/2. );
   return s;
}

Sed_riv
sed_river_split( Sed_riv s )
{
   if ( s )
   {
      if ( !sed_river_has_children(s) )
      {
         Sed_riv child = sed_river_dup( s );

         s->l = sed_river_split_discharge( child );
         s->r = sed_river_dup( s->l );

         sed_river_set_angle( s->l , .5*(sed_river_angle(s)+sed_river_max_angle(s)) );
         sed_river_set_angle( s->r , .5*(sed_river_angle(s)+sed_river_min_angle(s)) );
      }
   }

   return s;
}

/** Find the longest branch of a river

The length of a  branch is determined by its number of child branches.  The
longest branch is the branch that has the FEWEST children.  In the case of a
tie, the left-most branch is chosen.  That is, the one that was first created.

\param s A Sed_riv

\return The longest branch
*/
Sed_riv
sed_river_longest_branch( Sed_riv s )
{
   Sed_riv longest = NULL;

   if ( s )
   {
      if ( !sed_river_has_children(s) )
         longest = s;
      else if ( sed_river_n_branches(s->l) > sed_river_n_branches(s->r) )
         longest = sed_river_longest_branch( s->r );
      else
         longest = sed_river_longest_branch( s->l );
   }

   return longest;
}

Sed_riv
sed_river_destroy( Sed_riv s )
{
   if ( s )
   {
      eh_free( s->name );

      sed_river_hinge_destroy( s->hinge     );
      sed_hydro_destroy      ( s->data      );
      sed_river_destroy      ( s->l         );
      sed_river_destroy      ( s->r         );

      eh_free( s );
   }
   return NULL;
}

Sed_riv**
sed_river_leaves_helper( Sed_riv s , Sed_riv** leaf )
{
   if ( !sed_river_has_children(s) )
      eh_strv_append( (gchar***)leaf , (gchar*)s );
   else
   {
      sed_river_leaves_helper( s->l , leaf );
      sed_river_leaves_helper( s->r , leaf );
   }

   return leaf;
}

/** Get an array of the 'leaf' branches of a Sed_riv

Construct a NULL terminated array of Sed_riv's that represent the 'leaf' branches
of a Sed_riv.  These are the smallest branches of the river that are nearest the
mouth of the river.  If the input Sed_riv has no children, then it is returned as the only
branch (as the only element in a NULL-terminated array).

When finished, the returned array should be freed.  The elements of the
array should not be destroyed however as they reference the actual location
of the children of the parent Sed_riv.

\param s   The parent Sed_riv

\return A NULL-terminated array of Sed_riv structures

\see sed_river_branches
*/
Sed_riv*
sed_river_leaves( Sed_riv s )
{
   Sed_riv* leaf = NULL;

   if ( s )
   {
      sed_river_leaves_helper( s , &leaf );
   }

   return leaf;
}

Sed_riv**
sed_river_branches_helper( Sed_riv s , Sed_riv** leaf )
{

   if ( s )
   {
      sed_river_branches_helper( s->l , leaf );
      sed_river_branches_helper( s->r , leaf );

      eh_strv_append( (gchar***)leaf , (gchar*)s );
   }

   return leaf;
}

/** Get an array of all the branches of a Sed_riv

Construct a NULL terminated array of Sed_riv's that represent all of the branches
of a Sed_riv.  If the input Sed_riv has no children, then it is returned as the only
branch (as the only element in a NULL-terminated array).

When finished, the returned array should be freed.  The elements of the
array should not be destroyed however as they reference the actual location
of the children of the parent Sed_riv.

\param s   The parent Sed_riv

\return A NULL-terminated array of Sed_riv structures

\see sed_river_leaves
*/
Sed_riv*
sed_river_branches( Sed_riv s )
{
   Sed_riv* leaf = NULL;

   if ( s )
   {
      sed_river_branches_helper( s , &leaf );
   }

   return leaf;
}

gint
sed_river_fprint( FILE* fp , Sed_riv s )
{
   gint n=0;

   if ( s )
   {
      n += sed_hydro_fprint( fp , s->data );
      n += fprintf( fp , "Name : %s\n" , s->name?(s->name):"(null)" );
      n += fprintf( fp , "River mouth position : %d, %d\n" , s->x_ind , s->y_ind );
      n += fprintf( fp , "River hinge position : %d, %d\n" , sed_river_hinge(s).i , sed_river_hinge(s).j );
      n += fprintf( fp , "River angle (degs)   : %f\n" , sed_river_angle( s )/S_RADS_PER_DEGREE );
      n += fprintf( fp , "River angle limits (degs) : %f, %f\n" , sed_river_min_angle(s)/S_RADS_PER_DEGREE ,
                                                                  sed_river_max_angle(s)/S_RADS_PER_DEGREE );
   }

   return n;
}

gint
sed_river_fwrite( FILE* fp , Sed_riv s )
{
   gint n=0;

   if ( s )
   {
      eh_require_not_reached();
   }

   return n;
}

Sed_riv
sed_river_fread( FILE* fp , Sed_riv d )
{
   eh_require_not_reached();
   return NULL;
}


