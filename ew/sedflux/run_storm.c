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

#define SED_STORM_PROC_NAME "storms"
#define EH_LOG_DOMAIN SED_STORM_PROC_NAME

#include <stdio.h>
#include "my_processes.h"
#include <utils/eh_rand.h>

#include "sedflux.h"

typedef struct
{
   Eh_input_val h;
   double       t;
}
User_storm_data;

GSList *get_equivalent_storm( GFunc    get_storm          ,
                              gpointer user_data          ,
                              double   n_days             ,
                              double   sig_event_fraction ,
                              gboolean average_non_events );
void storm_func_user( double *ans , User_storm_data* user_data );

double *get_wave_from_beaufort_scale_power_law( double beaufort_storm ,
                                                double *wave );
double get_wind_from_beaufort_scale( double beaufort_storm );
double get_wave_length_from_wind( double wind_speed_in_mps );
double get_wave_period_from_wind( double wind_speed_in_mps );
double get_wave_height_from_wind( double wind_speed_in_mps );

double *get_wave_from_beaufort_scale( double beaufort_storm , double *wave );
double get_height_from_beaufort_scale( double beaufort_storm );
double get_beaufort_scale_from_height( double wave_height );
double get_wave_length_from_height( double wave_height );
double get_wave_period_from_height( double wave_height_in_meters );

gboolean init_storm_data( Sed_process proc , Sed_cube prof , GError** error );

Sed_process_info
run_storm( Sed_process proc , Sed_cube prof )
{
   Storm_t*         data       = sed_process_user_data(proc);
   Sed_process_info info       = SED_EMPTY_INFO;
   GSList*          storm_list = NULL;
   double           n_days;
   double           time_step;
   double           start_time;

   if ( sed_process_run_count(proc)==0 )
      init_storm_data( proc , prof , NULL );

   // average the requested number of days.
   start_time      = data->last_time;
   time_step       = sed_cube_age_in_years( prof ) - data->last_time;
   data->last_time = sed_cube_age_in_years( prof );
   n_days          = time_step*S_DAYS_PER_YEAR;

   if ( time_step > 1e-6 )
   {
      if ( TRUE )
      {
         User_storm_data user_data;

         user_data.h = data->wave_height;
         user_data.t = start_time;

         storm_list = get_equivalent_storm( (GFunc)storm_func_user ,
                                            &user_data             ,
                                            n_days                 ,
                                            data->fraction         ,
                                            data->average_non_events );
      }
   }
   else
      return info;

   sed_cube_set_storm_list( prof , storm_list );

   {
      GSList*         this_link;
      Sed_ocean_storm this_storm;
      gint            n                 = g_slist_length( storm_list );
      gint            i                 = 0;
      double          this_time         = start_time;
      double          significant_storm = G_MINDOUBLE;

      for ( this_link=storm_list ; this_link ; this_link=this_link->next )
      {
         this_storm = this_link->data;

         eh_dbl_set_max( significant_storm , sed_ocean_storm_wave_height( this_storm ) );

         eh_message( "time        : %f" , this_time                               );
         eh_message( "time step   : %f" , sed_ocean_storm_duration(this_storm)    );
         eh_message( "storm number: %d" , i++                                     );
         eh_message( "total number: %d" , n                                       );
         eh_message( "wave height : %f" , sed_ocean_storm_wave_height(this_storm) );
         eh_message( "wave period : %f" , sed_ocean_storm_wave_period(this_storm) );
         eh_message( "wave length : %f" , sed_ocean_storm_wave_length(this_storm) );

         this_time  += sed_ocean_storm_duration(this_storm)*S_YEARS_PER_DAY;

      }

      if ( n>0 )
      {
         significant_storm = get_beaufort_scale_from_height( significant_storm );
         sed_cube_set_storm( prof , significant_storm );
      }
      else
         sed_cube_set_storm( prof , 0. );

      eh_message( "storm value : %f" , sed_cube_storm( prof ) );

   }

   return info;
}

#include <time.h>

