/* Copyright (C) 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef _IEEE754_H
#define _IEEE754_H

#include <endian.h>

union ieee754_double
  {
    double d;
    
    /* This is the IEEE 754 double-precision format.  */
    struct
      {
#ifdef	__BIG_ENDIAN
	unsigned int negative:1;
	unsigned int exponent:11;
	/* Together these comprise the mantissa.  */
	unsigned int mantissa0:20;
	unsigned int mantissa1:32;
#endif
#ifdef	__LITTLE_ENDIAN
	/* Together these comprise the mantissa.  */
	unsigned int mantissa1:32;
	unsigned int mantissa0:20;
	unsigned int exponent:11;
	unsigned int negative:1;
#endif
      } ieee;
  };

#define _IEEE754_DOUBLE_BIAS            0x3ff   /* added to exp of ieee754_double */

#ifdef __i386__

union i387_float
  {
    float f;
    
    /* This is the i387 single-precision format.  */
    struct
      {
	unsigned int mantissa:23;
	unsigned int exponent:8;
	unsigned int negative:1;
      } i387;
  };

#define _I387_FLOAT_BIAS            0x7f   /* added to exp of i387_float */

#endif /* __i387__ */

#endif	/* _IEEE754_H */