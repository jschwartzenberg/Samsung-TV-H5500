/* Copyright (C) 1991, 1993, 1995, 1997, 2006 Free Software Foundation, Inc.
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
#undef	__OPTIMIZE__	/* Avoid inline `vprintf' function.  */
#include <stdio.h>
#include <libioP.h>

#undef	vprintf

#include "vd_debug.h"
#include "vd_vfprintf.h"

extern int vd_print_debug;

static inline void vd_vprintf_out_debug (void)
{
  if ( vd_print_debug & PRINT_DBG_VD_VPRINTF )
  {
      pid_t pid;
      char tname[16];

      pid = getpid();
      prctl (PR_GET_NAME, (unsigned long)&tname);
      printf_native("\n##### VD-GLIBC message, vd_vprintf was called :%s, pid:%d\n", tname, pid);
  }
}

/* Write formatted output to stdout according to the
   format string FORMAT, using the argument list in ARG.  */
int
__vprintf (const char *format, __gnuc_va_list arg)
{
  return vfprintf (stdout, format, arg);
}

int
__vd_vprintf (const char *format, __gnuc_va_list arg)
{
  /* VD debug message */
  vd_check_debug (&vd_print_debug, "VD_PRINT_DEBUG");
  vd_vprintf_out_debug ();

  return vfprintf_native (stdout, format, arg);
}
ldbl_strong_alias (__vprintf, vprintf)
ldbl_strong_alias (__vd_vprintf, vd_vprintf)