/** \name Input parameters for ocean storm module
@{
*/
/// Average length of an ocean storm in days
#define STORM_KEY_STORM_LENGTH    "average length of a storm"
/// Wave height (in meters) of the 100y ocean storm
#define STORM_KEY_STORM_MAGNITUDE "wave height of 100 year storm"
/// Variance (in meters) of 100y storm wave height
#define STORM_KEY_STORM_VARIANCE  "variance of 100 year storm"
/// Scale parameter for ocean storm wave height (Weibull distribution)
#define STORM_KEY_SCALE_PARAMETER "scale parameter for pdf"
/// Shape parameter for ocean storm wave height (Weibull distribution)
#define STORM_KEY_SHAPE_PARAMETER "shape parameter for pdf"
/// Give wave heights from a file, or normal, uniform, or user defined
/// distribution
#define STORM_KEY_WAVE_HEIGHT     "wave height"
/// Seed for the ocean storm random number generator
#define STORM_KEY_SEED            "seed for random number generator"
/// Fraction of days to model
#define STORM_KEY_FRACTION        "fraction of events to model"
/// Should the calm periods between storms be averaged to forgotten
#define STORM_KEY_NON_EVENTS      "average non-events?"
/*@}*/

static gchar* storm_req_labels[] =
{
   STORM_KEY_FRACTION    ,
   STORM_KEY_NON_EVENTS  ,
   STORM_KEY_WAVE_HEIGHT ,
   STORM_KEY_SEED        ,
   NULL
};

/** Initialize a storm process.

@param p     A pointer to Sed_process
@param tab   A pointer to a Symbol_table.
@param error       A GError to indicate user-input error

@return TRUE if no problems were encountered.  FALSE otherwise.
*/
gboolean
init_storm( Sed_process p , Eh_symbol_table tab , GError** error )
{
   Storm_t* data    = sed_process_new_user_data( p , Storm_t );
   GError*  tmp_err = NULL;
   gchar**  err_s   = NULL;
   gboolean is_ok   = TRUE;

   eh_return_val_if_fail( error==NULL || *error==NULL , FALSE );

   data->rand               = NULL;
   data->last_time          = 0.;

   eh_symbol_table_require_labels( tab , storm_req_labels , &tmp_err );

   if ( !tmp_err )
   {
      data->wave_height        = eh_symbol_table_input_value( tab , STORM_KEY_WAVE_HEIGHT , &tmp_err );
      data->fraction           = eh_symbol_table_dbl_value  ( tab , STORM_KEY_FRACTION   );
      data->average_non_events = eh_symbol_table_bool_value ( tab , STORM_KEY_NON_EVENTS );
      data->rand_seed          = eh_symbol_table_int_value  ( tab , STORM_KEY_SEED       );

      eh_check_to_s( data->fraction>=0. , "Event fraction between 0 and 1" , &err_s );
      eh_check_to_s( data->fraction<=1. , "Event fraction between 0 and 1" , &err_s );

      if ( !tmp_err && err_s )
         eh_set_error_strv( &tmp_err , SEDFLUX_ERROR , SEDFLUX_ERROR_BAD_PARAM , err_s );
   }

   if ( tmp_err )
   {
      g_propagate_error( error , tmp_err );
      is_ok = FALSE;
   }

   return is_ok;
}

gboolean
init_storm_data( Sed_process proc , Sed_cube prof , GError** error )
{
   Storm_t* data = sed_process_user_data( proc );

   if ( data )
   {
      if ( data->rand_seed>0 ) data->rand = g_rand_new_with_seed( data->rand_seed );
      else                     data->rand = g_rand_new( );
      data->last_time = sed_cube_age_in_years( prof );
   }

   return TRUE;
}

gboolean
destroy_storm( Sed_process p )
{
   if ( p )
   {
      Storm_t* data = sed_process_user_data( p );
      
      if ( data )
      {
         g_rand_free         ( data->rand        );
         eh_input_val_destroy( data->wave_height );
         eh_free             ( data              );
      }
   }

   return TRUE;
}

/** Calculate the magnitude of a storm.

The magnitude of a storm is calculated based on the average length of a storm,
the average magnitude of a storm, and the magnitude of the previous storm.  
Here a strom is merely a stretch of similar weather.  It need not be
particularly unpleasant.  The magnitude of the storm is drawn from a power
law pdf that is fixed by the average storm magnitude.  The average storm length
determines the probability that this storm will be the same as the last 
(yesterday's) storm.

\param rand          A GRand
\param storm_length  The average length of a storm.
\param average_storm The average magnitude of a storm.
\param variance      The variance of storm PDF
\param last_storm    The magnitude of the last storm.

\return The magnitude of the next storm.
*/
double
storm( GRand *rand , double storm_length, double average_storm, double variance , double last_storm)
{
   double alpha,f,a;
   static long seed[1];

   alpha=last_storm;

   f=1.-1./storm_length;
   a=exp(-1/average_storm);

   if ( eh_ran1(seed) > f )
      alpha = eh_max_log_normal( rand , average_storm , variance , 1./365./1. );
 
   return alpha;
}

