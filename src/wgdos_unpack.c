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

/* wgdos_unpack.c
 * 
 * Description:
 *   Unpack a WGDOS packed field as normal unpacked data
 *
 * ANSI C conforming to Met.Office C Programming Standard 1.0
 *
 * $Log: wgdos_unpack.c,v $
 * Revision 1.7  2010/07/27 12:19:51  hadmf
 * SRCE checkin. Type: Undef
 * Reason:
 *
 * Revision 1.6  2010/04/07 13:18:36  hadmf
 * SRCE checkin. Type: Update
 * Reason:
 *
 * Revision 1.5  2010/01/15 14:07:58  hadmf
 * SRCE checkin. Type: Update
 * Reason: Forgot to set status
 *
 * Revision 1.4  2010/01/15 13:47:03  hadmf
 * SRCE checkin. Type: Update
 * Reason: changed log levels, stored format error code, doesn't do corrective action, just skips the field
 *
 * Revision 1.3  2008/07/21 10:45:27  hadmf
 * SRCE checkin. Type: Update
 * Reason: Undo as far as possible an error in the WGDOS packing routine
 *
 * Revision 1.2  2007/09/11 12:36:28  hadmf
 * SRCE checkin. Type: Undef
 * Reason: syslog call changes
 *
 * Revision 1.1  2007/09/07 09:55:02  hadmf
 * SRCE checkin. Type: Undef
 * File Type: C source
 * Purpose: Main routine for unpacking WGDOS packed fields
 *
 * Revision 1.2  2006/10/27 16:28:15  hadmf
 * SRCE checkin. Type: Update
 * Reason: After review
 *
 * Revision 1.1  2006/06/27 15:29:18  hadmf
 * SRCE checkin. Type: Undef
 * File Type: C source
 * Purpose: Uncompress a WGDOS compressed field
 *
 * Revision 1.3  1999/06/22 14:35:59  itae
 * Release 1.0
 *
 * Revision 1.2  1999/04/12 15:15:19  itae
 * *** empty log message ***
 *
 * Revision 1.1  99/02/12  11:34:47  11:34:47  itae (Andrew Edmunds)
 * Initial revision
 * 
 *
 * Information:
 */

/* Standard header files used */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Package header files used */
#include "wgdosstuff.h"
#include "logerrors.h"

#define MAX_MESSAGE_SIZE 1024
static char message[MAX_MESSAGE_SIZE];
/* End of header */

