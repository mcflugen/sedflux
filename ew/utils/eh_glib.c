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

#include <eh_utils.h>

gsize eh_string_find_first_of(GString *s, char c)
{
   char *p = strchr(s->str,c);
   return (p==NULL)?G_NPOS:p-s->str;
}

gsize eh_string_find_last_of(GString *s, char c)
{
   char *p = strrchr(s->str,c);
   return (p==NULL)?G_NPOS:p-s->str;
}

void eh_string_remove_chr(GString *s,char c)
{
   gsize n;
   while ( (n=eh_string_find_first_of(s,c))!=G_NPOS )
      g_string_erase(s,n,1);
}

void eh_string_remove_white_space(GString *s)
{
   char *p=(gchar*)G_WHITESPACE;
   gsize i, len=strlen(p);
   for ( i=0 ; i<len ; i++, p++ )
      eh_string_remove_chr(s,*p);
}

char *eh_string_c_str(GString *s)
{
   return s->str;
}

gsize eh_array_size(GArray *a)
{
   return a->len;
}
/*
GArray *eh_array_set_vals(GArray *a, guint index, gconstpointer data, guint len)
{
   GRealArray *array = (GRealArray*)a;
   g_array_maybe_expand( a , index+len );
   g_memcpy( array->data + array->elt_size*index , data , len*array->elt_size );
   array->len += len;
   return a;
}
*/

// removes the link containing data and frees the data.  there is no
// check to see that other links in the list point to data.  only the
// first link containing data is removed.
// list : a pointer to a GSList
// data : the data to be removed and freed
GSList *eh_slist_remove_and_free( GSList *list , gpointer data )
{
   list = g_slist_remove( list , data );
   eh_free( data );
   return list;
}

void eh_free_slist_data( gpointer data , gpointer user_data )
{
   eh_free( data );
}