void set_ocean_storm_wave( Sed_ocean_storm s , gpointer user_data );
Sed_ocean_storm average_storms( GSList *storms );
void free_link_data( gpointer data , gpointer user_data );
gint cmp_storm_size( Sed_ocean_storm a , Sed_ocean_storm b );
gint cmp_storm_time( Sed_ocean_storm a , Sed_ocean_storm b );
void print_ocean_storm_list( GSList *list );
void print_ocean_storm( Sed_ocean_storm this_storm , gpointer user_data );

struct weibull_storm_data
{
   double sigma;
   double mu;
   GRand* rand;
};

void
storm_func_weibull( double* ans , struct weibull_storm_data* user_data )
{
   double variance      = user_data->sigma;
   double average_storm = user_data->mu;
   GRand* rand          = user_data->rand;

   *ans = eh_rand_max_weibull( rand          ,
                               average_storm ,
                               variance      ,
                               1./365. );
}

void
storm_func_user( double *ans , User_storm_data* user_data )
{
   Eh_input_val wave_height = user_data->h;
   double time              = user_data->t;
   *ans = eh_input_val_eval( wave_height , time );
}

GSList *get_equivalent_storm( GFunc get_storm           ,
                              gpointer user_data        ,
                              double n_days             ,
                              double sig_event_fraction ,
                              gboolean average_non_events )
{
   GSList *all_storms = NULL;
   GSList *big_storms = NULL, *little_storms;
   GSList *calm_period = NULL;
   GSList *final_list = NULL;
   GSList *this_link, *prev_link;
   gssize n_sig_events;
   Sed_ocean_storm this_storm, prev_storm;
   gssize i;
   gboolean sort_high_to_low;
   double storm_value;

   if ( sig_event_fraction < 0 )
      sort_high_to_low = FALSE;
   else
      sort_high_to_low = TRUE;

   sig_event_fraction = fabs(sig_event_fraction);

   n_sig_events = n_days*sig_event_fraction;

   {
      double n_events;
      double fraction = modf( n_days*sig_event_fraction , &n_events );

      if ( g_random_double_range( 0 , 1. ) < fraction )
         n_sig_events++;
   }

   //---
   // Create a list of ocean storm events for every day of the period.
   //---
   for ( i=0 ; i<n_days ; i++ )
   {
      this_storm = sed_ocean_storm_new();

      sed_ocean_storm_set_index   ( this_storm , i );
      sed_ocean_storm_set_duration( this_storm , 1. );

      get_storm( &storm_value , user_data );

      sed_ocean_storm_set_val( this_storm , storm_value );

      all_storms = g_slist_prepend( all_storms , this_storm );
   }

   //---
   // Sort the storms from largest to smallest.  We'll model the top storms
   // and average the rest.
   //---
   all_storms = g_slist_sort( all_storms , (GCompareFunc)cmp_storm_size );
   if ( sort_high_to_low )
      all_storms = g_slist_reverse( all_storms );

   //---
   // Break the sorted list into two.  One list will contain the big storms,
   // while the other contains the little storms.
   //---
   little_storms = g_slist_nth( all_storms , n_sig_events );
   if ( n_sig_events>0 )
   {
      big_storms = all_storms;
      g_slist_nth( big_storms , n_sig_events-1 )->next = NULL;
   }
   else
      big_storms = NULL;

   //---
   // Average the storms for each of the calm periods.  We are given the
   // option to either average the non-events or to ignore them completely.
   //---
   if ( average_non_events )
   {

      //---
      // Sort the list of small storms by time.  Break this list into smaller
      // lists of consecutive storms.  There will be some breaks because the
      // largest storms were removed.
      //---
      little_storms = g_slist_sort( little_storms ,
                                    (GCompareFunc)cmp_storm_time );

      calm_period = g_slist_append( calm_period , little_storms );
      prev_link = little_storms;
      for ( this_link = little_storms->next ;
            this_link ;
            this_link = this_link->next )
      {
         this_storm = this_link->data;
         prev_storm = prev_link->data;

         if ( sed_ocean_storm_index(prev_storm)+1 != sed_ocean_storm_index(this_storm) )
         {
            prev_link->next = NULL;
            calm_period = g_slist_append( calm_period , this_link );
         }

         prev_link = this_link;
      }

      for ( this_link=calm_period ; this_link ; this_link=this_link->next )
      {
         this_storm = average_storms( this_link->data );
         final_list = g_slist_append( final_list , this_storm );

         g_slist_foreach( this_link->data , &free_link_data , NULL );
         g_slist_free( this_link->data );
      }
      g_slist_free( calm_period );

   }
   else
   {
      g_slist_foreach( little_storms , &free_link_data , NULL );
      g_slist_free( little_storms );
      little_storms = NULL;
   }

   final_list = g_slist_concat( final_list , big_storms );
   final_list = g_slist_sort( final_list , (GCompareFunc)cmp_storm_time );
   g_slist_foreach( final_list , (GFunc)&set_ocean_storm_wave , NULL );

   return final_list;
}

