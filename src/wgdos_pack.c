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
/* wgdos_pack.c
 * 
 * Description:
 *   Pack data to a WGDOS packed field
 * Revision 2.0  2012/08/28 12:19:51  hadmf
 * SRCE checkin. Type: Undef
 * Reason: First cut into libmo_unpack
 *
 * Revision 2.1  2012/09/17  hadmf
 * SRCE checkin. Type: Undef
 * Reason: Reordering code, fixing bugs and testing compatability with genuine data and adding comments
 *
 * Revision 2.2  2014/11/27  hshep
 * SRCE checkin. Type: Update
 * Reason: Bug fix in the initial bitshifting used to pack a row of data
 *
 * Revision 2.2.1  2014/12/08  hshep
 * SRCE checkin. Type: Update
 * Reason: Add return code 31, and bug fix for integer overflow when calculating
 *         spread
 * Information:
 */

/* Standard header files used */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
/* Package header files used */
#include "wgdosstuff.h"
#include "logerrors.h"

char message[MAX_MESSAGE_SIZE];
/* End of header */


/* Count up the zeros in the row, no zeros returned means no zero mapping */
int count_zeros(int ncols, float* row_data, function* parent) {
  int zeros_count=0;
  int i;
  float min=row_data[0];
  float max=row_data[0];
  function subroutine;
  set_function_name(__func__, &subroutine, parent);

  /* Find out if we need to use zero bitmaps */
  for (i=0;i<ncols;i++) {
    if(row_data[i]==0) {
      zeros_count++;
    } else {
      if (min>row_data[i]) min=row_data[i];
      if (max<row_data[i]) max=row_data[i];
    }
  }

  /* Just making up an algorithm about whether to store data with zeros mapped out.
     Since I don't know the accuracy, take it as 50-50 chance if the range is sqrt(2)
     bigger including zeros than excluding them and this is good enough */
  if (zeros_count>0 && min>0.0) {
    if ((max-min)>(max/sqrt(2))) zeros_count=0;  /* turn off zero packing */
  }
  return zeros_count;
}

/* Fill the bitmap denoting when the given bitmap_value appears along with an easier to parse integer array replication
   if the value "true" is true, then bitmaps are true bit maps (1 = True). If "true" is false, then the bitmap is inverted */
int fill_bitmap(int ncols, float* row_data, float bitmap_value, int true, unsigned char* bitmap, int* array, function* parent) {
  int i,j;
  int count=0;
  unsigned char byte;
  function subroutine;
  set_function_name(__func__, &subroutine, parent);

  for (i=0;i<ncols;i+=8) {
    byte=0;
    for (j=0;j<8;j++) {
      if(i+j==ncols) {
        byte=byte<<(8-j);
        break; /* Stop counting at the end of the row */
      }
      byte=byte<<1;
      if(row_data[i+j]==bitmap_value) {
        byte+=1;
        count++;
        array[i+j]=1;
      } else {
        array[i+j]=0;
      }
    }
    if (true==1) {
      bitmap[i/8]=byte;
    } else {
      /* Invert the logic if true==false */
      bitmap[i/8]=~byte;
    }
  }
  return count;
}

/* Pack a 2-D field of floating point numbers stored linearly, storing the data as
   a bytestream (MSB first). If packing fails, return a nonzero code */
