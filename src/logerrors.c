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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "logerrors.h"

int verbosity=VERBOSITY_ALL;  /* How much logging should be done */
int logerror=VERBOSITY_ALL; /* What is the most serious error so far */
int logerrno=LOGERRNO_NO_EXCEPTION; /* What exit code should we use on exit() */
int error_level=VERBOSITY_ERROR; /* What level do we want to throw an error on? */

void set_error_level(int val) {
  error_level=val;
}

void set_logerrno(int val) {
  if (logerrno==0) {
    logerrno=val;
  }
}

void reset_logerror(void) {
  logerror=VERBOSITY_ALL;
}

void set_logerror(int val) {
  if (logerror>val) {
    logerror=val;
  }
}

int get_logerror(void) {
  return logerror;
}

int get_verbosity(void) {
  return verbosity;
}

void set_verbosity(int val) {
  if (val<0) {
    fprintf(stderr, "Cannot set logging level to less than 0. Setting to 0.\n");
    val=0;
  }
  verbosity=val;
}

char* verbosity_string(int val) {
  switch (val) {
  case VERBOSITY_NOTHING:
    return "NOTHING";
  case VERBOSITY_ERROR:
    return "ERROR";
  case VERBOSITY_WARNING:
    return "WARNING";
  case VERBOSITY_MESSAGE:
    return "MESSAGE";
  case VERBOSITY_INFO:
    return "INFO";
  default:
    return "ALL";
  }
}

void set_function_name(const char* name, function* const caller, const function* const parent) {
  strncpy(caller->name, name, 127);
  caller->parent=(function*)parent;
}

int caller_name_is(char* name, const function* const caller) {
  return (strncasecmp(caller->name, name, 127)==0);
}

int caller_name_contains(char* name, const function* const caller) {
  char compare_name[128];
  char caller_name[128];
  char* ptr;
  strncpy(compare_name, name, 127);
  ptr=compare_name;
  while (ptr) {
    *ptr=(char)(toupper(*ptr));
  }
  strncpy(caller_name, caller->name, 127);
  ptr=caller_name;
  while (ptr) {
    *ptr=(char)(toupper(*ptr));
  }
  return (strstr(caller_name, compare_name)!=0);
}

int caller_tree_contains(char*name, const function* const caller) {
  function* parent=caller->parent;
  
  while (parent) {
    if (caller_name_is(name, parent)) {
      return (1);
    }
  }
  return 0;
}

void logerror_exit(void) {
  if (logerrno==0 && logerror<=error_level) {
    logerrno=LOGERRNO_EXCEPTION;
  }
  exit (logerrno);
}
