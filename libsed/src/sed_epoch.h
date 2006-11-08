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

#if !defined(SED_EPOCH_H)
# define SED_EPOCH_H

// Name : Epoch -- a sedflux portion of time
//
// Synopsis :
//
// typedef           Epoch;
//
// Epoch*            epoch_create_epoch               ( void );
// void              epoch_destroy_epoch              ( Epoch *e );
// int               epoch_set_epoch_name             ( Epoch *e,
//                                                      char *name );
// int               epoch_set_epoch_duration         ( Epoch *e,
//                                                      double duration );
// int               epoch_set_epoch_time_step        ( Epoch *e,
//                                                      double time_step );
// int               epoch_set_epoch_filename         ( Epoch *e,
//                                                      char *filename );
// char*             epoch_get_epoch_name             ( Epoch *e );
// double            epoch_get_epoch_duration         ( Epoch *e );
// double            epoch_get_epoch_time_step        ( Epoch *e );
// char*             epoch_get_epoch_filename         ( Epoch *e );

typedef struct
{
   char*   name;
   double  number;
   double  duration;
   double  time_step;
   char*   filename;
} Epoch;

Epoch*  epoch_create_epoch         ( void );
void    epoch_destroy_epoch        ( Epoch *e );
int     epoch_set_epoch_name       ( Epoch *e , char *name );
int     epoch_set_epoch_duration   ( Epoch *e , double duration );
int     epoch_set_epoch_time_step  ( Epoch *e , double time_step );
int     epoch_set_epoch_filename   ( Epoch *e , char *filename );
char*   epoch_get_epoch_name       ( Epoch *e );
double  epoch_get_epoch_duration   ( Epoch *e );
double  epoch_get_epoch_time_step  ( Epoch *e );
char*   epoch_get_epoch_filename   ( Epoch *e );

#endif /* sed_epoch.h */
