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

/* read_wgdos_bitmaps.ibm.c
 * 
 * Description:
 *   Read the bitmap information from one row of a WGDOS packed field
 *
 * ANSI C conforming to Met.Office C Programming Standard 1.0
 *
 * $Log: read_wgdos_bitmaps.ibm.c,v $
 * Revision 1.4  2010/04/07 13:17:32  hadmf
 * SRCE checkin. Type: Update
 * Reason:
 *
 * Revision 1.3  2010/01/13 17:25:49  hadmf
 * SRCE checkin. Type: BugFix
 * Reason: bitmaps are contiguous, so the space reserved for it was calculated incorrectly
 *
 * Revision 1.2  2007/09/11 12:25:39  hadmf
 * SRCE checkin. Type: Undef
 * Reason: no need for if (debug) statement
 *
 * Revision 1.1  2007/09/07 09:58:54  hadmf
 * SRCE checkin. Type: Undef
 * File Type: C Source
 * Purpose: Read the bitmaps stored in a WGDOS packed field
 *
 * Revision 1.2  2006/06/28 10:18:29  hadmf
 * SRCE checkin. Type: Undef
 * Reason: remove detailed info printing from normal operation
 *
 * Revision 1.1  2006/06/27 15:27:01  hadmf
 * SRCE checkin. Type: Undef
 * File Type: C source
 * Purpose: read the bitmaps used in a WGDOS compressed field
 *
 * Revision 1.1  1999/06/22 14:35:59  itae
 * Release 1.0
 *
 *
 * Information:
 *   
 */

#include <stdio.h>

/* Package header files used */

#include "wgdosstuff.h"

/* End of header */

/* Local macros */

/* Set all elements of ITEMS (an array of N Booleans) to FALSE */

#define SET_FALSE(ITEMS, N) \
{ \
    int i; \
    for ( i = 0; i < (N); i++ ) { \
      (ITEMS)[i] = FALSE; \
    } /* end for */ \
}

/* Set COUNT to the no. of TRUE elements in ITEMS (an array of N Booleans) */
#define debug 0

#define COUNTER(COUNT, ITEMS, N) \
{ \
    int i; \
    (COUNT) = 0; \
    for ( i = 0; i < (N); i++ ) { \
      if ( (ITEMS)[i] ) (COUNT)++; \
    } /* end for */ \
}

int read_wgdos_bitmaps(
  /* IN */
  char** data,                     /* PP field to read from */
  int ncols,                 /* Number of elements in row, >=0 */
  Boolean missing_data_present,  /* Is missing data bitmap present? */
  Boolean zeros_bitmap_present,  /* Is zeros bitmap present? */
  /* IN - Workspace supplied by caller */
  void *buffer,                /* Buffer at least 2*ncols bits, rounded */
                                   /* up to the next complete int, long  */
  /* OUT */
  Boolean *missing_data,          /* Missing data bitmap for row */
  Boolean *zero,                  /* Zeros bitmap for row */
  int *missing_data_count,    /* Num TRUE elements in missing_data,>=0 */
  int *zeros_count           /* Num TRUE elements in zero, >=0 */
)
{
  int nsus_to_read;      /* Number of numeric storage units occupied */
                          /* by bitmaps */
  int zeros_start_bit;   /* Bit number in buffer where zeros bitmap starts */
    
  /* Determine length of bitmaps in numeric storage units */
  nsus_to_read = (ncols*(missing_data_present + zeros_bitmap_present)+ PP_BITS_PER_NUMERIC - 1) / PP_BITS_PER_NUMERIC;

  if ( nsus_to_read > 0 ) {
    /* Read bitmaps */
    buffer = *data;
    *data = *data + sizeof(int)*nsus_to_read;
  }

  /* Unpack missing data bitmap */
  if ( missing_data_present ) {
    extract_bitmaps(buffer, 0, ncols, TRUE, missing_data);                    
    COUNTER(*missing_data_count, missing_data, ncols);
  } else {
    SET_FALSE(missing_data, ncols);
    *missing_data_count = 0;
  }

  /* Unpack zeros bitmap */
  if ( zeros_bitmap_present ) {
    zeros_start_bit = (missing_data_present ? ncols : 0);
    extract_bitmaps(buffer, zeros_start_bit, ncols, FALSE, zero);
    COUNTER(*zeros_count, zero, ncols);
  } else {
    SET_FALSE(zero, ncols);
    *zeros_count = 0;
  }
  return 0;
}





