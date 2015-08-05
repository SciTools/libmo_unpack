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
# The views and conclusions contained in the software and documentation are those
# of the authors and should not be interpreted as representing official policies, 
# either expressed or implied, of the Met Office.
#
*/

/* pp_unpack_polybits.ibm.c
 * 
 * Description:
 *   Unpack a sequence of integers each bits_per_value bits long.
 *
 * ANSI C conforming to Met.Office C Programming Standard 1.0
 *
 * $Log: extract_nbit_words.c,v $
 * Revision 1.2  2010/04/07 13:16:06  hadmf
 * SRCE checkin. Type: Update
 * Reason:
 *
 * Revision 1.1  2007/09/07 10:02:05  hadmf
 * SRCE checkin. Type: Undef
 * File Type: C Source
 * Purpose: Extract as separate integers a bitstream of n-bit words
 *
 * Revision 1.6  2006/10/27 16:29:05  hadmf
 * SRCE checkin. Type: Update
 * Reason: After review
 *
 * Revision 1.5  2006/07/04 10:05:53  hadmf
 * SRCE checkin. Type: Undef
 * Reason: Update to make it understandable and conform to specified format on any platform
 *
 * Revision 1.2  1999/06/22 14:35:59  itae
 * Release 1.0
 *
 * Revision 1.1  1999/02/12 11:34:46  itae
 * Initial revision
 *
 *
 * Information:
 *  Machine independent: the given array of values are, as the spcification says, 
 *   treated as a bytestream, I.e. unsigned char type.
 *
 */

#include "wgdosstuff.h"

/* Return Value of function: number of items actually unpacked */

/*

We want

           from here          to here (inclusive)
                 |               |
      11111111 11111111 11111111 11111111 

      7      0 7      0 7      8 7      0
memory offset of the bits 



I.e. Start Bit=10, bits_per_value 15.

So reading from the end, we want bytes 2, 3 and 4.

We will eed to shift 7 bits (8 - (25%8)) right to get the lowet bit
  in byte4 correct.

Now, starting from Byte 4, read in the value.

Shift that down by 7 bits.

Add that to a running total

Take Byte 3, read in the value

Shift left 1 bit (one byte tracked back, less the 7 bit right shift)

(now the lower 7 bits now "occupy" the upper 7 bits of the previous number read)

Add that to the running total

Take Byte2, read in the value

Shift left 9 bits (two bytes tracked back, less the 7 bit right shift)

(moving the lower 7 bits into the "previous number" area)

Add to the total.

Now we have all the bits stacked up in the 32-bit integer, but we have
  higher order bits from Byte2 that we don't want.

We can mask these out by ANDing with (2^15)-1
  (15 is the number of bits per value, so the lowest 15 bits of the mask are set).

*/

#include <stdio.h>
int extract_nbit_words(
  /* IN */
  void *packed,           /* Integers in packed form */
  int bits_per_value,   /* Length of each packed integer in bits */
                              /* 1 <= bits_per_value <= 32 */
  int nitems,           /* Number of values to unpack */
  /* OUT */
  int *unpacked              /* Array of nitems integers. On exit contains */
                              /* the integers in unpacked form */
)
{
  int right_shift;            /* How far to shift the numbers left */
  int left_shift;            /* Used to work out right shift */
  unsigned char* offset;      /* What is the offset for the first useful byte */
  unsigned char* ptr;            /* what is the offset of the last/current useful byte */
  int i;                  /* a count of items extracted from "packed" */
  int byteshift;            /* How much to shift a byte to get it in the right area of an integer */
  unsigned int val;            /* Store the useful bits, once they have been extracted from a byte */
  unsigned int tval;            /* the value at "ptr" */
  unsigned int mask;            /* Masks out the unneeded higher bits */
    
  if (bits_per_value>32 || bits_per_value<=0) {
    return -1;
  }
    
  mask = (1<<bits_per_value)-1; /* The lowest bits_per_value bits of mask are set to 1 */

  /* Loop for each item to be unpacked */
  for ( i = 0; i < nitems; i++ ) {

    offset = packed + (i * bits_per_value/8); /* What is the first byte that contains bits we need? */
    ptr = packed + ((i+1) * bits_per_value/8); /* What is the last byte that contains bits we need? */
    left_shift = (i+1) * bits_per_value%8;  /* How many upper bits are required from the last byte? */
    right_shift = 8 - left_shift; /* Shift this much to move the upper bits down */

    val = 0;
    byteshift=-right_shift;

    while (offset<=ptr) {
      tval=*ptr; /* tval is the 8-bit unsigned number at the ptr offset. */
      if (byteshift<0) {
        tval = tval>>(-byteshift); /* Move this byte into the correct bit offset in tval */
      } else  {
        tval = tval<<byteshift; /* Move this byte into the correct bit offset in tval */
      }

      val+=tval; /* Put these bits, now in the right positions, in a safe place */
      byteshift+=8; /* We are moving to the next higher byte if we are going 'round again */
      ptr--; /* Move to the next higher byte */
    }
    unpacked[i] = val&mask; /* Mask out the unneeded bits in the final result  and put in the unpacked array */
  }
  return 0;
}
