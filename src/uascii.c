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

int uascii(int nchar)

/* Convert EBCDIC decimal code to ASCII decimal code */
/* Ansi C version */
/* T.Gowland IT(US) Graphics team */
/*
    Usage is thus ;

    extern int uascii();
    static int icode=38,con;
    con = uascii(icode);
    printf(" EBCDIC CODE %d ASCII CODE %d ",icode,con);

    -1 returned if input is out of range */
{
    /* Initialized data */

    static int ebcasc[256] = { 0,1,2,3,156,9,134,127,151,141,142,11,12,13,
          14,15,16,17,18,19,157,133,8,135,24,25,146,143,28,29,30,31,128,129,
          130,131,132,10,23,27,136,137,138,139,140,5,6,7,144,145,22,147,148,
          149,150,4,152,153,154,155,20,21,158,26,32,160,161,162,163,164,165,
          166,167,168,91,46,60,40,43,33,38,169,170,171,172,173,174,175,176,
          177,93,36,42,41,59,94,45,47,178,179,180,181,182,183,184,185,124,
          44,37,95,62,63,186,187,188,189,190,191,192,193,194,96,58,35,64,39,
          61,34,195,97,98,99,100,101,102,103,104,105,196,197,198,199,200,
          201,202,106,107,108,109,110,111,112,113,114,203,204,205,206,207,
          208,209,126,115,116,117,118,119,120,121,122,210,211,212,213,214,
          215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,
          231,123,65,66,67,68,69,70,71,72,73,232,233,234,235,236,237,125,74,
          75,76,77,78,79,80,81,82,238,239,240,241,242,243,92,159,83,84,85,
          86,87,88,89,90,244,245,246,247,248,249,48,49,50,51,52,53,54,55,56,
          57,250,251,252,253,254,255 };

    int ret_val;
    if(nchar < 0 || nchar > 256) 
       ret_val = -1;
    else
       ret_val = ebcasc[nchar];
    return ret_val;
}              
