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
#include <glib.h>
#include "utils.h"

#include "sed_epoch.h"

Epoch *epoch_create_epoch(void)
{
   Epoch *e = eh_new(Epoch,1);
   return e;
}

void epoch_destroy_epoch(Epoch *e)
{
   eh_free(e);
   return;
}

#include <string.h>

int epoch_set_epoch_name(Epoch *e,char *name)
{
   strcpy(e->name,name);
   return 0;
}

int epoch_set_epoch_duration(Epoch *e,double duration)
{
   e->duration = duration;
   return 0;
}

int epoch_set_epoch_time_step(Epoch *e,double time_step)
{
   e->time_step = time_step;
   return 0;
}

int epoch_set_epoch_filename(Epoch *e,char *filename)
{
   strcpy(e->filename,filename);
   return 0;
}

char *epoch_get_epoch_name(Epoch *e)
{
   return (e->name);
}

double epoch_get_epoch_duration(Epoch *e)
{
   return (e->duration);
}

double epoch_get_epoch_time_step(Epoch *e)
{
   return (e->time_step);
}

char *epoch_get_epoch_filename(Epoch *e)
{
   return (e->filename);
}

