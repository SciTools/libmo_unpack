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
*/
/* wgdos_expand_row_to_data.c
 * 
 * Description:
 *   Unpack one row of a WGDOS packed field
 *
 * ANSI C conforming to Met.Office C Programming Standard 1.0
 *
 * $Log: wgdos_expand_row_to_data.c,v $
 * Revision 1.6  2010/11/16 15:23:37  hadmf
 * SRCE checkin. Type: Update
 * Reason: don't waste lots of time printing if the message isn't going out. Saves 80%+ of the time unpacking a field
 *
 * Revision 1.5  2010/07/27 12:19:48  hadmf
 * SRCE checkin. Type: Undef
 * Reason:
 *
 * Revision 1.4  2010/04/07 13:18:25  hadmf
 * SRCE checkin. Type: Update
 * Reason:
 *
 * Revision 1.3  2008/07/21 10:45:59  hadmf
 * SRCE checkin. Type: Update
 * Reason: Add a routine that will invaidate data at the end of an overwritten WGDOS row.
 *
 * Revision 1.2  2007/09/11 12:35:45  hadmf
 * SRCE checkin. Type: Undef
 * Reason: syslog call changes
 *
 * Revision 1.1  2007/09/07 09:56:02  hadmf
 * SRCE checkin. Type: Undef
 * File Type: C Source
 * Purpose: expand a single extracted WGDOS row
 *
 * Revision 1.1  2006/06/27 15:26:37  hadmf
 * SRCE checkin. Type: Undef
 * File Type: C source
 * Purpose: Calculate the uncompressed values in a WGDOS compressed row
 *
 * Revision 1.1  1999/06/22 14:35:59  itae
 * Release 1.0
 *
 *
 * Information:
 *   
 */

/* Package header files used */
#include <string.h>
#include <stdio.h>
#include "wgdosstuff.h"
#include "logerrors.h"
#define debug 0

#define MAX_MESSAGE_SIZE 1024
char message[MAX_MESSAGE_SIZE];

/* End of header */

int wgdos_expand_row_to_data(
    int       ncols,         
    float     mdi,          
    float     accuracy,     
    float     base,         
    Boolean  *missing_data,  
    Boolean  *zero,          
    int  *data,           
    float    *unpacked_data, 
    int      *mdi_clashes,
    const function* const parent    
)
{
    
    int  non_special_so_far;  /* Number of non-special items unpacked so far */
    int  col;                 /* Number of items unpacked so far */
    int off=1;
    function subroutine;
    int log_messages=(get_verbosity()>=VERBOSITY_MESSAGE);
    set_function_name(__func__, &subroutine, parent);
    double dacc, ddata, dbase, dval;

    *mdi_clashes = 0;
    non_special_so_far = 0;
    dacc=accuracy;
    dbase=base;

    message[0]=0;
    for ( col = 0; col < ncols; col ++ ) {
      off=1;
      if ( missing_data[col] ) {
        unpacked_data[col] = mdi;
        #ifdef DEBUG
        snprintf (message+strlen(message), MAX_MESSAGE_SIZE-strlen(message), " %012g / %-12s", unpacked_data[col],"MDI");
        #endif
      } else if ( zero[col] ) {
        unpacked_data[col] = 0.0;
        #ifdef DEBUG
        snprintf (message+strlen(message), MAX_MESSAGE_SIZE-strlen(message), " %012g / %-12s", unpacked_data[col],"Zero");
        #endif
      } else {
        dval=dacc*data[non_special_so_far]+dbase;
        unpacked_data[col] = dval;
        if ( unpacked_data[col] == mdi ) {
          (*mdi_clashes)++;
        } 
        non_special_so_far++;
        off=1;
        #ifdef DEBUG
        snprintf (message+strlen(message), MAX_MESSAGE_SIZE-strlen(message), " %012g / %-12d", unpacked_data[col],data[non_special_so_far-off]);
        #endif
      }
      #ifdef DEBUG
      if (log_messages &&(col%4 == 3)) {
        snprintf(message+strlen(message), MAX_MESSAGE_SIZE-strlen(message), "%3d", col);
        MO_syslog (VERBOSITY_MESSAGE, message, &subroutine);
        message[0]=0;
      }
      #endif
    }
    #ifdef DEBUG
    if (message[0]!=0 && log_messages) {
      snprintf(message+strlen(message), MAX_MESSAGE_SIZE-strlen(message), "%3d", col);
      MO_syslog (VERBOSITY_MESSAGE, message, &subroutine);
      message[0]=0;
    }
    #endif
    return 0;
} /* end function PP_Wgdos_Expand_Row */

int wgdos_expand_broken_row_to_data(
    int       ncols,         
    int       badnumbers,
    float     mdi,          
    float     accuracy,     
    float     base,         
    Boolean  *missing_data,  
    Boolean  *zero,          
    int  *data,           
    float    *unpacked_data, 
    int      *mdi_clashes,
    const function* const parent    
)
{
    
    int  non_special_so_far;  /* Number of non-special items unpacked so far */
    int  col;                 /* Number of items unpacked so far */
    int off=1;
    int log_messages=(get_verbosity()>=VERBOSITY_MESSAGE);
    function subroutine;
    
    set_function_name(__func__, &subroutine, parent);
    
    *mdi_clashes = 0;
    non_special_so_far = 0;
    
    message[0]=0;
    for ( col = 0; col < ncols; col ++ ) {
      off=1;
      if ( missing_data[col]) {
        unpacked_data[col] = mdi;
      } else if ( zero[col] ) {
        unpacked_data[col] = 0.0;
      } else {
        if (non_special_so_far >= badnumbers) {
          unpacked_data[col] = mdi;
        } else {
          unpacked_data[col] = accuracy * data[non_special_so_far] + base;
          if ( unpacked_data[col] == mdi ) {
            (*mdi_clashes)++;
          } 
          non_special_so_far++;
          off=1;
        }
      }
      #ifdef DEBUG
      if (log_messages && (col%3 == 0)) {
        snprintf(message+strlen(message), MAX_MESSAGE_SIZE-strlen(message), "%3d", col);
        MO_syslog (VERBOSITY_MESSAGE, message, &subroutine);
        message[0]=0;
      }
      if (log_messages) {
        snprintf (message+strlen(message), MAX_MESSAGE_SIZE-strlen(message), " %.16g/%05d", unpacked_data[col],data[non_special_so_far-off]);
      }
      #endif
    }
    #ifdef DEBUG
    if (log_messages && message[0]!=0) {
      snprintf(message+strlen(message), MAX_MESSAGE_SIZE-strlen(message), "%3d", col);
      MO_syslog (VERBOSITY_MESSAGE, message, &subroutine);
      message[0]=0;
    }
    #endif
    return 0;
} /* end function PP_Wgdos_Expand_Broken_Row */
