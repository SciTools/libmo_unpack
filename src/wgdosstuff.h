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

#include <stdint.h>
#include <netinet/in.h>

#ifndef _WGDOSSTUFF_H
  #define _WGDOSSTUFF_H 1

  #define  PP_BYTES_PER_NUMERIC 4
  #define  ST_SUCCESS 1
  #define  PP_BITS_PER_NUMERIC 32

  #define UNPACKED 0
  #define WGDOS_PACKED 1
  #define RLE_PACKED 4

  #define MAX_MESSAGE_SIZE 1024

  #define INVALID_PACKING_ACCURACY 31

  #include "logerrors.h"

  #ifndef TRUE
    #define TRUE 1
    #define FALSE 0
  #endif

  typedef int Boolean;
  extern int recno;
  extern int stashno;
  extern int debug;
  extern int start_data;
  typedef struct wgdos_field_header {
    uint32_t total_length;
    uint32_t precision;
    uint16_t pts_in_row;
    uint16_t rows_in_field;
  } wgdos_field_header;

  typedef struct wgdos_row_t {
    float baseval;
    short flags;
    short len_of_data;
  } wgdos_row_t;

  int wgdos_unpack(char* packed_data, 
    int unpacked_len,
    float* unpacked_data,
    float mdi,
    function* parent);

  int wgdos_decode_row_parameters(char** data,
    float* base,
    Boolean* missing_data_present, 
    Boolean* zeros_bitmap_present,
    int* bits_per_value,
    int* nop,
    const function* const parent);

  int wgdos_decode_field_parameters(char** data,
    int unpacked_len,
    float* accuracy,
    int* ncols,
    int* nrows,
    const function* const parent);

  int extract_wgdos_row(char** packed_data,
    int ndata,
    int bits_per_value,
    void* buffer,
    int* data);

  int read_wgdos_bitmaps(char** data,
    int ncols,
    Boolean missing_data_present,
    Boolean zeros_bitmap_present,
    void* buffer,
    Boolean* missing_data,
    Boolean* zero,
    int* missing_data_count,
    int* zeros_count);

  int wgdos_expand_row_to_data(int ncols,         
    float mdi,          
    float accuracy,     
    float base,         
    Boolean* missing_data,  
    Boolean* zero,          
    int* data,           
    float* unpacked_data, 
    int* mdi_clashes,
    const function* const parent);

  int wgdos_expand_broken_row_to_data(int ncols,
    int badnumbers,     
    float mdi,          
    float accuracy,     
    float base,         
    Boolean* missing_data,  
    Boolean* zero,          
    int* data,           
    float* unpacked_data, 
    int* mdi_clashes,
    const function* const parent);

  int extract_nbit_words(void     *packed,
    int bits_per_value,
    int nitems,
    int *unpacked);

  int extract_bitmaps(void    *packed,
    int start_bit,
    int nbits,
    Boolean one_true,
    Boolean *unpacked);

  int convert_float_ibm_to_ieee32(int ibm[], int ieee[], int* n);

  int convert_float_ieee32_to_ibm(int ieee[], int ibm[], int* n);

  int unpack_ppfield(float mdi,
    int data_size,
    char* data,
    int pack,
    int unpacked_size,
    float* to,
    function* parent);

  int unpack_ppfield32(uint32_t* lookup,
    char* data,
    float* to,
    function* parent);

  int unpack_ppfield64(uint64_t* lookup,
    char* data,
    float* to,
    function* parent);

  int byteorder_data_unpack_ppfield(float mdi,
    int data_size,
    char* data,
    int network_order_in,
    int pack,
    int unpacked_size,
    float* to,
    int network_order_out,
     function* parent);

  int byteorder_unpack_ppfield(int* lookup_in,
    char* data,
    int network_order_in,
    float* to,
    int network_order_out,
    function* parent);

  int wgdos_pack(
    int ncols,
    int nrows,
    float* unpacked_data,
    float mdi,
    int bpacc,
    unsigned char* packed_data,
    int* packed_length,
    function* parent);

  int wgdos_calc_row_header(
    int* wgdos_header,
    float minval,
    int bpp,
    int npts,
    int zeros,
    int mdis,
    function* parent);

  int bitstuff(
    unsigned char* byte,
    int bitnum,
    unsigned int value,
    unsigned int nbits,
    function* parent);

  unsigned int bitsplit(
    unsigned char* byte,
    int bitnum,
    unsigned int nbits,
    function* parent);

  int pack_ppfield(
    float mdi,
    int ncols,
    int nrows,
    float* data,
    int pack,
    int bpacc,
    int nbits,
    int* packed_size,
    char* to,
    function* parent);

#endif