int wgdos_pack(
    int       ncols,                 /* Number of columns in each row */
    int       nrows,                 /* Number of rows in field */
    float*    unpacked_data,         /* Data to pack */
    float     mdi,                   /* Missing data indicator value */
    int       bpacc,                 /* WGDOS packing accuracy */
    unsigned char* packed_data,      /* Packed data */
    int*      packed_length,         /* Packed data length */
    function* parent)
{
  float accuracy;              /* Absolute accuracy to which data held */
  float minval, maxval;
  unsigned char* mdi_bitmap;   /* Missing data bitmap for current row as bitstream */
  int* mdi_array;              /* Integer array representation of mdi_bitmap 1=TRUE*/
  unsigned char* zero_bitmap;  /* Zeros bitmap for current row as bitstream*/
  int* zero_array;             /* Integer array representation of zero_bitmap 1=TRUE*/
  int row;                     /* Number of rows transmitted so far */
  int mdis_count;              /* Number of missing data elements */
  int zeros_count;             /* Number of bitmapped zeros */
  int ndata;                   /* Number of non-bitmapped data items */
  unsigned int spread;         /* Spread of values in each row */
  float f_spread;              /* A floating point value of spread */
  float epsilon_spread;        /* Accuracy for floating point comparison of
				  spread */
  int bpp;                     /* Number of bits required to store the packed values */
  int bitmap_size;

  /* wgdos field/row constituents*/
  wgdos_field_header* wgdos_field_header_pointer; /* Where the field header will be filled in */
  float* row_data;             /* Spare location to write the row as its being constructed */
  int wgdos_row_header[2];     /* Spare location to write the row header as its being constructed */
  int wgdos_field_header[3];   /* Spare location to write the field header as its being constructed */

  unsigned char* packed_row;   /* Pointer to the packed row being calculated */
  int offset;                  /* Number of bytes into output field to write the next set of bytes */
  unsigned int digits;         /* Integer equivalent to the row data after compression */
  int size_of_packed_field;
  int size_of_packed_row;
  int first_value;
  int i,j;
  int log_message = (get_verbosity()>=VERBOSITY_MESSAGE);  /* Used to reduce the number of sprintf calls for loggin */

  function subroutine;
  set_function_name(__func__, &subroutine, parent);

  if (ncols <= 1) {
    MO_syslog(VERBOSITY_ERROR, "Not a two-dimensional field. Cannot pack.", &subroutine);
    return 1;
  }

  epsilon_spread = 0.00001;
  wgdos_field_header_pointer=(void*)packed_data;
  bitmap_size=(ncols+7)/8;
  size_of_packed_field=0;
  accuracy=powf(2.0, (float)bpacc);

  /* Reserve work areas */
  row_data = (float *) malloc(sizeof(float)  * ncols);
  zero_bitmap = (unsigned char*) malloc(bitmap_size);
  mdi_bitmap = (unsigned char*) malloc(bitmap_size);
  packed_row = (unsigned char*) malloc(sizeof(int) * ncols);
  mdi_array = (int*) malloc(sizeof(int) * ncols);
  zero_array = (int*) malloc(sizeof(int) * ncols);

  /* The offset (size of field so far) already skips the field header */
  offset=sizeof(wgdos_field_header);

  /* For each row */
  for (row=0;row<nrows;row++) {

    /* Clean up counts, bitmaps etc */
    zeros_count=0;
    mdis_count=0;
    size_of_packed_row=0;
    memset(mdi_bitmap, 0, bitmap_size);
    memset(zero_bitmap, 0, bitmap_size);
    memset(packed_row, 0, ncols * sizeof(int));

    snprintf(message, MAX_MESSAGE_SIZE, "Row %d size of packed field at start %d", row, size_of_packed_field);
    MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);
    first_value=1;

    /* create the zeros bitmap */
    zeros_count=count_zeros(ncols, &unpacked_data[row*ncols], &subroutine);
    if (zeros_count>0) {
      zeros_count=fill_bitmap(ncols, &unpacked_data[ncols*row], 0.0, 0, zero_bitmap, zero_array, &subroutine);
    }
    /* create the MDI bitmap */
    mdis_count=fill_bitmap(ncols, &unpacked_data[ncols*row], mdi, 1, mdi_bitmap, mdi_array, &subroutine);

    /* collect the remaining data values*/
    ndata=0;
    for (i=0;i<ncols;i++) {
      if (mdi==unpacked_data[i+ncols*row]) {
        /* Skip mdis, we're going to bitmap them out */
      } else if (zeros_count && (0.0==unpacked_data[i+ncols*row])) {
        /* Skip zeros if we're going to bitmap them out */
      } else {
        row_data[ndata]=unpacked_data[i+ncols*row];
        if (first_value) {
          minval=row_data[ndata];
          maxval=row_data[ndata];
          first_value=0;
        }
        if (row_data[ndata]<minval) minval=row_data[ndata];
        if (row_data[ndata]>maxval) maxval=row_data[ndata];
        ndata++;
      }
    }
    if ((mdis_count+zeros_count)==ncols) minval=maxval;

    /* Calculate the number of bits required to contain the interval at the required accuracy */
    spread=(maxval-minval)/accuracy;
    spread+=((maxval-minval)>=accuracy);
    /* It is possible to get an error where the value of spread becomes far
       too large for the capacity of an unsigned  integer. Therefore
       we check that spread is smaller than the max value for an unsigned
       integer ie. 4294967295 */
    f_spread = (maxval-minval)/accuracy;
    f_spread += (float) ((maxval-minval)>=accuracy);
    if (f_spread <= (float)UINT_MAX-epsilon_spread) {
      for (bpp=0;spread;spread=spread>>1,bpp++) {/*nothing*/}
    } else {
      //override to failure
      bpp = 32;
    }
    if (bpp>31) {
      snprintf(message, MAX_MESSAGE_SIZE, "Data spread over the row (%f - %f)too large to manage at this accuracy (%f)", minval, maxval, accuracy);
      MO_syslog(VERBOSITY_ERROR, message, &subroutine);
      return INVALID_PACKING_ACCURACY;
    }

    snprintf(message, MAX_MESSAGE_SIZE, "scale %d min %f max %f accuracy %f bpacc %d ndata %d bpp %d", spread, minval, maxval, accuracy, bpacc,ndata, bpp);
    MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);

    /* Pack the data as a bitstream of integers as per WGDOS packing scheme */
    message[0]=0;
    for (i=0,ndata=0;i<ncols;i++) {
      if(mdi_array[i]) {
        if (log_message) {
          snprintf(message+strlen(message), MAX_MESSAGE_SIZE-strlen(message), "%012g / %-12s ", mdi, "MDI");
        }
      } else if (zeros_count && zero_array[i]){
        if (log_message) {
          snprintf(message+strlen(message), MAX_MESSAGE_SIZE-strlen(message), "%012g / %-12s ", 0.0, "Zero");
        }
      } else {
        /* Calculate the packed integer equivalent of the row data*/
        digits=(row_data[ndata]-minval)/accuracy;
        if (log_message) {
          snprintf(message+strlen(message), MAX_MESSAGE_SIZE-strlen(message), "%012g / %-12d ", row_data[ndata], digits);
        }
        /* Stuff these bits into the row data spare space */
        bitstuff(packed_row, ndata*bpp, digits, bpp, &subroutine);
        /* Move on to the next value */
        ndata++;
      }
      /* If we log stuff out, print out four values per line to get clean logging output */
      if (i%4==3 && log_message) {
        snprintf(message+strlen(message), MAX_MESSAGE_SIZE-strlen(message), "%3d",i);
        MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);
        message[0]=0;
      }
    }

    /* If we logged, we may not have finished on a four-word boundary */
    if (message[0]!=0 && log_message) {
      snprintf(message+strlen(message), MAX_MESSAGE_SIZE-strlen(message), "%3d",i);
      MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);
      message[0]=0;
    }

    /* Calculate the WGDOS row header for this row now we have all the info */
    wgdos_calc_row_header(wgdos_row_header, minval, bpp, ncols, zeros_count, mdis_count, &subroutine);

    /* finally, pack all this info into the field */

    /* first the row header */
    memcpy(&packed_data[offset], wgdos_row_header, sizeof(wgdos_row_header));
    offset+=sizeof(wgdos_row_header);

    /* then the MDI bitmap */
    if (mdis_count) {
      memcpy(&packed_data[offset], mdi_bitmap, bitmap_size);
      offset+=bitmap_size;
    }

    /* Then the zero bitmap */
    if (zeros_count) {
      memcpy(&packed_data[offset], zero_bitmap, bitmap_size);
      offset+=bitmap_size;
    }

    /* Round off to the next word boundary */
    offset=4*((offset+3)/4);

    /* lastly the data */
    memcpy(&packed_data[offset], packed_row, 4*((bpp*ndata+31)/32));
    size_of_packed_row+=((bpp*ndata+31)/32);
    offset+=(4*size_of_packed_row);
    snprintf(message, MAX_MESSAGE_SIZE, "Field length on row %d is %d packed row size %d",row, offset, size_of_packed_row*4);
    MO_syslog(VERBOSITY_INFO, message, &subroutine);
  }

  /* size of packed field is in bytes. Neet to store it as 32-bit words for WGDOS */
  size_of_packed_field+=(offset/4);

  snprintf(message, MAX_MESSAGE_SIZE, "precision %d ncols %d nrows %d length %d\n", bpacc, ncols, nrows, size_of_packed_field);
  MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);

  /* Fill in the WGDOS field header at the beginning of the field */
  wgdos_field_header_pointer->total_length=htonl(size_of_packed_field);
  wgdos_field_header_pointer->precision=htonl(bpacc);
  wgdos_field_header_pointer->pts_in_row=htons(ncols);
  wgdos_field_header_pointer->rows_in_field=htons(nrows);
  *packed_length=size_of_packed_field;

  snprintf(message, MAX_MESSAGE_SIZE, "Packed field size %d", size_of_packed_field);
  MO_syslog(VERBOSITY_INFO, message, &subroutine);

  /* Free the work areas */
  free(row_data);  
  free(zero_bitmap);  
  free(mdi_bitmap);
  return 0;
}

