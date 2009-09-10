#include <stdio.h>
#include <glib.h>
#include <utils/utils.h>
#include <sed/csdms.h>
#include <sed/sed_sedflux.h>

#if defined (OLD_WAY)

typedef struct
{
   Sed_epoch_queue q;
   Sed_cube        p;
   gchar*          file;
}
Sed_state;

gboolean sedflux_init         ( Sed_epoch_queue* q , Sed_cube* p , const gchar* init_file );
gboolean sedflux_finalize     ( Sed_epoch_queue  q , Sed_cube  p );
double*  sedflux_get_val_at   ( Sed_epoch_queue q , Sed_cube p , const gchar* val_s , gint* here , double now );

gboolean
sed_init_func( CSDMSComp* c , const gchar* file )
{
   gboolean success = FALSE;

   eh_require( c                    );
   eh_require( c->user_data == NULL );
   eh_require( file                 );

   if ( c && !c->user_data && file )
   {
      Sed_state* s = eh_new( Sed_state , 1 );

      s->q = NULL;
      s->p = NULL;

      success = sedflux_init( &(s->q) , &(s->p) , file );

      if ( success )
      {
         s->file      = g_strdup( file );
         c->user_data = s;
      }
      else
      {
         eh_free( s );
         c->user_data = NULL;
      }
   }

   eh_require( success==TRUE || success==FALSE );

   return success;
}

gboolean
sed_run_func( CSDMSComp* c , const gchar* val_s , gint* here , double now , double** vals )
{
   gboolean success = FALSE;

   eh_require( c                  );
   eh_require( c->user_data==NULL );

   if ( c && c->user_data )
   {
      Sed_state* s = (Sed_state*)(c->user_data);

      (*vals) = sedflux_get_val_at( s->q , s->p , val_s , here , now );

      success = (*vals)!=NULL;
   }

   eh_require( success==TRUE || success==FALSE );

   return success;
}

gboolean
sed_finalize_func( CSDMSComp* c )
{
   gboolean success = FALSE;

   eh_require( c                  );
   eh_require( c->user_data==NULL );

   if ( c && !c->user_data )
   {
      Sed_state* s = (Sed_state*)(c->user_data);

      eh_require( s->q );
      eh_require( s->p );

      success = sedflux_finalize( s->q ,  s->p );

      eh_free( s->file );
      eh_free( s       );

      c->user_data = NULL;
   }

   eh_require( success==TRUE || success==FALSE );

   return success;
}

gint
csdms_sample_prog( void )
{
   double*    z_vals = NULL;
   double*    d_vals = NULL;
   CSDMSComp* c      = csdms_comp_new ( );
   gint*      here   = NULL;
   double     now    = 100.;

   csdms_comp_set_irf ( c , sed_init_func , sed_run_func , sed_finalize_func );

   csdms_comp_init    ( c , "sedflux_init_file" );

   csdms_comp_get_val ( c , "Elevation"  , here , now      , &z_vals );
   csdms_comp_get_val ( c , "Grain Size" , here , now      , &d_vals );
   csdms_comp_get_val ( c , "Elevation"  , here , now + 5. , &z_vals );

   csdms_comp_finalize( c );

   c = csdms_comp_destroy( c );

   return EXIT_SUCCESS;
}

#endif
