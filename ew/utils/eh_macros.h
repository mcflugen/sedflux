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

#ifndef __EH_MACROS_H__
#define __EH_MACROS_H__

#ifdef __cplusplus
extern "C" {
#endif
#define E_BADVAL (G_MAXFLOAT)
#define E_NOVAL  (G_MAXFLOAT)

#define S_LINEMAX     (2048)
#define S_NAMEMAX     (255)
#define S_MAXPATHNAME (1024)

#define E_MAJOR_VERSION (1)
#define E_MINOR_VERSION (1)
#define E_MICRO_VERSION (0)

#define E_CHECK_VERSION(major,minor,micro)    \
    (E_MAJOR_VERSION > (major) || \
     (E_MAJOR_VERSION == (major) && E_MINOR_VERSION > (minor)) || \
     (E_MAJOR_VERSION == (major) && E_MINOR_VERSION == (minor) && \
      E_MICRO_VERSION >= (micro)))

#define eh_p_msg( msg_level , str ) \
   eh_print_msg( msg_level , __PRETTY_FUNCTION__ , str )

#define eh_lower_bound( val , low  ) ( val = ((val)<(low) )?(low ):(val) )
#define eh_upper_bound( val , high ) ( val = ((val)>(high))?(high):(val) )

#define eh_clamp( val , low , high ) \
   ( val = ((val)<(low))?(low):(((val)>(high))?(high):(val)) )

#define EH_SWAP_PTR( a , b ) { __typeof__ (b) t=(b); (b)=(a); (a)=t; }
#define swap_int( a , b ) { int temp=b; b=a; a=temp; }
#define swap_dbl( a , b ) { double temp=b; b=a; a=temp; }
#define swap_dbl_vec( x , i , j ) { double temp=x[i]; x[i]=x[j]; x[j]=temp; }
#define eh_memswap( a , b , n ) { void* temp=g_memdup(a,n); g_memmove(a,b,n); g_memmove(b,temp,n); eh_free(temp); }
#define EH_STRUCT_MEMBER( type , st , mem ) ( ((type*)st)->mem )

#define eh_new_2( type , m , n ) ( (type**)eh_alloc_2( m , n , sizeof(type) ) )
#define eh_free_2( p ) ( eh_free_void_2( ((void**)(p)) ) )

#define EH_RADS_PER_DEGREE ( 0.01745329251994 )
#define EH_DEGREES_PER_RAD ( 57.29577951308232 )

#define EH_SQRT_PI ( 1.77245385090552 )

#define POLYGON_IN_CROSSINGS  ( 1<<0 )
#define POLYGON_OUT_CROSSINGS ( 1<<1 )


#define BIT_ON	00000001
#define BIT_OFF	00000000
#define NO  0
#define YES 1
#define PTR_SIZE 8	/* length of a pointer in bytes */
#define ALL -1

/**********
*
* **** Macro descriptions
*
*  max(a,b)
*    evaluates to the maximum of the numbers a and b.
*
*  min(a,b)
*    evaluates to the minimum of the numbers a and b.
*
*  get_bit(pt,n)
*    returns the value of the nth bit in the data pointed to by pt.  To work
*    properly pt must be converted to char *.
*
*  put_bit(pt,n,bit)
*    Sets the value of the nth bit in the data pointed to by pt to value bit.
*    To work properly pt must be converted to char *. 
*
*  nbb(n)
*    evaluates to the minimum number of bytes able to contain n bits
*
**********/

#define eh_sign(a)         ( (a>=0)?1:-1 )
#define eh_max(a,b)    	   ( ( (a) > (b) ) ? (a) : (b) )
#define eh_set_max(a,b)    if ( (b)>(a) ) { (a) = (b); }
#define eh_set_min(a,b)    if ( (b)<(a) ) { (a) = (b); }
#define eh_dbl_set_max(a,b) { double __t=(b); if ( __t>(a) ) { (a)=__t; } }
#define eh_dbl_set_min(a,b) { double __t=(b); if ( __t<(a) ) { (a)=__t; } }
#define eh_min(a,b)    	   ( ( (a) < (b) ) ? (a) : (b) )

#define get_bit(pt,n)	   ( (pt)[(n)/8]&(0x80>>((n)%8)) )
#define turn_bit_on(pt,n)  (pt)[(n)/8] = (pt)[(n)/8] | (0x80>>((n)%8))
#define turn_bit_off(pt,n) (pt)[(n)/8] = (pt)[(n)/8] &~ (0x80>>((n)%8));
#define nbb(n)             ( ( (n) + 7 ) / 8 )

#define eh_sqr( x )        ( (x)*(x) )
#define eh_nrsign( a , b )   ( (b>0)?fabs(a):(-fabs(a)) )


/**********
*
* **** Macro descriptions
*
*  initarray(pt,len,c)
*    Initializes the array pointed to by pt of length len to value c.
*
*  count_pix(pt,len,val,c)
*    Counts the number of elements of value, val in the data pointed to by pt
*    of length len.  The count is stored in c.
*
*  rotate_array(ar_type,pt,len)
*    Rotates the values in the array pointed to by pt (of length, len and type,
*    ar_type).  The values are rotated such that pt[i]=pt[i+1].  Also, pt[0] is
*    stored in a temporary variable and copied to pt[len-1].
*
*  mmove(dst,src,len)
*    Copies the len characters pointed to by src into the object pointed to by
*    dst.  The characters are not copied to a temporary location before being 
*    copied.  Thus, for the data to be copied correctly, the source and
*    destination objects must not overlap.
*
**********/

#ifndef initarray
# define initarray(pt,len,c) { unsigned long i; for (i=0;i<len;(pt)[i++]=c); }
#endif
#define count_pix(pt,len,val,c) { unsigned long i; for (i=0;i<len;i++) if ((pt)[i]==val) c++; }
#define rotate_array(ar_type,pt,len) { unsigned long i;				      \
				       ar_type temp;					      \
				       for (i=0,temp=(pt)[0];i<len-1;(pt)[i]=(pt)[i+1],i++); \
				       (pt)[len-1]=temp;				      \
				     }
#define mmove(dst,src,len) { unsigned long j; 		        \
			     for (j=0;j<(len);j++) 		\
			              (dst)[j]=(src)[j];	\
			   }



#ifdef __cplusplus
}
#endif

#endif /* eh_macros.h is included */
