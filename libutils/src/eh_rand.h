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

#if !defined( EH_RAND_H )
#define EH_RAND_H

#include <math.h>
#include <glib.h>

// double            eh_ran0                          ( long* );
// double            eh_ran1                          ( long* );
// double            eh_ran2                          ( long* );
// double            eh_ran3                          ( long* );
// double            eh_ran4                          ( long* );
// double            eh_cosdev                        ( long* );
// double            eh_expdev                        ( long* );
// double            eh_powdev                        ( long* );
// double            eh_maxpowdev                     ( long* );
// double            eh_gasdev                        ( long* );
// double            eh_reject                        ( long* );

double eh_ran0(long*);
double eh_ran1(long*);
double eh_ran2(long*);
double eh_ran3(long*);
double eh_ran4(long*);

double eh_cosdev               (long*);
double eh_expdev               (long*);
double eh_powdev         (double,long*);
double eh_maxpowdev      (double,double,long*);
double eh_gasdev         ( long* );
double eh_reject         (double (*)(double),double (*)(double),double (*)(double) );

double eh_rand_exponential     ( GRand* rand , double                                 );
double eh_rand_max_exponential ( GRand* rand , double mean , double n                 );
double eh_log_normal           ( GRand* rand , double mean , double std               );
double eh_max_log_normal       ( GRand* rand , double mean , double std , double n    );
double eh_rand_weibull         ( GRand* rand , double eta  , double beta              );
double eh_rand_max_weibull     ( GRand* rand , double eta  , double beta , double n   );
double eh_rand_normal          ( GRand* rand , double mu   , double sigma             );
double eh_rand_user            ( GRand* rand , double *x   , double *F  , gssize len  );

#endif
