#include <glib.h>
#include "utils.h"
#include "eh_polygon.h"

Eh_polygon_2
eh_get_rectangle_polygon( Eh_pt_2 center , double dx , double dy )
{
   int n;
   Eh_polygon_2 poly=NULL;
   Eh_pt_2 *this_corner;
   gssize x_offset[4] = { -1 , +1 , +1 , -1 };
   gssize y_offset[4] = { -1 , -1 , +1 , +1 };

   for ( n=0 ; n<4 ; n++ )
   {
      this_corner = eh_new( Eh_pt_2 , 1 );

      this_corner->x = center.x + x_offset[n]*dx/2.;
      this_corner->y = center.y + y_offset[n]*dy/2.;

      poly = g_list_append( poly , this_corner );
   }
   return poly;
}

Eh_polygon_2
eh_get_polygon_from_grid( Eh_grid g , gssize i , gssize j )
{
   int n;
   Eh_polygon_2 poly=NULL;
   Eh_pt_2 *this_corner;
   gssize x_offset[4] = { -1 , -1 , +1 , +1 };
   gssize y_offset[4] = { -1 , +1 , +1 , -1 };

   for ( n=0 ; n<4 ; n++ )
   {
      this_corner = eh_new( Eh_pt_2 , 1 );

      this_corner->x = ( eh_grid_x(g)[x_offset[n]] + eh_grid_x(g)[i] )/ 2.;
      this_corner->y = ( eh_grid_y(g)[y_offset[n]] + eh_grid_y(g)[j] )/ 2.;

      poly = g_list_append( poly , this_corner );
   }
   return poly;
}

void
eh_destroy_polygon( Eh_polygon_2 p )
{
   GList *this_link;
   for ( this_link=p ; this_link ; this_link=this_link->next )
      eh_free( this_link->data );
   g_list_free( p );
}

GList*
eh_find_polygon_crossings( Eh_pt_2 start ,
                           double angle  ,
                           Eh_polygon_2 area ,
                           int in_or_out )
{
   GList *crossing=NULL, *in=NULL, *out=NULL;
   GList *this_link;
   Eh_pt_2 this_corner;
   Eh_pt_2 next_corner;
   Eh_pt_2 u, v, n;
   Eh_pt_2 *intercept;
   double angle_to_this_corner, angle_to_next_corner;
   double theta;
   double m_0, b_0, m_1, b_1;
   gboolean is_entering;

   m_0 = tan( angle );
   b_0 = start.y - start.x*m_0;

   for ( this_link=area ; this_link ; this_link = this_link->next )
   {
      this_corner = *((Eh_pt_2*)(this_link->data));
      if ( this_link->next )
         next_corner = *((Eh_pt_2*)(this_link->next->data));
      else
         next_corner = *((Eh_pt_2*)(area->data));

      angle_to_this_corner = atan2( this_corner.y - start.y ,
                                    this_corner.x - start.x );
      angle_to_next_corner = atan2(  next_corner.y - start.y ,
                                     next_corner.x - start.x );

      //---
      // u is the vector pointing from this corner to the next. n is its
      // normal.
      //---
      u = eh_get_dir_vector( this_corner , next_corner );
      n = eh_get_norm_vector( u );
      v = eh_get_dir_vector( start , this_corner );

      //---
      // If v is the same direction as the normal vector of this side,
      // then the line is exiting the polygon, otherwise if is entering.
      //---
      theta = eh_get_angle_between_vectors( v , n );
      if ( theta > -M_PI_2 && theta < M_PI_2 )
         is_entering = FALSE;
      else
         is_entering = TRUE;

      if ( is_entering )
         swap_dbl( angle_to_this_corner , angle_to_next_corner );

      //---
      // The ray intercepts this side.
      //---
      if (  (    (  is_entering && (in_or_out & POLYGON_IN_CROSSINGS ) )
              || ( !is_entering && (in_or_out & POLYGON_OUT_CROSSINGS) ) )
           && is_between_angles( angle                ,
                              angle_to_this_corner ,
                              angle_to_next_corner ) )
      {

         intercept = eh_new( Eh_pt_2 , 1 );

         if ( u.x == 0 )
         {
            intercept->x = this_corner.x;
            intercept->y = m_0*intercept->x + b_0;
         }
         else
         {
            //---
            // Calculate where the ray crosses this side.
            //---
            m_1 = ( next_corner.y - this_corner.y )
                / ( next_corner.x - this_corner.x );
            b_1 = this_corner.y - ( this_corner.x * m_1 );

            intercept->x = ( b_1 - b_0 ) / ( m_0 - m_1 );
            intercept->y = m_0*intercept->x + b_0;
         }

         if ( is_entering )
            in  = g_list_append( in  , intercept );
         else
            out = g_list_append( out , intercept );

      }

   }

   if ( in_or_out & POLYGON_IN_CROSSINGS )
      crossing = g_list_concat( crossing , in  );
   if ( in_or_out & POLYGON_OUT_CROSSINGS )
      crossing = g_list_concat( crossing , out );

   return crossing;
}

