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

#include <stdio.h>
#include <math.h>
#include <glib.h>
#include "utils.h"
#include "sed_sedflux.h"
#include "avulsion.h"

static char *help_msg[] =
{
" avulsion - create a family of avulsing streams.      ",
"                                                      ",
" parameters :                                         ",
"                                                      ",
"  dev     : the standard deviation to use in the      ",
"          : avulsion routine. [1]                     ",
"                                                      ",
"  n       : the number of streams to model. [1]       ",
"                                                      ",
"  len     : the number of time steps to run the model ",
"          : for. [1]                                  ",
"                                                      ",
"  eps     : the minimum angle between streams before  ",
"          : they merge (in degrees). [5]              ",
"                                                      ",
"  a       : the maximum angle (in degrees) allowed    ",
"          : between adjacent streams before a new     ",
"          : stream is created. [45]                   ",
"                                                      ",
"  verbose : be verbose. [off]                         ",
"                                                      ",
"  help    : print this help message and exit. [off]   ",
"                                                      ",
NULL
};

typedef struct
{
   double angle;
   double stddev;
   double size;
   int id;
}
stream_st;

GList *avulse_streams( GList *streams );
GList *create_streams( GList *streams , double alpha );
GList *merge_streams( GList *streams , double eps );
GList *print_streams( GList *streams );
GList *print_streams_as_table( GList *streams );
void avulse_stream( stream_st *stream , gpointer data );
int sort_streams_by_angle( stream_st *s1 , stream_st *s2 );
int sort_streams_by_discharge( stream_st *s1 , stream_st *s2 );
int sort_streams_by_id( stream_st *s1 , stream_st *s2 );
void combine_rivers( Sed_river *r_1 , Sed_river *r_2 );

void sed_merge_all_rivers( Sed_cube c , double eps );
GList *merge_rivers( GList *rivers , double eps );
int sort_rivers_by_angle( Sed_river *r_1 , Sed_river *r_2 );
int sort_rivers_by_discharge( Sed_river *r_1 , Sed_river *r_2 );
void combine_river_discharge( Sed_river *r_1 , Sed_river *r_2 );
Sed_river *split_river_discharge( Sed_river *r_1 , Sed_river *r_2 );
void sed_split_all_rivers( Sed_cube c , double alpha );
GList *create_rivers( GList *rivers , double alpha );
void deposit_sediment_at_river_mouth( Sed_cube c );