void set_ocean_storm_wave( Sed_ocean_storm s , gpointer user_data )
{
   double height = sed_ocean_storm_val(s);
   double freq   = 2.*G_PI/get_wave_period_from_height( height );
   double number = pow(freq,2)/sed_gravity();
   Sed_wave new_wave = sed_wave_new( height , number , freq );

   sed_ocean_storm_set_wave( s , new_wave );
}

void print_ocean_storm_list( GSList *list )
{
   GSList *this_link;

   for ( this_link=list ; this_link ; this_link=this_link->next )
      sed_ocean_storm_fprint( stderr , (Sed_ocean_storm)(this_link->data) );
}

Sed_ocean_storm average_storms( GSList *storm_list )
{
   Sed_ocean_storm this_storm = NULL;

   if ( storm_list )
   {
      GSList *this_link;
      double duration  = 0.;
      double storm_val = 0.;
      gssize time_ind;

      for ( this_link = storm_list ; this_link ; this_link = this_link->next )
      {
         this_storm = this_link->data;

         duration  += sed_ocean_storm_duration(this_storm);
         storm_val += pow(sed_ocean_storm_val(this_storm),2.5)*sed_ocean_storm_duration(this_storm);
         time_ind   = sed_ocean_storm_index(this_storm);
      }
      storm_val /= duration;

      storm_val = pow( storm_val , 0.4 );

      this_storm = sed_ocean_storm_new();
   
      sed_ocean_storm_set_val     ( this_storm , storm_val );
      sed_ocean_storm_set_duration( this_storm , duration  );
      sed_ocean_storm_set_index   ( this_storm , time_ind  );
   }

   return this_storm;
}

void free_link_data( gpointer data , gpointer user_data )
{
   eh_free( data );
}

gint cmp_storm_size( Sed_ocean_storm a , Sed_ocean_storm b )
{
   double val_a = sed_ocean_storm_val( a );
   double val_b = sed_ocean_storm_val( b );

   if ( val_a < val_b )
      return -1;
   else if ( val_a > val_b )
      return 1;
   else
      return 0;
}

gint cmp_storm_time( Sed_ocean_storm a , Sed_ocean_storm b )
{
   gssize val_a = sed_ocean_storm_index(a);
   gssize val_b = sed_ocean_storm_index(b);

   if ( val_a < val_b )
      return -1;
   else if ( val_a > val_b )
      return 1;
   else
      return 0;
}

