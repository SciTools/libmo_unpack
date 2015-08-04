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

/* extract_wgdos_row.c
 * 
 * Description:
 *   Read the non-bitmapped elements from one row of a WGDOS packed field
 *
 * ANSI C conforming to Met.Office C Programming Standard 1.0
 *
 * $Log: extract_wgdos_row.c,v $
 * Revision 1.3  2010/04/07 13:16:20  hadmf
 * SRCE checkin. Type: Update
 * Reason:
 *
 * Revision 1.2  2007/09/11 12:23:27  hadmf
 * SRCE checkin. Type: Undef
 * Reason: no need for the if (debug) statement
 *
 * Revision 1.1  2007/09/07 10:01:34  hadmf
 * SRCE checkin. Type: Undef
 * File Type: C Source
 * Purpose: Extract the bitmap data from a WGDOS packed row
 *
 * Revision 1.3  2006/10/27 16:28:44  hadmf
 * SRCE checkin. Type: Update
 * Reason: After review
 *
 * Revision 1.2  2006/06/28 10:18:33  hadmf
 * SRCE checkin. Type: Undef
 * Reason: remove detailed info printing from normal operation
 *
 * Revision 1.1  2006/06/27 15:27:35  hadmf
 * SRCE checkin. Type: Undef
 * File Type: C source
 * Purpose: read the non-bitmap values from a WGDOS compressed row
 *
 * Revision 1.1  1999/06/22 14:35:59  itae
 * Release 1.0
 *
 *
 * Information:
 *   
 */
/* Standard header files used */

#include <stdio.h>
#include <string.h>

/* Package header files used */

#include "wgdosstuff.h"
#define debug 0
/* End of header */

int extract_wgdos_row(
  /* IN */
  char** packed_data,            /* PP field to read from */
  int ndata,           /* Num non-bitmapped elements in row, >=0 && < 65636 */
  int bits_per_value,  /* Num bits per packed data value, >=0 && < 32*/
  /* OUT - Workspace supplied by caller */
  void *buffer,          /* Buffer at least bits_per_value*ndata bits, */
                             /* rounded up to the next complete numeric */
                             /* storage unit, long */
  /* OUT */
  int *data            /* Non-bitmapped elements, as integers */
)
{
  int  i;
  int status=0;
  int  nsus_to_read;      /* Number of numeric storage units occupied */
                          /* by bitmaps */

  if ( bits_per_value == 0 ) {
    /* No data to read (entire row equal to base value) */
    for ( i = 0; i < ndata; i++ ) {
      data[i] = 0;
    } /* end for */
  } else if ( ndata > 0 ) {
    /* Read packed data */
    nsus_to_read = (((bits_per_value * ndata) + PP_BITS_PER_NUMERIC - 1) /
                      PP_BITS_PER_NUMERIC);

    memcpy(buffer, *packed_data, nsus_to_read*(PP_BITS_PER_NUMERIC/8));
    *packed_data = *packed_data + nsus_to_read*(PP_BITS_PER_NUMERIC/8);
    /* Unpack it into integer form */

    status=extract_nbit_words(buffer, bits_per_value, ndata, data);

  } /* end if data to read */

  return status;
} /* end function PP_Wgdos_Read_Data */