int main( int argc , char *argv[] )
{
   int i, j;
   int n_i, n_j;
   int n_rivers, n_steps;
   double d_theta;
   Eh_args *args;
   double stddev, eps, alpha;
   gboolean verbose;
   Sed_cube cube;
   Sed_river *new_river;
   Eh_dbl_grid bathy_grid;
   Eh_ind_2 hinge_point;
   Sed_hydro hydro_data;
  
   args = eh_opts_init(argc,argv);
   if ( eh_check_opts( args , NULL , NULL , help_msg )!=0 )
      eh_exit(-1);

   stddev    = eh_get_opt_dbl ( args , "dev" , 1.    );
   eps       = eh_get_opt_dbl ( args , "eps" , 5.    );
   alpha     = eh_get_opt_dbl ( args , "a"   , 45.   );
   n_i       = eh_get_opt_int ( args , "ni"  , 100.  );
   n_j       = eh_get_opt_int ( args , "nj"  , 100.  );
   n_rivers  = eh_get_opt_int ( args , "n"   , 1     );
   n_steps   = eh_get_opt_int ( args , "len" , 1     );
   verbose   = eh_get_opt_bool( args , "v"   , FALSE );

   if ( verbose )
      eh_print_all_opts( args , "avulsion" , stderr );

   eps   *= M_PI/180.;
   alpha *= M_PI/180.;

   {
      Sed_sediment sediment_type = sed_sediment_scan( SED_SEDIMENT_TEST_FILE );

      sed_sediment_set_env( sediment_type );

      sed_sediment_destroy( sediment_type );
   }

   bathy_grid    = sed_get_floor_3_default( 1 , n_i , n_j );
   hydro_data    = sed_hydro_init( NULL );
   cube          = sed_cube_new( eh_grid_n_x(bathy_grid) ,
                                 eh_grid_n_y(bathy_grid) );
   sed_cube_set_dz( cube , 1. );
   sed_cube_set_bathy( cube , bathy_grid );

   hinge_point.i = eh_grid_n_x(bathy_grid)/2;
   hinge_point.j = 0;

   d_theta = M_PI/n_rivers;
   for ( i=0 ; i<n_rivers ; i++ )
   {
      new_river                = sed_create_river( sed_sediment_env_size() ,
                                                   &hinge_point );
if ( i==0 )
{
      new_river->hinge->angle     = d_theta*(i+1);
      new_river->hinge->x         = hinge_point.i;
      new_river->hinge->y         = hinge_point.j;
      new_river->hinge->min_angle = 0;
      new_river->hinge->max_angle = M_PI;
      new_river->hinge->std_dev   = .05;
}
else
{
      new_river->hinge->angle     = 0;
      new_river->hinge->x         = 0;
      new_river->hinge->y         = eh_grid_n_y(bathy_grid)/2;
      new_river->hinge->min_angle = -M_PI/2;
      new_river->hinge->max_angle =  M_PI/2;
      new_river->hinge->std_dev   = .1;
}

      sed_hydro_copy( new_river->data , hydro_data );
      sed_cube_add_river( cube , new_river );
   }

   for ( j=0 ; j<n_steps ; j++ )
   {
      sed_cube_avulse_all_rivers( cube );
/*
      sed_merge_all_rivers ( cube , eps );
      sed_split_all_rivers( cube , alpha );
*/

      deposit_sediment_at_river_mouth( cube );
   }

   {
      Sed_property p           = sed_property_new( "grain" );
      Sed_property_file sed_fp = sed_property_file_new( "test.grain" , p , NULL );

      sed_property_file_write( sed_fp , cube );

      sed_property_file_destroy( sed_fp );
      sed_property_destroy     ( p      );
   }

   {
      Sed_measurement x = sed_measurement_new( "depth" );
      Sed_tripod met_fp = sed_tripod_new( "test.depth" , x , NULL );

      sed_tripod_write( met_fp , cube );

      sed_tripod_destroy( met_fp );
      sed_measurement_destroy( x );
   }

   sed_sediment_unset_env();

   return 0;
}

void deposit_sediment_helper( Sed_river *this_river , Sed_cube c );

void deposit_sediment_at_river_mouth( Sed_cube c )
{
   g_list_foreach( sed_cube_river_list(c) , (GFunc)&deposit_sediment_helper , c );
}

void deposit_sediment_helper( Sed_river *this_river , Sed_cube c )
{
   int i, j;
   Sed_cell deposit_cell;

   {
      gssize n;
      gssize len = sed_sediment_env_size();
      double* f = eh_new( double , len );

      f[0] = eh_get_fuzzy_dbl( 0 , 1 );
      for ( n=1 ; n<len ; n++ )
         f[n] = ( 1.-f[0] ) / ( (double)len - 1. );

      deposit_cell = sed_cell_new_sized( len , 1. , f );

      eh_free( f );
   }

   this_river = sed_cube_find_river_mouth( c , this_river );

   i = this_river->x_ind;
   j = this_river->y_ind;

   if ( sed_cube_is_in_domain( c , i , j ) )
      sed_column_add_cell( sed_cube_col_ij(c,i,j) , deposit_cell );

   sed_cell_destroy( deposit_cell );
}

void sed_merge_all_rivers( Sed_cube c , double eps )
{
   sed_cube_set_river_list( c , merge_rivers( sed_cube_river_list(c) , eps ) );
}

