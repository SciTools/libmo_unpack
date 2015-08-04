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

#ifndef _LOGERRORS_H
  #define _LOGERRORS_H 1
  #define VERBOSITY_NOTHING 0
  #define VERBOSITY_ERROR 1
  #define VERBOSITY_WARNING 2
  #define VERBOSITY_INFO 3
  #define VERBOSITY_MESSAGE 4
  #define VERBOSITY_ALL 99
  #define LOGERRNO_NO_EXCEPTION 0
  #define LOGERRNO_EXCEPTION 1
  #define LOGERRNO_FORMAT_EXCEPTION 2
  #define LOGERRNO_IO_EXCEPTION 3

  typedef struct function {
    char name[128];
    struct function* parent;
  } function;

  char* verbosity_string(int verbosity);
  void reset_logerror(void);
  void set_function_name(const char* name, function* const caller, const function* const parent);
  int caller_name_is(char* name, const function* const caller);
  int caller_name_contains(char* name, const function* const caller);
  int caller_tree_contains(char* name, const function* const caller);
  void logerror_exit(void);
  void set_logerror(int val);
  int get_verbosity(void);
  void set_verbosity(int val);
  int get_logerror(void);
  void set_logerrno(int val);
  void set_error_level(int val);

  /* NOTE: Your main function must define the MO_syslog routine */
  extern void MO_syslog(int value, char* message, const function* const caller);

#endif