/* A routine written to test the bitstuff routine works properly */
static int test1_in[]={20,4,0,3,30,11,12,12};
static char test1_out[]={161,0,63,45,150};
static int test2_in[]={921,91,2491,1001,3275};
static char test2_out[]={57,144,91,155,179,233,204,176};
static unsigned char test_buffer[72];
int test_bitstuff() {
  int i,bitnumber, worked;
  for (i=0,bitnumber=0;i<8;i++,bitnumber+=5) {
    bitstuff(test_buffer, bitnumber, test1_in[i], 5, NULL);
  }
  worked=(0==memcmp(test_buffer, test1_out, 5));
  memset(test_buffer, 0, 72);
  for (i=0,bitnumber=0;i<5;i++,bitnumber+=12) {
    bitstuff(test_buffer, bitnumber, test2_in[i], 12, NULL);
  }
  worked+=2*(0==memcmp(test_buffer, test2_out, 8));
  return (worked);
}

/* The packing version of wgdos_pack_polybits */
int bitstuff(unsigned char* byte, int bitnum, unsigned int value, unsigned int nbits, function* parent) {
  unsigned long long int shifted; /* The bitshifted value, needs to be definitely bigger than an unsigned int32 */
  int bitshift;        /* How many bits need to get shifted to insert the next value correctly */
  int bytes_used;      /* How many bytes out of shifted were used to store the value */
  int base_byte;       /* The byte offset that we start putting bits from the value passed in to */
  int n;

  function subroutine;
    
  set_function_name(__func__, &subroutine, parent);

  /* Just make sure we don't store more than can be fitted in */
  if (nbits>31) {
    snprintf(message, MAX_MESSAGE_SIZE, "bpp value out of range (%d)",nbits);
    MO_syslog(VERBOSITY_ERROR, message, &subroutine);
    return (-1);
  }

  /* And another belt-and-braces check */
  if (value>=(1<<nbits)) {
    snprintf(message, MAX_MESSAGE_SIZE, "Value %d too large for a %d bit number",value, nbits);
    MO_syslog(VERBOSITY_ERROR, message, &subroutine);
    return (-1);
  }

  bitshift=8-(bitnum+nbits)%8;  /* How many bits to shift value so it occupies the right byte positions */
  shifted=(unsigned long long int)value << bitshift; /* Cast value to long long int before shifting otherwise we run out of bits */
  base_byte=bitnum/8;  /* The byte that the first (highest) byte in shifted needs to go in to */
  bytes_used=(nbits+bitshift+7)/8;  /* How many bytes used (nbits are shifted up by bitshift, so add them) */

  /* now stuff each used byte from shifted into the bytestream in the appropriate location */
  for (n=0;n<bytes_used;n++) {
    byte[n+base_byte]+=shifted>>(8*(bytes_used-n-1));  /* first byte needs highest byte of shifted put in it */
  }
  return(0);
}