GList *merge_rivers( GList *rivers , double eps )
{
   double d_theta;
   Sed_river *this_river, *last_river;
   GList *this_link, *last_link;

   rivers = g_list_sort( rivers , (GCompareFunc)&sort_rivers_by_angle );

   for ( this_link=rivers->next ; this_link ; this_link=this_link->next )
   {
      last_link  = this_link->prev;

      this_river = this_link->data;
      last_river = last_link->data;

      d_theta = this_river->hinge->angle
              - last_river->hinge->angle;
      if ( d_theta < eps )
      {
         if ( sort_rivers_by_discharge( this_river , last_river ) > 0 )
         {
            combine_river_discharge( this_river , last_river );
            rivers = g_list_remove_link( rivers , last_link );
         }
         else
         {
            combine_river_discharge( last_river , this_river );
            rivers = g_list_remove_link( rivers , this_link );
         }
      }
   }
      
   return rivers;
}

int sort_rivers_by_angle( Sed_river *r_1 , Sed_river *r_2 )
{
   if ( r_1->hinge->angle > r_2->hinge->angle )
      return 1;
   else if ( r_1->hinge->angle < r_2->hinge->angle )
      return -1;
   else
      return 0;
}

int sort_rivers_by_discharge( Sed_river *r_1 , Sed_river *r_2 )
{
   double q_1 = sed_hydro_water_flux(r_1->data);
   double q_2 = sed_hydro_water_flux(r_2->data);

   if ( q_1 > q_2 )
      return 1;
   else if ( q_1 < q_2 )
      return -1;
   else
      return 0;
}

int sort_streams_by_id( stream_st *s1 , stream_st *s2 )
{
   if ( s1->id > s2->id )
      return 1;
   else if ( s1->id < s2->id )
      return -1;
   else
      return 0;
}

void combine_river_discharge( Sed_river *r_1 , Sed_river *r_2 )
{
   Sed_hydro rec_1;
   Sed_hydro rec_2;
   double total_q;

   eh_require( r_1!=NULL );
   eh_require( r_2!=NULL );

   rec_1 = r_1->data;
   rec_2 = r_2->data;

   total_q = sed_hydro_water_flux( rec_1 ) 
           + sed_hydro_water_flux( rec_2 );

   sed_hydro_set_width( rec_1 ,   total_q
                                / ( sed_hydro_velocity(rec_1)*sed_hydro_depth(rec_1) ) );

}

Sed_river *split_river_discharge( Sed_river *r_1 , Sed_river *r_2 )
{
   double q_1, q_2;
   double f=.25;
   Sed_river *new_river;
   Sed_hydro rec_1, rec_2, new_rec;

   eh_require( r_1!=NULL );
   eh_require( r_2!=NULL );

   rec_1 = r_1->data;
   rec_2 = r_2->data;

   q_1 = sed_hydro_water_flux( rec_1 );
   q_2 = sed_hydro_water_flux( rec_2 );

   new_river = sed_dup_river( r_1 );

   new_river->hinge->angle = ( r_1->hinge->angle + r_2->hinge->angle ) / 2.;

   new_rec = new_river->data;

   sed_hydro_set_velocity( new_rec , .5*(   sed_hydro_velocity(rec_1)
                                          + sed_hydro_velocity(rec_2) ) );
   sed_hydro_set_depth   ( new_rec , .5*(   sed_hydro_depth(rec_1)
                                          + sed_hydro_depth(rec_2) ) );
   sed_hydro_set_bedload ( new_rec , .5*(   sed_hydro_bedload(rec_1)
                                          + sed_hydro_bedload(rec_2) ) );
   sed_hydro_set_width   ( new_rec , f*( q_1+q_2 )
                                     / (   sed_hydro_velocity( new_rec )
                                         * sed_hydro_depth   ( new_rec ) ) );

   sed_hydro_set_width( rec_1 , sed_hydro_width(rec_1)*(1.-f) );
   sed_hydro_set_width( rec_2 , sed_hydro_width(rec_2)*(1.-f) );

   return new_river;
}

