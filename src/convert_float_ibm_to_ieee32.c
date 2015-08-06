/*
# Copyright (c) 2012 - 2015, The Met Office, UK
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met: 
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer. 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution. 
# 3. Neither the name of copyright holder nor the names of any
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
*//***************************************************************************** 
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
 * procedure: convert_float_ibm_to_ieee32(float ibm,float ieee,int *n) *
 * ------------------------------------------------------------------- *        
 * PROCESSOR    - High C Compiler R2.1n                                *        
 *                                                                     *        
 * DEPENDENCES  - NONE                                                 *        
 *                                                                     *        
 * ATTRIBUTES   - REENTERANT                                           *        
 *                                                                     *        
 * ENTRY POINT  - _cfsi32                                              *        
 *                                                                     *        
 * STATUS       - NEW:           22 February 1990                      *        
 *                LAST REVISION: 19 July     1990                      *        
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
 * ------------------------------------------------------------------- *        
 * Change log:                                                         *        
 * ------------------------------------------------------------------- *        
 * Convert floating point, 32-bit IBM to 32-bit IEEE standard.         *        
 *                                                                     *        
 *   Example: call cfsi32(ibm, ieee, n)                                *        
 *                                                                     *        
 *  input: ibm    Array of IBM floating point numbers, REAL*4 values.  *        
 *         n      Number of elements in ibm to convert, integer.       *        
 * output: ieee   Array of 32-bit IEEE floating point numbers,         *        
 *                  single  precision.                                 *        
 *                                                                     *        
 * Format (bits, left to right):            |    Exponent bias:        *        
 *              sign   exponent   mantissa  |                          *        
 *  IBM           1      7           24     |     64 hex               *        
 *  IEEE          1      8           23     |     127                  *        
 *                                          |                          *        
 * Usage notes:                                                        *        
 * 1. Data could be converted "inplace".                               *        
 * 2. IBM values that do not conform to IEEE standard are converted to *        
 *    either infinite IEEE values (positive or negative)  or to zero.  *        
 * 3. Non-normalized with zero exponent values are kept intact.        *        
 * 4. Conversion does not incur the loss of mantissa accuracy.         *        
 * =================================================================== *        
 */                                                                             
#include <stdint.h>
#define   exp   0x7F000000                                                      
#define   sign  0x80000000                                                      
#define   tiss  0x00FFFFFF                                                      
#define   etis  0x007FFFFF                                                      
#define   nrm   0x00F00000                                                      
                                                                                
#pragma linkage (cfsi32, fortran)                                               

int convert_float_ibm_to_ieee32(int ibm[], int ieee[], int* n)
{
  int status = 0;
  int32_t j, ibs, ibe, ibt, it, k;
  union { int32_t i; float r; } u;

  for(j = 0; j < *n; j++) {
    ibs = ibm[j] & sign;
    ibe = ibm[j] & exp ;
    ibt = ibm[j] & tiss;
    if (ibt == 0) {
      ibe = 0 ;
    } else {
      if ( (ibe != 0) && (ibt & nrm) == 0 ) {
        u.i = ibm[j] ;
        u.r = u.r + 0e0 ;
        ibe = u.i & exp ;
        ibt = u.i & tiss ;
      }
      /* mantissa */
      it = ibt << 8;
      for (k = 0; (k < 5) && (it >= 0); k++ ) {
        it = it << 1;
      }
      if ( k < 4 ) {
        ibt = (it >> 8) & etis;
        ibe = (ibe >> 22) - 256 + 127 - k - 1;
        if (ibe < 0) {
          ibe = ibt = 0;
        }
        if (ibe >= 255) {
         ibe = 255; ibt = 0;
        }
        ibe = ibe << 23;
       }
    }
    ieee[j] = ibs | ibe | ibt;
    if (ibe == 255<<23) {
      status = -1;
    }
  }
  return status;
}
