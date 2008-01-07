#include <eh_utils.h>

Eh_sequence*
eh_create_sequence( void )
{
   Eh_sequence* s = eh_new( Eh_sequence , 1 );
   s->len  = 0;
   s->t    = NULL;
   s->data = NULL;

   return s;
}

Eh_sequence*
eh_add_to_sequence( Eh_sequence* s , double t , gpointer data )
{
   eh_require( s!=NULL );

   s->len++;

   s->t    = g_renew( double   , s->t    , s->len );
   s->data = g_renew( gpointer , s->data , s->len );

   s->t[s->len-1] = t;
   s->data[s->len-1] = data;

   return s;
}

void
eh_destroy_sequence( Eh_sequence* s , gboolean free_mem )
{
   gssize i;
   if ( s )
   {
      if ( free_mem )
         for ( i=0 ; i<s->len ; i++ )
            eh_free( s->data[i] );
      eh_free( s->data );
      eh_free( s->t    );
      eh_free( s       );
   }
}

