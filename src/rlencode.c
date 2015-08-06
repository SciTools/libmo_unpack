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

#include <stdio.h>
#include "rlencode.h"
#include "wgdosstuff.h"
#include "logerrors.h"

static char message[MAX_MESSAGE_SIZE];
#define debug 0
/*
 * runlenEncode returns RL_OK if success, RL_ERR if input the number of
 * points encoded is not equal to input full length. On entry,"thinlen" must
 * be set to the size of the thinvec buffer. On exit, "thinlen" contains
 * the size of the encoded field. Missing data values represented
 * by "bmdi" are mapped into a single missing data value followed
 * by an integer value indicating the length of the run of missing data
 * values. fatvec and thinvec are the expanded (input) and compressed (output)
 * fields respectively.
 *
 */
int runlen_encode (float *fatvec, int fatlen, float *thinvec, int *thinlen, float bmdi, function* parent)
{
  int i = 0;       /* loop over expanded field */
  int nmdi = 0;       /* length of current run of mdi */
  int tmdi = 0;       /* Count of total number of mdi */
  int other = 0;       /* Count of total number of non mdi */
  int maxthinlen = *thinlen;
  float *vp = thinvec; /* pointer used to set encoded field */ 
  function subroutine;
  int log_messages=(get_verbosity()>=VERBOSITY_MESSAGE);
  set_function_name(__func__, &subroutine, parent);

  *thinlen = 0;
  for (i=0; i<fatlen; i++, fatvec++) {
    if (*fatvec == bmdi) {
      nmdi++, tmdi++;
    } else {
      // Check whether we have enough room in thinvec for what we're
      // about to store.
      if (*thinlen + 1 + 2 * (nmdi > 0) > maxthinlen) {
        return RL_ERR;
      }
      if (nmdi > 0) {
        if ((tmdi + other) <= fatlen) {
          if (log_messages) {
            snprintf(message, MAX_MESSAGE_SIZE, "adding %d mdi values", nmdi);
            MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);
          }
          *vp++ = bmdi;
          *vp++ = nmdi;
          *thinlen += 2;
          nmdi = 0;
        } else {
          return RL_ERR;
        }
      }
      *vp++ = *fatvec;
      (*thinlen)++;
      other++;
    }
  }
  if (nmdi>0) {
    *vp++ = bmdi;
    *vp++ = nmdi;
    *thinlen += 2;
  }
  if (log_messages) {
    snprintf(message, MAX_MESSAGE_SIZE, "%d MDI values in field packed out, %d words encoded", nmdi, *thinlen);
    MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);
  }
  return RL_OK;
}

/*
 * runlenDecode returns RL_OK if success, RL_ERR if the number of expanded
 * points is more than the input length "fatlen" => possible corrupt input
 * data. fatlen contains the size of expanded field on return. Missing data
 * values represented by "bmdi"  are followed by a value indicating the
 * length of the run of missing data values are expanded out. fatvec and
 * thinvec are the full (output) and compressed (input) fields respectively.
 */
int runlen_decode(float *fatvec, int fatlen, float *thinvec, int thinlen, float bmdi, function* parent)
{
  int i = 0;        /* loop over encoded field */
  int j = 0;        /* loop over current run of mdi */
  int nmdi = 0;        /* length of current run of mdi */
  int checklen = fatlen;   /* Field size which expanded data must not exceed */ 
  float *vp = fatvec;   /* pointer used to set expanded field */
  int log_messages=(get_verbosity()>=VERBOSITY_MESSAGE);
  function subroutine;
  set_function_name(__func__, &subroutine, parent);
  snprintf(message, MAX_MESSAGE_SIZE, "Started a with input data %d long, expect to get %d (when the mdi=%f)", thinlen, checklen, bmdi);
  MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);
  fatlen=0; /* start output size from 0 */

  while (i<thinlen) {
    if (*thinvec == bmdi) {
      thinvec++;
      nmdi = *thinvec;
      thinvec++;
      if (log_messages) {
        snprintf(message, MAX_MESSAGE_SIZE, "adding %d mdi values", nmdi);
        MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);
      }
      /* Check nmdi looks sensible i.e positive integer */
      if (!(nmdi >= 1 && nmdi < checklen)) {
        return RL_ERR;
      }
      i+=2;
      for (j=0; j<nmdi; j++) {
        if (++fatlen > checklen) {
          snprintf(message, MAX_MESSAGE_SIZE, "Too many values out (%d>=%d) at byte %d of packed field", fatlen, checklen, i);
          MO_syslog(VERBOSITY_ERROR, message, &subroutine);
          set_logerrno(LOGERRNO_FORMAT_EXCEPTION);
          return RL_ERR;
        }
        *vp++ = bmdi;
      }
    } else {
      if (++fatlen > checklen) {
        snprintf(message, MAX_MESSAGE_SIZE, "Too a many bytes %d>=%d at %d/%d", fatlen, checklen, i, thinlen);
        MO_syslog(VERBOSITY_ERROR, message, &subroutine);
        set_logerrno(LOGERRNO_FORMAT_EXCEPTION);
        return RL_ERR;
      }
      *vp++ = *thinvec++;
      i++;
    }
  }
  snprintf(message, MAX_MESSAGE_SIZE, "Finished with output data %d long", fatlen);
  MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);
  if (fatlen!=checklen) {
    snprintf(message, MAX_MESSAGE_SIZE, "RLE error: unpacked %d numbers, expected %d.", fatlen, checklen);
    MO_syslog(VERBOSITY_ERROR, message, &subroutine);
    set_logerrno(LOGERRNO_FORMAT_EXCEPTION);
    return RL_ERR;
  }
  return RL_OK;
}
