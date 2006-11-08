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

#if !defined( COMPLEX_H )
#define COMPLEX_H

typedef struct
{
   double r;
   double i;
}
Complex;

Complex c_add(Complex a, Complex b);
Complex c_sub(Complex a, Complex b);
Complex c_mul(Complex a, Complex b);
Complex c_div(Complex a, Complex b);
Complex c_complex(double re, double im);
Complex c_conj(Complex z);
double  c_abs(Complex z);
Complex c_sqrt(Complex z);
Complex c_rcmul(double x, Complex a);

#endif 

