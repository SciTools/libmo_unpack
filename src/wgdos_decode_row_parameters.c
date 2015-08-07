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

/* wgdos_decode_row_parameters.c
 * 
 * Description:
 *   Read the row header information from one row of a WGDOS packed field
 *
 * ANSI C conforming to Met.Office C Programming Standard 1.0
 *
 * $Log: wgdos_decode_row_parameters.c,v $
 * Revision 1.5  2010/07/27 12:19:46  hadmf
 * SRCE checkin. Type: Undef
 * Reason:
 *
 * Revision 1.4  2010/04/07 13:18:14  hadmf
 * SRCE checkin. Type: Update
 * Reason:
 *
 * Revision 1.3  2010/01/15 13:46:22  hadmf
 * SRCE checkin. Type: Update
 * Reason: changed log level
 *
 * Revision 1.2  2007/09/11 12:35:15  hadmf
 * SRCE checkin. Type: Undef
 * Reason: syslog call changes
 *
 * Revision 1.1  2007/09/07 09:56:26  hadmf
 * SRCE checkin. Type: Undef
 * File Type: C Source
 * Purpose: Extract the WGDOS row header
 *
 * Revision 1.3  2006/10/27 16:28:23  hadmf
 * SRCE checkin. Type: Update
 * Reason: After review
 *
 * Revision 1.2  2006/07/04 10:05:06  hadmf
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

#include <stdio.h>
#include <string.h>
#include "wgdosstuff.h"
#include "logerrors.h"

#define MAX_MESSAGE_SIZE 1024
static char message[MAX_MESSAGE_SIZE];
/* End of header */

int wgdos_decode_row_parameters(
    char** data,
    float* base,
    Boolean  *missing_data_present, 
    Boolean  *zeros_bitmap_present,
    int      *bits_per_value,
    int      *nop,
    const function* const parent 
)

{
    int  int2_pair[2], int2_temp;
    int baselen = 1;
    int status=0;
    union {
      int i;
      float f;
    } basetemp;
    function subroutine;
    
    set_function_name(__func__, &subroutine, parent);
    
    /* Read base value for row */
    /* Use the union to change byteorder of basetemp value:
         needs an integer, so use the int version of basetemp */
    memcpy(&basetemp, *data, 4);
    basetemp.i = ntohl(basetemp.i);
    basetemp.i = ntohl(*(int*)*data);
    /* And change the IBM float to an IEEE float: uses the float version of basetemp */
    status=convert_float_ibm_to_ieee32(&basetemp.i, (int*)base, &baselen);
    if (status <0) {
      #ifdef DEBUG
      MO_syslog(VERBOSITY_ERROR, "IEEE/IBM float conversion failed", &subroutine);
      #endif
    }
    
    *data = *data + 4;        
    /* Read bits_per_value and flags */

    int2_temp =ntohl(*((int *)*data));
    *nop=int2_temp%65536;
    
    int2_pair[0] = int2_temp >> 16;

    *zeros_bitmap_present  = ((int2_pair[0] & 128) != 0); /* 8th bit: zero's present? */
    *missing_data_present  = ((int2_pair[0] &  32) != 0); /* 6th bit: MDI's present? */
    *bits_per_value        =   int2_pair[0] &  31; /* Lowest 5 bits: bits per value (<32) */

    *data = *data + 4;

    #ifdef DEBUG 
    snprintf(message, MAX_MESSAGE_SIZE, "Decoded BitFlags Zero:%d, MDI: %d. %d bits per value base value %f, %d words taken",
      *zeros_bitmap_present, *missing_data_present, *bits_per_value, *base, *nop);
    MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);
    #endif
    return status;
} /* end function PP_Wgdos_Start_Row */
