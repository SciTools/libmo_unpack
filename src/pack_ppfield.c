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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#ifdef _AIX
  #include <sys/machine.h>
#elif _DARWIN_SOURCE
  #include <architecture/byte_order.h>
#else
  #include <endian.h>
#endif
#include "wgdosstuff.h"
#include "logerrors.h"
#include "rlencode.h"

// Message buffer for the syslog interface
static char message[MAX_MESSAGE_SIZE];

// DATA field structure interface
// pack the data, calling the correct method based on the lookup associated with it

/* Pack the given array if possible, returning zero and the packed data in Big Endian form (MSB first, as required by WGDOS packing),
   return nonzero and the original packed field but in Big Endian form (MSB first) so that the post-call process is identical on success
   or failure */
int pack_ppfield(float mdi, int ncols, int nrows, float* data, int pack, int bpacc, int nbits, int* packed_size, char* to, function* parent) {
  char* packed;
  int* ip_in;
  int* ip_out;
  int count;
  int retcode=0;
  int unpacked_size=nrows*ncols;
  int pack_rcode=0; /* return code from the wgdos_pack function */
  function subroutine;
  set_function_name(__func__, &subroutine, parent);

  packed=malloc(unpacked_size*sizeof(int));
  snprintf(message, MAX_MESSAGE_SIZE, "MDI %f, packing code %d", mdi, pack);
  MO_syslog(VERBOSITY_INFO, message, &subroutine);
  switch(pack) {
  case UNPACKED:
    /* No packing? Just make the numbers big endian then */
    MO_syslog(VERBOSITY_INFO, "Not packing data", &subroutine);
    ip_in=(int*)data;
    ip_out=(int*)packed;
    for (count=0; count<unpacked_size; count++) {
      ip_out[count]=htonl(ip_in[count]);
    }
    *packed_size=unpacked_size;
    /* There's no way to fail packing unless you've given it extreme rubbish */
    break;
  case WGDOS_PACKED:
    /* WGDOS packing packs as a bytestream, MSB first */
    MO_syslog(VERBOSITY_INFO, "WGDOS packing data", &subroutine);
    pack_rcode = wgdos_pack(ncols, nrows, data, mdi, bpacc, packed, packed_size, parent);
    if (pack_rcode != 0) {
      /* Couldn't pack, so remember this when exiting */
      MO_syslog(VERBOSITY_INFO, "wgdos_pack Failed", &subroutine);
      if (pack_rcode == INVALID_PACKING_ACCURACY) {
	retcode=INVALID_PACKING_ACCURACY;
      } else {
	retcode=1;
      }
    }
    break;
  case RLE_PACKED:
    /* RLE packing keeps the numbers in host order, so needs endian shift after packing */
    MO_syslog(VERBOSITY_INFO, "RLE packing data", &subroutine);
    if (runlen_encode(data, unpacked_size, (float*)packed, packed_size, mdi, &subroutine)) {
      /* Couldn't pack, so remember this when exiting */
      MO_syslog(VERBOSITY_INFO, "runlen_encode Failed", &subroutine);
      retcode=1;
    } else {
      ip_in=(int*)data;
      ip_out=(int*)packed;
      for (count=0; count<unpacked_size; count++) {
        ip_out[count]=htonl(ip_in[count]);
      }
    }
    break;
  default:
    /* Default: didn't make sense, so let the calling program know */
    MO_syslog(VERBOSITY_ERROR, "Unrecognised packing code", &subroutine);
    retcode=1;
  }

  /* If there's somewhere to pass the data back */
  if (to!=NULL) {

    if (retcode!=0) {
      /* And packing didn't work, copy the unpacked data in and make it Big Endian (MSB first) */
      ip_in=(int*)data;
      ip_out=(int*)to;
      for (count=0; count<unpacked_size; count++) {
        ip_out[count]=htonl(ip_in[count]);
      }
      *packed_size=unpacked_size;
    } else {
      /* Else, copy the correctly packed data array */
      memcpy(to, packed, (*packed_size)*sizeof(int));
    }
  }
  free(packed);
  return retcode;
}
