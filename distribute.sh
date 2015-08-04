#!/bin/sh
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

# Only used to distribute this project results to the synchronisation
# directory on the Linux Desktop.

synch_dir=${1:-/project/ukmo/rhel6/synch/share}
mkdir -p $synch_dir/lib $synch_dir/include 2>/dev/null
chmod 775 $synch_dir/lib $synch_dir/include 2>/dev/null

libverfile=`ls lib/libmo_unpack.so.[0-9].[0-9].[0-9]`

if [ -f $synch_dir/$libverfile ]
then
  echo "ERROR: Library version not updated. Installed version $libverfile." 2>&1
  echo "       Please change the rel version in the make_library script and recompile" 2>&1
  exit 1
fi

cp -d lib/lib* $synch_dir/lib
cp -d include/*.h $synch_dir/include

if [ $? -ne 0 ]
then
  echo "Failed to synchronise"
  exit 1
fi