gboolean
is_between_angles( double angle , double angle_1 , double angle_2 )
{
   angle_1 = eh_reduce_angle( angle_1 );
   angle_2 = eh_reduce_angle( angle_2 );

   //---
   // The first angle will be greater than the second angle if the angle between
   // them crosses the negative y-axis.
   //---
   if ( angle_1 > angle_2 )
   {
      if ( angle < angle_2 )
         angle += 2.*M_PI;
      angle_2 += 2.*M_PI;
   }
   if ( angle > angle_1 && angle < angle_2 )
      return TRUE;
   else
      return FALSE;
}

gboolean
is_inside_area( Eh_pt_2 x , Eh_polygon_2 area )
{
   GList *crossings;
   GList *this_link;
   Eh_pt_2 *this_corner, *next_corner;
   gboolean is_inside = TRUE;
   guint number_of_crossings;
   Eh_pt_2 u;
   double angle;

   for ( this_link=area ; this_link && is_inside ; this_link = this_link->next )
   {
      this_corner = (Eh_pt_2*)(this_link->data);
      if ( this_link->next )
         next_corner = (Eh_pt_2*)(this_link->next->data);
      else
         next_corner = (Eh_pt_2*)(area->data);

      u = eh_get_dir_vector( *this_corner , *next_corner );

      angle = eh_get_vector_angle( u );
      crossings = eh_find_polygon_crossings( x , angle , area ,
                                               POLYGON_IN_CROSSINGS
                                             | POLYGON_OUT_CROSSINGS );

      number_of_crossings = g_list_length( crossings );

      //---
      // If the number of crossings is even, the point is outside of the
      // polygon.
      //---
      if ( number_of_crossings%2 == 0 )
         is_inside = FALSE;
   }

   return is_inside;
}

Eh_pt_2
eh_get_unit_vector( double angle )
{
   Eh_pt_2 u;
   u.x = cos( angle );
   u.y = sin( angle );
   return u;
}

Eh_pt_2
eh_get_dir_vector( Eh_pt_2 point_1 , Eh_pt_2 point_2 )
{
   Eh_pt_2 u;

   u.x = point_2.x - point_1.x;
   u.y = point_2.y - point_1.y;

   return eh_normalize_vector( u );
}

Eh_pt_2
eh_get_norm_vector( Eh_pt_2 u )
{
   return eh_get_unit_vector( eh_get_vector_angle( u )-M_PI_2 );
}

double
eh_get_vector_length( Eh_pt_2 u )
{
   return sqrt( pow( u.x , 2. ) + pow( u.y , 2. ) );
}

double
eh_get_vector_angle( Eh_pt_2 u )
{
   return atan2( u.y , u.x );
}

Eh_pt_2
eh_normalize_vector( Eh_pt_2 u )
{
   double r = eh_get_vector_length( u );
   u.x /= r;
   u.y /= r;
   return u;
}

double
eh_dot_vectors( Eh_pt_2 u , Eh_pt_2 v )
{
   return u.x*v.x + u.y*v.y;
}

double
eh_get_angle_between_vectors( Eh_pt_2 u , Eh_pt_2 v )
{
   return acos(   eh_dot_vectors( u , v )
                / eh_get_vector_length( u )
                / eh_get_vector_length( v ) );
}

pos_t*
createPosVec(int size)
{
   pos_t *v;
//   v = (pos_t*)malloc1D(sizeof(pos_t));
//   v->x = (double*)malloc1D(sizeof(double)*size);
//   v->y = (double*)malloc1D(sizeof(double)*size);
   v    = eh_new( pos_t  , 1    );
   v->x = eh_new( double , size );
   v->y = eh_new( double , size );
   v->size = size;
   return v;
}

#define DERIVATIVE_FORWARD_DIFFERENCE

double *derivative(pos_t v)
{
   int i;
   double *slope;
//   slope = (double*)malloc1D(sizeof(double)*v.size);
   slope = eh_new( double , v.size );
#if defined(DERIVATIVE_FORWARD_DIFFERENCE)
   for (i=0;i<v.size-1;i++)
      slope[i] = (v.y[i+1]-v.y[i])/(v.x[i+1]-v.x[i]);
   slope[v.size-1] = slope[v.size-2];
#elif defined(DERIVATIVE_BACKWARD_DIFFERENCE)
   for (i=1;i<v.size;i++)
      slope[i] = (v.y[i]-v.y[i-1])/(v.x[i]-v.x[i-1]);
   slope[0] = slope[1];
#elif defined(DERIVATIVE_CENTERED_DIFFERENCE)
   for (i=1;i<v.size-1;i++)
      slope[i] = (v.y[i+1]-v.y[i-1])/(v.x[i+1]-v.x[i-1]);
   slope[v.size-1] = slope[v.size-2];
   slope[0] = slope[1];
#endif
   return slope;
}

void
destroyPosVec(pos_t *v)
{
   eh_free(v->x);
   eh_free(v->y);
   eh_free(v);
   return;
}

Eh_pt_2
eh_create_pt_2( double x , double y )
{
   Eh_pt_2 pt;
   pt.x = x;
   pt.y = y;
   return pt;
}

gboolean
eh_cmp_pt_2( Eh_pt_2 a , Eh_pt_2 b , double eps )
{
   return fabs( a.x-b.x ) < eps && fabs( a.y-b.y ) < eps;
}

