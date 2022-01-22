/* Copyright (C) 1993,1996-1999,2003,2009 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.

   As a special exception, if you link the code in this file with
   files compiled with a GNU compiler to produce an executable,
   that does not cause the resulting executable to be covered by
   the GNU Lesser General Public License.  This exception does not
   however invalidate any other reasons why the executable file
   might be covered by the GNU Lesser General Public License.
   This exception applies to code released by its copyright holders
   in files containing the exception.  */

#include "libioP.h"
#include <string.h>
#include <limits.h>

#include "vd_debug.h"
#include "../stdio-common/vd_vfprintf.h"

extern int vd_print_debug;

static inline void vd_puts_out_debug (void)
{
  if ( vd_print_debug & PRINT_DBG_VD_PUTS )
  {
      pid_t pid;
      char tname[16];

      pid = getpid();
      prctl (PR_GET_NAME, (unsigned long)&tname);
      printf_native ("\n##### VD-GLIBC message, vd_puts was called :%s, pid:%d\n", tname, pid);
  }
}

extern int vd_print_disable;
static int _IO_puts_native (const char *str);

int
_IO_puts (str)
     const char *str;
{
  char buf[2];

  if (vd_print_disable == 1)
     return 0;
  if (vd_print_disable == 0)
    {
      int errno_backup = errno;
      if (getenv_r ("VD_PRINT_DISABLE", buf, sizeof (buf)))
	{
          vd_print_disable = -1;
	  /* Do not corrupt "errno" in case of getenv_r fails */
	  __set_errno (errno_backup);
	}
      else
        {
          if (buf[0] == '1')
            {
              vd_print_disable = 1;
              return 0;
            }
          if (buf[0] == '0')
            vd_print_disable = -1;
        }
    }
    return _IO_puts_native (str);
}

int
_vd_IO_puts (str)
     const char *str;
{
  /* VD debug message */
  vd_check_debug (&vd_print_debug, "VD_PRINT_DEBUG");
  vd_puts_out_debug ();

  return _IO_puts_native(str);
}

int
_IO_puts_native (str)
     const char *str;
{
  int result = EOF;
  _IO_size_t len = strlen (str);
  _IO_acquire_lock (_IO_stdout);

  if ((_IO_vtable_offset (_IO_stdout) != 0
       || _IO_fwide (_IO_stdout, -1) == -1)
      && _IO_sputn (_IO_stdout, str, len) == len
      && _IO_putc_unlocked ('\n', _IO_stdout) != EOF)
    result = MIN (INT_MAX, len + 1);

  _IO_release_lock (_IO_stdout);
  return result;
}

#ifdef weak_alias
weak_alias (_IO_puts, puts)
weak_alias (_vd_IO_puts, vd_puts)
#endif
