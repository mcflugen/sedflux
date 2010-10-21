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

#ifndef __EH_GLIB_H__
#define __EH_GLIB_H__

#ifdef __cplusplus
extern "C" {
#endif
// #define           EH_NPOS                          G_MAXLONG
// #define           EH_WHITESPACE                    " \t\n"
//
// int               eh_string_find_first_of          ( GString*,
//                                                      char );
// int               eh_string_last_first_of          ( GString*,
//                                                      char );
// void              eh_string_remove_chr             ( GString*,
//                                                      char );
// void              eh_string_remove_white_space     ( GString* );
// char*             eh_string_c_str                  ( GString* );
// GSList*           eh_slist_remove_and_free         ( GSList*,
//                                                      gpointer );

#define G_NPOS G_MAXLONG
#define G_WHITESPACE " \t\n"

#define g_array_set_val(a,i,v) g_array_set_vals( a , i , &v , 1 )

// find the first occurance of a character in a string.
// s            : the search string.
// c            : the character to search for.
// return value : the position of the first occurance of the character.
//              : EH_NPOS if the character is not found.
gsize eh_string_find_first_of( GString *s , char c );

// find the last occurance of a character in a string.
// s            : the search string.
// c            : the character to search for.
// return value : the position of the last occurance of the character.
//              : EH_NPOS if the character is not found.
gsize eh_string_last_first_of( GString *s , char c );

// remove all occurances of a character from a string.
// s            : the string to alter
// c            : the character to remove.
// return value : nothing.
void eh_string_remove_chr( GString *s , char c );

// remove all white spaces (as given in EH_WHITESPACE) from a string.
// s            : the string to alter.
// return value : nothing.
void eh_string_remove_white_space( GString *s );

// the c-style (NULL terminated) string representation of a GString.
// s            : a pointer to a GString.
// return value : the c-style string.
char *eh_string_c_str(GString*);

// remove a list element from a singly-liked list and free the data of that element.
// l            : a pointer to a singly-liked list.
// p            : a pointer to the data part of the list element to remove.
// return value : the list with the element removed.
GSList *eh_slist_remove_and_free( GSList *l , gpointer p );

void eh_free_slist_data( gpointer data , gpointer user_data );
#ifdef __cplusplus
}
#endif

#endif
