/*
# Copyright (c) 2012, The Met Office, UK
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
*/

/* Extract_Bitmaps.c
 * 
 * Description:
 *   Unpack a sequence of bits into an array of Boolean.
 *
 * ANSI C conforming to Met.Office C Programming Standard 1.0
 *
 * $Log: extract_bitmaps.c,v $
 * Revision 1.1  2007/09/07 10:03:06  hadmf
 * SRCE checkin. Type: Undef
 * File Type: C Source
 * Purpose: Extract the bitmaps from a WGDOS packed field
 *
 * Revision 1.3  2006/10/27 16:29:13  hadmf
 * SRCE checkin. Type: Update
 * Reason: After review
 *
 * Revision 1.2  2006/07/04 10:06:02  hadmf
 * SRCE checkin. Type: Undef
 * Reason:
 *
 * Revision 1.2  1999/06/22 14:35:59  itae
 * Release 1.0
 *
 * Revision 1.1  1999/02/12 11:34:46  itae
 * Initial revision
 *
 *
 * Information:
 *   MACHINE DEPENDENT code; works only on big-endian architechtures
 *
 *   Since execution speed is very important here, the complete loop is
 *   duplicated for the "one true" and "zero true" cases, rather than testing
 *   one_true on each iteration of the loop.
 *
 *  Machine independence has been written in by using "unsigned char" type, 
 *    whose order is not machine dependent. Done by the simple expedient of
 *    changing UINT_BITS from 32 to 8.
 */
#include "wgdosstuff.h"
#define UINT_BITS  (8)  /* Num bits in an unsigned int */
#define TOP_BIT    (1U << (UINT_BITS - 1))     /* Unsigned int with just the */
                                               /* top bit set */

/* End of header */

int extract_bitmaps(void    *packed, /* Sequence of bits to be unpacked */
  int start_bit, /* Bit number of first bit to be unpacked (0 upwards) */
  int nbits, /* Number of bits to unpack */
  Boolean one_true, /* How to interpret bits
                       TRUE => 1->TRUE, 0->FALSE.
                       FALSE => 0->TRUE, 1->FALSE. */
  Boolean *unpacked /* OUT: Array containing the unpacked bits */
)
{
  /* Local variables */
  unsigned char* packed_uint; /* Packed data regarded as an array of unsigned bytes */
  unsigned char mask; /* Mask to select current bit */
  int i; /* Number of bits unpacked so far */

  /* Interpret packed data as an array of unsigned bytes and find 
     the byte containing first bit to be unpacked */
    
  packed_uint = (unsigned char *) packed;
  packed_uint += (start_bit / UINT_BITS);

  /* Initialise mask so that it selects first bit to be unpacked */
  mask = (TOP_BIT >> (start_bit  % UINT_BITS));

  if ( one_true ) {
    /* Mark a TRUE value (1) when the bit is a 1 */
    for ( i = 0; i < nbits; i++ ) {
      /* Unpack bit */
      unpacked[i] = (((*packed_uint) & mask) != 0);
      if ( mask == 1U ) {
        /* We are at the end of this byte. Move to the beginning of the next byte */
        packed_uint++;
        mask = TOP_BIT;
      } else {
        /* Rotate mask right so that it selects next bit. */
        mask = mask >> 1;
      }
    }
  } else {
    /* Mark a TRUE value (1) when the bit is a 0 */
    for ( i = 0; i < nbits; i++ ) {
      /* Unpack bit */
      unpacked[i] = (((*packed_uint) & mask) == 0);
      /* Rotate mask right so that it selects next bit. Move on to next */
      /* element of packed_uint if necessary.*/
      if ( mask == 1U ) {
        packed_uint++;
        mask = TOP_BIT;
      } else {
        mask = mask >> 1;
      }
    }
  }
  return 0;
}