int wgdos_unpack(
    /* IN */
    char*     packed_data,           /* Packed data */
    int       unpacked_len,          /* Expected length that data should expand */
                                     /* to when unpacked, >=0 */
    float*    unpacked_data,
    float     mdi,                   /* Missing data indicator value */
    function* parent)
{
    float     accuracy;              /* Absolute accuracy to which data held */
    int       ncols;                 /* Number of columns in each row */
    int       nrows;                 /* Number of rows in field */

    char*     buffer;                /* Buffer for packed data */
    Boolean*  missing_data;          /* Missing data bitmap for current row */
    Boolean*  zero;                  /* Zeros bitmap for current row */
    int*      data;                  /* Non-bitmapped data for current row */
    int       row;                   /* Number of rows transmitted so far */
    float     base;                  /* Base value for current row */
    Boolean   missing_data_present;  /* Is missing data bitmap present? */
    Boolean   zeros_bitmap_present;  /* Is zeros bitmap present? */
    int       bits_per_value;        /* Number of bits per packed data value */
    int       missing_data_count;    /* Number of missing data elements */
    int       zeros_count;           /* Number of bitmapped zeros */
    int       ndata;                 /* Number of non-bitmapped data items */
    int       row_mdi_clashes;       /* Number of MDI values in row */
    int       mdi_clashes;           /* Number of MDI values in field */
    float*    unpacked_row;
    int       nop;                   /* number of values to extract according to header */
    char*     start_off;
    int status = 0;
    mdi_clashes = 0;
    int offset;
    int next_packed_row;
    function subroutine;
    
    set_function_name(__func__, &subroutine, parent);

    #ifdef DEBUG
    snprintf(message, MAX_MESSAGE_SIZE, "MDI given as %f from %s", mdi, parent->name);
    MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);
    #endif
    /* Read field header information */
    status=wgdos_decode_field_parameters(&packed_data, unpacked_len, &accuracy, &ncols, &nrows, &subroutine);
    packed_data=packed_data+12;

    /* Reserve work areas */
    buffer        = (void *) malloc(PP_BYTES_PER_NUMERIC * ncols); 
    missing_data  = (Boolean *) malloc(sizeof(Boolean) * ncols);
    zero          = (Boolean *) malloc(sizeof(Boolean) * ncols);
    data          = (int *) malloc(sizeof(int) * ncols);
    unpacked_row  = (float *) malloc(sizeof(float) * ncols); 
    
    /* did it work? */
    if (status || !(buffer && missing_data && zero && data && unpacked_row)) {
      status=-1;
      return status;
    }

    /* Unpack each row */
    row = 0;
    next_packed_row=0;
    while ( row < nrows)  {
      #ifdef DEBUG
      snprintf(message, MAX_MESSAGE_SIZE, "On row %d", row);
      MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);
      #endif
      start_off=packed_data;
 
      /* Read row header information */
      status=wgdos_decode_row_parameters(&packed_data, &base, &missing_data_present, 
                           &zeros_bitmap_present, &bits_per_value, &nop, &subroutine );
      next_packed_row=nop*4+8; /* bytes = nop*4 (32 bit words) + 8 bytes for the row header*/
      if (status) {
        #ifdef DEBUG
        snprintf(message, MAX_MESSAGE_SIZE, "Failed to properly decode row %d parameters. Base %f. bpv %d", row, base, bits_per_value);
        MO_syslog(VERBOSITY_ERROR, message, &subroutine);
        #endif
        set_logerrno(LOGERRNO_FORMAT_EXCEPTION);
        status=-1;
        break;
      }

      /* Read in and expand the bitmaps in the packed data field */
      status=read_wgdos_bitmaps(&packed_data, ncols, missing_data_present,
                              zeros_bitmap_present, buffer,
                              missing_data, zero, &missing_data_count,
                              &zeros_count);
      
      ndata = ncols - missing_data_count - zeros_count;

      /* Read in the packed data into the data buffer */
      status=extract_wgdos_row(&packed_data, ndata, bits_per_value,
                           buffer, data);
      /* Unpack the data in the data buffer */
      status=wgdos_expand_row_to_data(ncols, mdi,  accuracy, base,
                            missing_data, zero, data,
                            unpacked_row, &row_mdi_clashes, &subroutine);
      #ifdef DEBUG
      snprintf (message, MAX_MESSAGE_SIZE, "Row %d: Base %g, %d bits per value, accuracy %g", row, base, bits_per_value, accuracy);
      MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);
      #endif
      mdi_clashes += row_mdi_clashes;
      offset=row*ncols;
      /* Copy row across from unpacked_row to the right offset in the unpacked_data*/
      memcpy(&unpacked_data[row*ncols], unpacked_row, sizeof(float)*ncols);

      /* Check that the number of data values read is correct wrt the WGDOS header */
      if (packed_data-start_off != next_packed_row) {
        #ifdef DEBUG
        snprintf (message, MAX_MESSAGE_SIZE, "WGDOS row (%d) length (%d) doesn't agree with disk length (%d) for %d values", \
                 row, next_packed_row, packed_data-start_off, ndata);
        MO_syslog(VERBOSITY_ERROR, message, &subroutine);
        #endif
        set_logerrno(LOGERRNO_FORMAT_EXCEPTION);
        status=-1;
        break;
      }
      row++;
    }

    /* Free the work areas */
    free(buffer);
    free(missing_data);
    free(zero);
    free(data);
    free(unpacked_row);  
    return status;
}
