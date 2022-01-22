/* Copyright (C) 1991, 1997, 2002, 2004, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdarg.h>
#include <stdio.h>
#include <libioP.h>
#include <string.h>

#include "vd_debug.h"
#include "vd_vfprintf.h"

extern int vd_print_debug;
extern int __fprintf_native (FILE *stream, const char *format, ...);

static inline void vd_fprintf_out_debug (FILE *s)
{
  if ( vd_print_debug & PRINT_DBG_VD_FPRINTF )
  {
      pid_t pid;
      char tname[16];

      pid = getpid();
      prctl (PR_GET_NAME, (unsigned long)&tname);
      __fprintf_native (s, "\n##### VD-GLIBC message, vd_fprintf was called :%s, pid:%d\n", tname, pid);
  }
}

extern int vd_print_disable;
#define VD_PRINT_DISABLE_LEN 19
/* Write formatted output to STREAM from the format string FORMAT.  */
/* VARARGS2 */
int
__fprintf (FILE *stream, const char *format, ...)
{
  va_list arg;
  int done;

  va_start (arg, format);
  done = vfprintf (stream, format, arg);
  va_end (arg);

  return done;
}

int
__vd_fprintf (FILE *stream, const char *format, ...)
{
  va_list arg;
  int done;
  if (format && stream == stdin && !strncmp (format, "VD_PRINT_DISABLE=%d", VD_PRINT_DISABLE_LEN))
    {
      va_start (arg, format);
      int val = va_arg (arg, int);
      if (val == 0)
        vd_print_disable = -1;
      else if (val == 1)
        vd_print_disable = 1;
      va_end (arg);
      return 1;
    }

  /* VD debug message */
  vd_check_debug (&vd_print_debug, "VD_PRINT_DEBUG");
  vd_fprintf_out_debug (stream);

  va_start (arg, format);
  done = vfprintf_native (stream, format, arg);
  va_end (arg);

  return done;
}

int
__fprintf_native (FILE *stream, const char *format, ...)
{
  va_list arg;
  int done;

  va_start (arg, format);
  done = vfprintf_native (stream, format, arg);
  va_end (arg);

  return done;
}

ldbl_hidden_def (__fprintf, fprintf)
ldbl_strong_alias (__fprintf, fprintf)
ldbl_strong_alias (__vd_fprintf, vd_fprintf)
ldbl_strong_alias (__fprintf_native, fprintf_native)

/* We define the function with the real name here.  But deep down in
   libio the original function _IO_fprintf is also needed.  So make
   an alias.  */
ldbl_weak_alias (__fprintf, _IO_fprintf)
