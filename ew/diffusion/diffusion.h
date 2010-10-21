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

#ifndef _DIFFUSION_H_
# define _DIFFUSION_H_

#define DIFFUSION_OPT_FILL  (1<<0)
#define DIFFUSION_OPT_LAND  (1<<1)
#define DIFFUSION_OPT_WATER (1<<2)

# include <utils/utils.h>
# include <sed/sed_sedflux.h>
# include <glib.h>

G_BEGIN_DECLS

Sed_cell *diffuse_sediment( Sed_cube prof , double k_max ,
                            double skin_depth , double dt    ,
                            int options );
Sed_cell *diffuse_sediment_2( Sed_cube prof    , double k_cross_max ,
                              double k_long_max , double skin_depth  ,
                              double dt         , int options );

G_END_DECLS

#endif
