/***************************************************************************** 
*                                                                               
*                         NCSA HDF version 3.10                                 
*                               July 1, 1990                                    
*                                                                               
* NCSA HDF Version 3.10 source code and documentation are in the public         
* domain.  Specifically, we give to the public domain all rights for future                                                                          
* licensing of the source code, all resale rights, and all publishing rights.                                                                         
*                                                                               
* We ask, but do not require, that the following message be included in all                                                                             
* derived works:                                                                
*                                                                               
* Portions developed at the National Center for Supercomputing Applications at                                                                 
* the University of Illinois at Urbana-Champaign.                               
*                                                                               
* THE UNIVERSITY OF ILLINOIS GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE                                                                         
* SOFTWARE AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION,                                                                     
* WARRANTY OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE                                                                         
*                                                                               
*****************************************************************************/  
/* ------------------------------------------------------------------- *        
 * procedure: convert_float_ieee32_to_ibm(float ieee, float ibm, int *n)*
 * ------------------------------------------------------------------- *        
 * processor   - High C Compiler R2.1n                                 *        
 *                                                                     *        
 * dependences - none                                                  *        
 *                                                                     *        
 * attributes  - reenterant                                            *        
 *                                                                     *        
 * entry point - _cfi32s                                               *        
 *                                                                     *        
 * status      - new:           16 June  1989                          *        
 *               last revision: 03 May   1990                          *        
 *                                                                     *        
 *                            Val I. Garger, Technology Integration    *        
 *                            Group, CNSF, Cornell University          *        
 *                                                                     *        
 *                               vig@eagle.cnsf.cornell.edu            *        
 *-------------------------------------------------------------------- *        
 *                                                                     *        
 *  COPYRIGHT -  VAL GARGER, CORNELL NATIONAL SUPERCOMPUTER FACILITY,  *        
 *               (JUNE 1990) CORNELL UNIVERSITY, ITHACA, NY.           *        
 *               CONTAINS RESTRICTED MATERIALS OF CORNELL UNIVERSITY,  *        
 *               (C) COPYRIGHT CORNELL UNIVERSITY 1990                 *        
 *                                                                     *        
 *-------------------------------------------------------------------- *        
 * Change log:                                                         *        
 *-------------------------------------------------------------------- *        
 * Convert floating point, IEEE 32-bit to IBM 32-bit (REAL in Fortran).*        
 *                                                                     *        
 *   Example: call cfi32s(ieee, ibm, n)                                *        
 *                                                                     *        
 *   input: ieee   Array of 32-bit IEEE floating point numbers,        *        
 *                 single  precision.                                  *        
 *          n      Number of elements in ieee  to convert, integer.    *        
 *  output: ibm    Array of IBM floating point numbers, REAL*4 values. *        
 *                                                                     *        
 * Format (bits, left to right):            |    Exponent bias:        *        
 *              sign   exponent   mantissa  |                          *        
 *  IBM           1      7           24     |     64 hex               *        
 *  IEEE          1      8           23     |     127                  *        
 *                                          |                          *        
 * Usage notes:                                                        *        
 * 1. Data could be converted "inplace".                               *        
 * 2. Infinite IEEE values are converted to the largest IBM values     *        
 *    which are x'7FFFFFFF' and x'FFFFFFFF' for positive and negative  *        
 *    respectively.                                                    *        
 * 3. Like infinite values, NaN (Not a Number) values are converted to *        
 *    the largest values.                                              *        
 * 4. Precision in the mantissa could be lost by rounding off the      *        
 *    least significant bits.         0 <= |error| <= 0.24E-6          *
 *    (From 0 to 3 least significant bits out of 24 mantissa bits      *
 *    could be rounded.)                                               *
 * =================================================================== *
 */                                                                             
#include <stdint.h>
#define last 0x000000ff
#define impl 0x00800000
#define sign 0x80000000
#define tiss 0x007fffff
                                                                                
#pragma linkage (cfi32s, fortran)
int convert_float_ieee32_to_ibm(int ieee[], int ibm[], int* n)
{
  int status = 0;
  int32_t j, k, ibs, ibe, ibt;
  for(j = 0; j < *n; j++) {
    ibt = ieee[j];
    ibs = ieee[j] & sign;
    ibe = (ieee[j] >> 23) & last;

    if (ibe != 0)
    {
      if (ibe == 255)
      { ibe = 378;
        ibt = tiss;
      status = -1; /* Inf or NaN */
      }
      ibe = ibe - 127 + 256 +1;
      k = ibe%4;
      ibe = ibe >> 2;
      if (k != 0)
        ibe = ibe + 1;
      ibe = ibe << 24 ;
      ibt = (ibt & tiss) | impl ;
      if (k != 0)
        ibt = ( ibt + (1 << (3-k) )  ) >> (4-k);
      if (status == 0) {
        status = 1; /* Rounding error */
      }
    }
    ibm[j] = ibs | ibe | ibt;
  }
  return status;
}