/* Not used here, but ought to produce the opposite effect of bitstuff */
unsigned int bitsplit(unsigned char* byte, int bitnum, unsigned int nbits, function* parent) {
  unsigned long long int shifted;
  int bitshift;
  int n;
  int bytes_used;
  int base_byte;
  int mask;
  function subroutine;
    
  set_function_name(__func__, &subroutine, parent);
  if (nbits>31) {
    snprintf(message, MAX_MESSAGE_SIZE, "bpp out of range (%d)",nbits);
    MO_syslog(VERBOSITY_ERROR, message, &subroutine);
    return (-1);
  }
  mask=(1<<nbits)-1;  /* Mask out bits from any earlier numbers in the bitstream */
  bitshift=8-(bitnum+nbits)%8;  /* How many bits to shift value so it occupies the right byte positions */
  shifted=0;
  base_byte=bitnum/8; /* The byte that the first (highest) byte in shifted needs to come from */
  bytes_used=(nbits+bitshift+7)/8;  /* How many bytes used (nbits are shifted up by bitshift, so add them) */
  for (n=0;n<bytes_used;n++) {
    shifted=shifted<<8;  /* Any earlier bytes read go higher up the shifted number */
    shifted+=byte[n+base_byte]; /* and add the number from this byte */
  }
  return(mask&(shifted>>bitshift));  /* Shift back to remove later bits, mask to hide earier ones */
}