/** Get wave characteristics from a beaufort scale value.

Calculate the wave height, period, and velocity given a Beaufort scale value.
The wave characteristics are calculated using some ad hoc empirical relations.

   wave_height = exp( 0.252 beaufort_value ) - 1.

   wave_period = 10^(beaufort_value/4.)

   wave_length = 10*wave_height

For a non-NULL wave pointer, care must be taken to assure that there is enough
memory to hold the three wave characteristic values.  These values are stored
as wave height, wave period, and wave length.  If wave is NULL, a newly
allocated array is used and returned.

@param beaufort_storm Storm magnitude as expressed as a beaufort value.
@param wave           Array of wave characteristics.

@return A pointer to the wave characteristics.
*/
double *get_wave_from_beaufort_scale_old( double beaufort_storm , double *wave )
{
   double wave_height, wave_period, wave_length;
   double beaufort_tab[2][18] = {
      {0, 1, 2, 3,4,5,6,7,8,9,10,11  ,12,13,14,15,16,17},
      {0,.1,.3,1,1.5,2.5,4,5.5,7.5,10,12.5,16,18,20,22,24,26,28} };

   eh_require( beaufort_storm>0   );
   eh_require( beaufort_storm<=17 );

   if ( !wave )
      wave = eh_new( double , 3 );

   interpolate(  beaufort_tab[0] ,  beaufort_tab[1] , 18 ,
                &beaufort_storm  , &wave_height     , 1 );
//   wave_height = exp(0.252*beaufort_storm) - 1.;
/*
   do
   {
      wave_period = eh_get_fuzzy_dbl_norm( pow(10,beaufort_storm/4.) , pow(10,beaufort_storm/4.)/10. );
   }
   while ( wave_period<0. );
*/
   wave_period = pow(10,beaufort_storm/10);
   wave_length = 25.*wave_height;
/*
   wave_length = 5.*sed_gravity()*pow(wave[2]*sinh(M_PI/10)/M_PI,2.));
*/

   wave[0] = wave_height;
   wave[1] = wave_period;
   wave[2] = wave_length;

   return wave;
}

/** Get wave characteristics from a beaufort scale value.

Calculate the wave height, period, and length given a Beaufort scale value.
The wave characteristics are calculated using power-law relations based on
a fully developed sea.

For a non-NULL wave pointer, care must be taken to assure that there is enough
memory to hold the three wave characteristic values.  These values are stored
as wave height, wave period, and wave length.  If wave is NULL, a newly
allocated array is used and returned.

@param beaufort_storm Storm magnitude as expressed as a beaufort value.
@param wave           Array of wave characteristics.

@return A pointer to the wave characteristics.

@see get_wave_period_from_wind , get_wave_height_from_wind ,
     get_wave_length_from_wind , get_wind_from_beaufort_storm .
*/
double *get_wave_from_beaufort_scale_power_law( double beaufort_storm ,
                                                double *wave )
{
   double wind_speed;

   eh_require( beaufort_storm>=0  );
   eh_require( beaufort_storm<=17 );

   if ( !wave )
      wave = eh_new( double , 3 );

   wind_speed = get_wind_from_beaufort_scale( beaufort_storm );

   wave[0] = get_wave_height_from_wind( wind_speed );
   wave[1] = get_wave_period_from_wind( wind_speed );
   wave[2] = get_wave_length_from_wind( wind_speed );

   return wave;
}

/** Get wind speed from Beaufort storm value.

Get the wind speed (in meters per second) from a Beaufort storm value.
Fractional storm values are allowed.  In such a case,  the wind speed will be
estimated by linear interpolation.  

@param beaufort_storm A Beaufort storm value.

@return The wind speed (m/s).

@see get_wave_length_from_wind , see get_wave_period_from_wind ,
     get_wave_height_from_wind .
*/
double get_wind_from_beaufort_scale( double beaufort_storm )
{
   double wind;
   double beaufort_tab[2][18] = {
      {0,1,2,3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 16, 17},
      {0,1,4,7,11,17,22,28,34,41,48,56,64,72,81,90,100,109} };

   eh_require( beaufort_storm>=0  );
   eh_require( beaufort_storm<=17 );

   interpolate(  beaufort_tab[0] ,  beaufort_tab[1] , 18 ,
                &beaufort_storm  , &wind            , 1 );

   return wind*S_MPS_PER_KNOT;
}

/** Get wave characteristics from a beaufort scale value.

Calculate the wave height, period, and length given a Beaufort scale value.
The wave heights are taken from Beaufort strom tables.  Deep-water wave
equations are used to relate wave height to wave length and period.

For a non-NULL wave pointer, care must be taken to assure that there is enough
memory to hold the three wave characteristic values.  These values are stored
as wave height, wave period, and wave length.  If wave is NULL, a newly
allocated array is used and returned.

@param beaufort_storm Storm magnitude as expressed as a beaufort value.
@param wave           Array of wave characteristics.

@return A pointer to the wave characteristics.

@see get_wave_period_from_height , get_wave_length_from_height ,
     get_height_from_beaufort_storm .
*/
double *get_wave_from_beaufort_scale( double beaufort_storm , double *wave )
{
   double wave_height;

   eh_require( beaufort_storm>=0  );
   eh_require( beaufort_storm<=17 );

   if ( !wave )
      wave = eh_new( double , 3 );

   wave_height = get_height_from_beaufort_scale( beaufort_storm );

   wave[0] = wave_height;
   wave[1] = get_wave_period_from_height( wave_height );
   wave[2] = get_wave_length_from_height( wave_height );

   return wave;
}