void sed_split_all_rivers( Sed_cube c , double alpha )
{
   sed_cube_set_river_list( c , create_rivers( sed_cube_river_list(c) , alpha ) );
}

GList *create_rivers( GList *rivers , double alpha )
{
   double d_theta;
   GList *this_link, *last_link;
   Sed_river *left_wall, *right_wall;
   Sed_river *this_river, *last_river, *new_river;

   rivers = g_list_sort( rivers , (GCompareFunc)&sort_rivers_by_angle );

   left_wall  = sed_dup_river( g_list_first( rivers )->data );
   left_wall->hinge->angle  = M_PI;
   sed_hydro_set_width( left_wall->data , 0. );

   right_wall  = sed_dup_river( g_list_last( rivers )->data );
   right_wall->hinge->angle  = 0.;
   sed_hydro_set_width( right_wall->data , 0. );

   rivers = g_list_insert_sorted( rivers     ,
                                  left_wall  ,
                                  (GCompareFunc)&sort_rivers_by_angle );
   rivers = g_list_insert_sorted( rivers     ,
                                  right_wall ,
                                  (GCompareFunc)&sort_rivers_by_angle );

   for ( this_link=rivers->next ; this_link ; this_link=this_link->next )
   {
      last_link = this_link->prev;

      this_river = this_link->data;
      last_river = last_link->data;

      d_theta = this_river->hinge->angle - last_river->hinge->angle;

      if ( d_theta > alpha )
      {

         new_river = split_river_discharge( this_river , last_river );

         rivers = g_list_insert_sorted( rivers    ,
                                        new_river ,
                                        (GCompareFunc)&sort_rivers_by_angle );
      }
   }
   
   rivers = g_list_remove( rivers , left_wall  );
   rivers = g_list_remove( rivers , right_wall );

   sed_destroy_river( left_wall  );
   sed_destroy_river( right_wall );

   return rivers;
}
/*
GList *print_streams( GList *streams )
{
   int i, j;
   int n_cols = 144, pos, last_pos, d_pos;
   stream_st left_wall = { -M_PI_2 , 0. , 0. , -1 };
   GList *cur, *last;

   streams = g_list_sort( streams , (GCompareFunc)&sort_streams_by_angle );
   streams = g_list_insert_sorted( streams , &left_wall , (GCompareFunc)&sort_streams_by_angle );

   last_pos = 0;
   for ( cur=streams->next ; cur ; cur=cur->next )
   {
      pos = (((stream_st*)(cur->data))->angle+M_PI_2)*n_cols/M_PI;
      d_pos = pos-last_pos;
      for ( j=0 ; j<d_pos-1 ; j++ )
         fprintf(stdout," ");
      fprintf(stdout,"%d",((stream_st*)(cur->data))->id);
      last_pos = pos;
   }

   d_pos = n_cols-last_pos;
   for ( j=0 ; j<d_pos-1 ; j++ )
      fprintf(stdout," ");
   fprintf(stdout,"|\n");

   streams = g_list_remove( streams , &left_wall );

   return streams;
}

GList *print_streams_as_table( GList *streams )
{
   int i, j;
   int n_cols = 144, pos, last_pos, d_pos;
   stream_st left_wall = { -M_PI_2 , 0. , 0. , -1 };
   GList *cur, *last;

   streams = g_list_sort( streams , (GCompareFunc)&sort_streams_by_id );

   last_pos = 0;
   for ( cur=streams ; cur->next ; cur=cur->next )
      fprintf(stdout,"%f, %f, ",((stream_st*)(cur->data))->angle,((stream_st*)(cur->data))->size);
   fprintf(stdout,"%f, %f\n",((stream_st*)(cur->data))->angle,((stream_st*)(cur->data))->size);

   return streams;
}
*/
