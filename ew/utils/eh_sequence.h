#ifndef __EH_SEQUENCE_H__
#define __EH_SEQUENCE_H__

#include <glib.h>

/** A sequence of data
*/
typedef struct
{
   double*   t;    //< The 'time' value for each member of the sequence
   gpointer* data; //< A pointer to each member of the sequence
   gssize    len;  //< The number of members in the sequence
}
Eh_sequence;

Eh_sequence* eh_create_sequence ( void );
Eh_sequence* eh_add_to_sequence ( Eh_sequence* s , double t , gpointer data );
void         eh_destroy_sequence( Eh_sequence* s , gboolean free_mem );

#endif
