/* Copyright (C) 1991, 1995, 1996, 1997, 2001, 2004, 2006
   Free Software Foundation, Inc.
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

#include <libioP.h>
#include <stdarg.h>
#include <stdio.h>

#undef printf

#include "vd_debug.h"
#include "vd_vfprintf.h"

extern int vd_print_debug;
extern int __printf_native (const char *format, ...);

static inline void vd_printf_out_debug (void)
{
  if ( vd_print_debug & PRINT_DBG_VD_PRINTF )
  {
      pid_t pid;
      char tname[16];

      pid = getpid();
      prctl (PR_GET_NAME, (unsigned long)&tname);
      __printf_native("\n##### VD-GLIBC message, vd_printf was called :%s, pid:%d\n", tname, pid);
  }
}

/* Write formatted output to stdout from the format string FORMAT.  */
/* VARARGS1 */
int
__printf (const char *format, ...)
{
  va_list arg;
  int done;

  va_start (arg, format);
  done = vfprintf (stdout, format, arg);
  va_end (arg);

  return done;
}

int
__vd_printf (const char *format, ...)
{
  va_list arg;
  int done;

  /* VD debug message */
  vd_check_debug (&vd_print_debug, "VD_PRINT_DEBUG");
  vd_printf_out_debug ();

  va_start (arg, format);
  done = vfprintf_native (stdout, format, arg);
  va_end (arg);

  return done;
}

int
__printf_native (const char *format, ...)
{
  va_list arg;
  int done;

  va_start (arg, format);
  done = vfprintf_native (stdout, format, arg);
  va_end (arg);

  return done;
}

#undef _IO_printf
ldbl_strong_alias (__printf, printf);
ldbl_strong_alias (__vd_printf, vd_printf);
ldbl_strong_alias (__printf_native, printf_native);
/* This is for libg++.  */
ldbl_strong_alias (__printf, _IO_printf);
ldbl_strong_alias (__vd_printf, _IO_vd_printf);
ldbl_strong_alias (__printf_native, _IO_printf_native);
