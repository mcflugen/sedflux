#include "utils.h"

typedef struct 
{
  gint             size;
  gint             nnodes;
  gpointer         **nodes;
  GHashFunc        hash_func;
  GEqualFunc       key_equal_func;
  GDestroyNotify   key_destroy_func;
  GDestroyNotify   value_destroy_func;
} _GHashTable;

typedef struct
{
  guint          scope_id;
  gchar         *symbol;
  gpointer       value;
} GScannerKey;

static void
g_scanner_destroy_symbol_table_entry (gpointer _key,
                                      gpointer _value,
                                      gpointer _data)
{
  GScannerKey *key = _key;

  g_free (key->symbol);
  g_free (key);
}

static inline void
g_scanner_free_value (GTokenType     *token_p,
                      GTokenValue     *value_p)
{
  switch (*token_p)
    {
    case G_TOKEN_STRING:
    case G_TOKEN_IDENTIFIER:
    case G_TOKEN_IDENTIFIER_NULL:
    case G_TOKEN_COMMENT_SINGLE:
    case G_TOKEN_COMMENT_MULTI:
      g_free (value_p->v_string);
      break;

    default:
      break;
    }

  *token_p = G_TOKEN_NONE;
}

int main()
{
   Eh_key_file f;
   GScanner* s;

   eh_init_glib();
/*
   f = eh_key_file_scan( "basin_3d.init" );
   eh_key_file_destroy( f );
*/
   s = g_scanner_new( NULL );
eh_watch_ptr( s );
eh_watch_ptr( s->config );
eh_watch_ptr( s->symbol_table );
eh_watch_ptr( ((_GHashTable*)(s->symbol_table))->nodes );
   g_scanner_destroy( s );
eh_debug( "here" );

/*
  g_datalist_clear (&s->qdata);
  g_hash_table_foreach (s->symbol_table,
                        g_scanner_destroy_symbol_table_entry, NULL);
  g_hash_table_destroy (s->symbol_table);
  g_scanner_free_value (&s->token, &s->value);
  g_scanner_free_value (&s->next_token, &s->next_value);
  g_free (s->config);
  g_free (s->buffer);
  g_free (s);
*/

/*
   s = eh_open_scanner( "basin_3d.init" );
   eh_close_scanner( s );
*/

   eh_heap_dump( "heap_dump.txt" );

   return 0;
}

