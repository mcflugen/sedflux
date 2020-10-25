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

#include <math.h>

#include "complex.h"

Complex
c_add(Complex a, Complex b)
{
    Complex c;
    c.r = a.r + b.r;
    c.i = a.i + b.i;
    return c;
}

Complex
c_sub(Complex a, Complex b)
{
    Complex c;
    c.r = a.r - b.r;
    c.i = a.i - b.i;
    return c;
}


Complex
c_mul(Complex a, Complex b)
{
    Complex c;
    c.r = a.r * b.r - a.i * b.i;
    c.i = a.i * b.r + a.r * b.i;
    return c;
}

Complex
c_complex(double re, double im)
{
    Complex c;
    c.r = re;
    c.i = im;
    return c;
}

Complex
c_conj(Complex z)
{
    Complex c;
    c.r = z.r;
    c.i = -z.i;
    return c;
}

Complex
c_div(Complex a, Complex b)
{
    Complex c;
    double r, den;

    if (fabs(b.r) >= fabs(b.i)) {
        r = b.i / b.r;
        den = b.r + r * b.i;
        c.r = (a.r + r * a.i) / den;
        c.i = (a.i - r * a.r) / den;
    } else {
        r = b.r / b.i;
        den = b.i + r * b.r;
        c.r = (a.r * r + a.i) / den;
        c.i = (a.i * r - a.r) / den;
    }

    return c;
}

double
c_abs(Complex z)
{
    double x, y, ans, temp;
    x = fabs(z.r);
    y = fabs(z.i);

    if (x == 0.0) {
        ans = y;
    } else if (y == 0.0) {
        ans = x;
    } else if (x > y) {
        temp = y / x;
        ans = x * sqrt(1.0 + temp * temp);
    } else {
        temp = x / y;
        ans = y * sqrt(1.0 + temp * temp);
    }

    return ans;
}

Complex
c_sqrt(Complex z)
{
    Complex c;
    double x, y, w, r;

    if ((z.r == 0.0) && (z.i == 0.0)) {
        c.r = 0.0;
        c.i = 0.0;
        return c;
    } else {
        x = fabs(z.r);
        y = fabs(z.i);

        if (x >= y) {
            r = y / x;
            w = sqrt(x) * sqrt(0.5 * (1.0 + sqrt(1.0 + r * r)));
        } else {
            r = x / y;
            w = sqrt(y) * sqrt(0.5 * (r + sqrt(1.0 + r * r)));
        }

        if (z.r >= 0.0) {
            c.r = w;
            c.i = z.i / (2.0 * w);
        } else {
            c.i = (z.i >= 0) ? w : -w;
            c.r = z.i / (2.0 * c.i);
        }

        return c;
    }
}

Complex
c_rcmul(double x, Complex a)
{
    Complex c;
    c.r = x * a.r;
    c.i = x * a.i;
    return c;
}