/* Calculate the entries in the rather overcomplex WGDOS row header */
int wgdos_calc_row_header(int* wgdos_header, float minval, int bpp, int npts, int zeros, int mdis, function* parent) {
  int one=1;   /* Needed because of the fortran-compatible convert_float... call takes pointers */
  int header;  /* The 32 bits of the row header, ready to run ntohl on... */
  int mapsize=(npts+7)/8;
  int size_of_row=0;
  union {
    int i;
    float f;
  } basetemp;  /* Used to convert floats to big endian floats */

  function subroutine;

  set_function_name(__func__, &subroutine, parent);

  /* WGDOS requires IBM floats not IEEE and they need to be Big Endian. Make it so... */
  basetemp.f=minval;
  basetemp.i=htonl(basetemp.i);
  convert_float_ieee32_to_ibm((int*)&minval, &basetemp.i, &one);
  basetemp.i=htonl(basetemp.i);

  npts-=(zeros+mdis);
  snprintf(message, MAX_MESSAGE_SIZE, "Zero: %d MDI: %d(%d) %d bits per value base value %f %d words taken for %d values", (zeros>0), (mdis>0), mdis, bpp, minval, (bpp*npts+31)/32, npts);
  MO_syslog(VERBOSITY_MESSAGE, message, &subroutine);

  /* Do the rest of the header flags, etc */
  header=0;
  if (zeros>0) header+=128;
  if (mdis>0) header+=32;
  header+=(bpp&0x1f);
  header=header<<16;
  size_of_row=((bpp*npts+31)/32);
  size_of_row+=(((zeros>0)+(mdis>0))*mapsize+3)/4;
  header+=size_of_row;
  header=htonl(header);

  /* fill in the two-word array that is the WGDOS row header */
  wgdos_header[0]=basetemp.i;
  wgdos_header[1]=header;

  return 0;
}