/** Get probable wave height from Beaufort storm value.

Get the probable wave height (in meters) from a Beaufort storm value.
Fractional storm values are allowed.  In such a case,  the wind speed will be
estimated by linear interpolation.  

@param beaufort_storm A Beaufort storm value.

@return The probable wave height (m).

@see get_wave_length_from_height , get_wave_period_from_height .
*/
double get_height_from_beaufort_scale( double beaufort_storm )
{
   double wave_height;
   double beaufort_tab[2][18] = {
      {0, 1, 2, 3,4,5,6,7,8,9,10,11  ,12,13,14,15,16,17},
      {0,.1,.2,.6,1,2,3,4,6,7,9 ,11.5,14,16,18,20,22,24} };

   eh_require( beaufort_storm>=0  );
   eh_require( beaufort_storm<=17 );

   interpolate(  beaufort_tab[0] ,  beaufort_tab[1] , 18 ,
                &beaufort_storm  , &wave_height     , 1 );

   return wave_height;
}

double get_beaufort_scale_from_height( double wave_height )
{
   double beaufort_storm;
   double beaufort_tab[2][18] = {
      {0, 1, 2, 3,4,5,6,7,8,9,10,11  ,12,13,14,15,16,17},
      {0,.1,.2,.6,1,2,3,4,6,7,9 ,11.5,14,16,18,20,22,24} };

   eh_require( wave_height>=0  );

   if ( wave_height>beaufort_tab[1][17] )
      beaufort_storm = beaufort_tab[0][17];
   else
      interpolate(  beaufort_tab[1] ,  beaufort_tab[0] , 18 ,
                   &wave_height     , &beaufort_storm  , 1 );

   return beaufort_storm;
}

/** Get wave length from wind height.

Assume a wave steepness of 1/7 to calculate the length of a wave from
its height.  When wave steepness exceeds 1/7, waves become unstable and begin
to break.

@param wave_height Wave height

@return Wave length in the same units as the input wave height.

@see get_wave_period_from_height , get_wind_from_beaufort_storm .
*/
double get_wave_length_from_height( double wave_height )
{
   return wave_height*7;
}

/** Get wave period from wind height.

@param wave_height_in_meters Wave height (m)

@return Wave period (s)

@see get_wave_length_from_height , get_wind_from_beaufort_storm .
*/
double get_wave_period_from_height( double wave_height_in_meters )
{
//   return sqrt(get_wave_length_from_height( wave_height_in_meters ))/1.25;
   return pow(wave_height_in_meters/.00195,1/2.5)*.245;

// NOTE: Multiplying by 2 here matches Gulf of Lions data.  May not be correct
// everywhere
//   return pow(wave_height_in_meters/.00195,1/2.5)*.245*2;
}

/** Get wave length from wind speed.

@param wind_speed_in_mps Wind speed (m/s)

@return Wave length (m)

@see get_wave_period_from_wind , get_wave_height_from_wind ,
     get_wind_from_beaufort_storm .
*/
double get_wave_length_from_wind( double wind_speed_in_mps )
{
   return .3203*pow(wind_speed_in_mps,2);
}

/** Get wave period from wind speed.

@param wind_speed_in_mps Wind speed (m/s)

@return Wave period (s)

@see get_wave_length_from_wind , get_wave_height_from_wind ,
     get_wind_from_beaufort_storm .
*/
double get_wave_period_from_wind( double wind_speed_in_mps )
{
   return .5481*wind_speed_in_mps;
}

/** Get wave height from wind speed.

@param wind_speed_in_mps Wind speed (m/s)

@return Wave height (m)

@see get_wave_length_from_wind , get_wave_period_from_wind ,
     get_wind_from_beaufort_storm .
*/
double get_wave_height_from_wind( double wind_speed_in_mps )
{
   return .004449*pow(wind_speed_in_mps,2.5);
}

