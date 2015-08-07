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

/* wgdos_decode_field_parameters
 * 
 * Description:
 *   Read the field header information from a WGDOS packed field
 *
 * ANSI C conforming to Met.Office C Programming Standard 1.0
 *
 * $Log: wgdos_decode_field_parameters.c,v $
 * Revision 1.5  2010/07/27 12:19:42  hadmf
 * SRCE checkin. Type: Undef
 * Reason:
 *
 * Revision 1.4  2010/04/07 13:18:04  hadmf
 * SRCE checkin. Type: Update
 * Reason:
 *
 * Revision 1.3  2010/01/15 13:46:09  hadmf
 * SRCE checkin. Type: Update
 * Reason: changed log level
 *
 * Revision 1.2  2007/09/11 12:34:51  hadmf
 * SRCE checkin. Type: Undef
 * Reason: syslog call changes
 *
 * Revision 1.1  2007/09/07 09:56:49  hadmf
 * SRCE checkin. Type: Undef
 * File Type: C Source
 * Purpose: Extract the Field header from a WGDOS packed field
 *
 * Revision 1.3  2006/10/27 16:28:30  hadmf
 * SRCE checkin. Type: Update
 * Reason: After review
 *
 * Revision 1.2  2006/07/04 10:05:20  hadmf
 * SRCE checkin. Type: Undef
 * Reason:
 *
 * Revision 1.1  1999/06/22 14:35:59  itae
 * Release 1.0
 *
 *
 * Information:
 *   
 */

/* Standard header files used */

#include <math.h>
#include <stdio.h>

#include "wgdosstuff.h"
#include "logerrors.h"

#define MAX_MESSAGE_SIZE 1024
static char message[MAX_MESSAGE_SIZE];

/* End of header */

int wgdos_decode_field_parameters (
    /* IN */
    char** data,        /* address of PP field to read from */
    int unpacked_len, /* Expected length that data should expand to when */
                        /* unpacked, >=0 */
    /* OUT */
    float *accuracy,     /* Absolute accuracy to which data held in field */
    int *ncols,        /* Number of columns in field, >=0 */
    int *nrows,        /* Number of rows in field, >=0 */
    const function* const parent
)
{
  int log_2_acc;     /* Log to base 2 of accuracy */
  int status=0;
  function subroutine;
  set_function_name(__func__, &subroutine, parent);

  typedef struct wgdos_field_header {
    uint32_t total_length;
    uint32_t precision;
    uint16_t pts_in_row;
    uint16_t rows_in_field;
  } wgdos_field_header;
    
  wgdos_field_header* field_header_pointer;
  wgdos_field_header field_header;
    
  *accuracy = 0.0;
  *ncols = 0;
  *nrows = 0;
  field_header_pointer=(wgdos_field_header*)*data;
  field_header.total_length=ntohl(field_header_pointer->total_length);
  field_header.precision=ntohl(field_header_pointer->precision);
  field_header.pts_in_row=ntohs(field_header_pointer->pts_in_row);
  field_header.rows_in_field=ntohs(field_header_pointer->rows_in_field);
  log_2_acc=field_header.precision;
    
  *accuracy=pow(2, log_2_acc);
  *ncols=field_header.pts_in_row;
  *nrows=field_header.rows_in_field;

  /* read ncols, nrows */
  if (*ncols <= 0) {
    printf("zero/negative ncols\n");
    status=-1;
  } else if (*nrows <= 0) {
    printf("zero/negative nrows\n");
    status = -1;
  } else if (*nrows * *ncols != unpacked_len) {
    printf("size inconsistent %d * %d != %d\n", *nrows, *ncols, unpacked_len);
    status = -1;
  }
  #ifdef DEBUG  
  snprintf(message, MAX_MESSAGE_SIZE, "WGDOS_decode_field_parameters returned total length %d, precision %d, row length %d and rows %d",
      field_header.total_length, field_header.precision, field_header.pts_in_row, field_header.rows_in_field);
  MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);
  #endif

  return status;
}
