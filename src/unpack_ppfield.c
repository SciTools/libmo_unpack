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

// LOOKUP structure interface

// Message buffer for the syslog interface
#define MAX_MESSAGE_SIZE 1024
char message[MAX_MESSAGE_SIZE];

// DATA field structure interface
// unpack the data, calling the correct method based on the lookup associated with it

int unpack_ppfield(float mdi, int data_size, char* data, int pack, int unpacked_size, float* to, function* parent) {
  float* unpacked;
  int* ip_in;
  int* ip_out;
  int count;
  function subroutine;
  set_function_name(__func__, &subroutine, parent);
  unpacked=malloc(unpacked_size*sizeof(float));
  snprintf(message, MAX_MESSAGE_SIZE, "MDI %f", mdi);
  MO_syslog(VERBOSITY_INFO, message, &subroutine);
  switch(pack) {
  case 0:
    MO_syslog(VERBOSITY_INFO, "Unpacked data", &subroutine);
    ip_in=(int*)data;
    ip_out=(int*)unpacked;
    for (count=0; count<data_size; count++) {
      ip_out[count]=htonl(ip_in[count]);
    }
    break;
  case 1:
    MO_syslog(VERBOSITY_INFO, "WGDOS packed data", &subroutine);
    if (wgdos_unpack(data, unpacked_size, unpacked, mdi, parent)) {
      MO_syslog(VERBOSITY_INFO, "wgdos_unpack Failed", &subroutine);
      free(unpacked);
      return 1;
    }
    break;
  case 4:
    MO_syslog(VERBOSITY_INFO, "RLE packed data", &subroutine);
    ip_in=(int*)data;
    ip_out=(int*)data;
    for (count=0; count<data_size; count++) {
      ip_out[count]=htonl(ip_in[count]);
    }
    if (runlen_decode(unpacked, unpacked_size, (float*)data, data_size, mdi, &subroutine)) {
      MO_syslog(VERBOSITY_INFO, "runlen_decode Failed", &subroutine);
      free(unpacked);
      return 1;
    }
    break;
  default:
    MO_syslog(VERBOSITY_ERROR, "Unrecognised packing code", &subroutine);
    return 1;
  }
  if (to!=NULL) {
    memcpy(to, unpacked, unpacked_size*sizeof(float));
  }
  free(unpacked);
  return 0;
}

// Structure definitions
/* Required for these function calls that use the PP header information, so specific to PP fields only */

#define DATA_START 28
#define DATA_LENGTH 29
#define PACK 20
#define FIELD_LENGTH 14
#define EXT 19
#define MDI 62
#define NROWS 17
#define NCOLS 18
#define PACKED_SIZE 29
#define FC 23

int unpack_ppfield64(uint64_t* lookup, char* data, float* to, function* parent) {
  int unpacked_size;
  int data_size;
  int pack;
  float mdi;
  double dmdi;
  int ret;
  function subroutine;
  set_function_name(__func__, &subroutine, parent);

  if (sizeof(float) != 8) {
    dmdi=*(double*)(lookup+MDI);
    mdi=(float)dmdi;
  } else {
    mdi=*(float*)(lookup+MDI);
  }
  unpacked_size=lookup[NROWS]*lookup[NCOLS];
  data_size = (lookup[FIELD_LENGTH] - lookup[EXT]);
  pack=lookup[PACK] % 10;
  ret=unpack_ppfield(mdi, data_size, data, pack, unpacked_size, to, parent);
  lookup[FIELD_LENGTH]=unpacked_size + lookup[EXT];
  lookup[PACK]=0;
  return (ret);
}

int unpack_ppfield32(uint32_t* lookup, char* data, float* to, function* parent) {
  int unpacked_size;
  int data_size;
  int pack;
  float mdi;
  int ret;
  function subroutine;
  set_function_name(__func__, &subroutine, parent);

  mdi=*(float*)(lookup+MDI);
  unpacked_size=lookup[NROWS]*lookup[NCOLS];
  data_size = lookup[FIELD_LENGTH] - lookup[EXT];
  pack=lookup[PACK] % 10;
  ret=unpack_ppfield(mdi, data_size, data, pack, unpacked_size, to, parent);
  lookup[FIELD_LENGTH]=unpacked_size + lookup[EXT];
  lookup[PACK]=0;
  return (ret);
}

int byteorder_data_unpack_ppfield(float mdi, int data_size, char* data, int network_order_in,
                                  int pack, int unpacked_size, float* to, int network_order_out,
                                  function* parent) {
  int retval=0;
  int* ip_in;
  int* ip_out;
  int count;
  char* newdata=NULL;
  int temp;
  function subroutine;
  set_function_name(__func__, &subroutine, parent);
#if __BYTE_ORDER == __LITTLE_ENDIAN
  if (network_order_in) {
    switch (pack) {
    case 1:
      newdata=malloc(data_size*sizeof(int));
      ip_in=(int*)data;
      ip_out=(int*)newdata;
      for (count=0; count<data_size; count++) {
        ip_out[count]=ntohl(ip_in[count]);
      }
      break;
    default:
      newdata=data;
    }
  }
#endif

  retval=unpack_ppfield(mdi, data_size, newdata, pack, unpacked_size, to, &subroutine);

#if __BYTE_ORDER == __LITTLE_ENDIAN
  if (network_order_out && (to!=NULL)) {
    ip_in=(int*)to;
    ip_out=(int*)to;
    for (count=0; count<unpacked_size; count++) {
      ip_out[count]=htonl(ip_in[count]);
    }
  }
#endif

  if (newdata!=data) {
    free (newdata);
  }
  return retval;
}

int byteorder_unpack_ppfield(int* lookup, char* data, int network_order_in,
                             float* to, int network_order_out,
                             function* parent) {
  int retval=0;
  int unpacked_size;
  int data_size;
  int pack;
  float mdi;
  int* ip_in;
  int* ip_out;
  int count;
  char* newdata=NULL;
  int temp;
  function subroutine;
  set_function_name(__func__, &subroutine, parent);

  mdi=*(float*)(lookup+MDI);
  unpacked_size=lookup[NROWS]*lookup[NCOLS];
  data_size = lookup[FIELD_LENGTH] - lookup[EXT];
  pack=lookup[PACK] % 10;
  byteorder_data_unpack_ppfield(mdi, data_size, data, network_order_in,
                                pack, unpacked_size, to, network_order_out,
                                &subroutine);
}
