/* Copyright (C) 1993-2012 Free Software Foundation, Inc.
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
#include <stdio.h>

extern int vd_print_disable;

_IO_size_t
_IO_fwrite (buf, size, count, fp)
     const void *buf;
     _IO_size_t size;
     _IO_size_t count;
     _IO_FILE *fp;
{
  _IO_size_t request = size * count;
  _IO_size_t written = 0;
  CHECK_FILE (fp, 0);
  if (request == 0)
    return 0;

  if (fp == stdout || fp == stderr)
    {
      char buf[2];
      if (vd_print_disable == 1)
         return count;
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
                  return count;
                }
              if (buf[0] == '0')
                vd_print_disable = -1;
            }
        }
    }

  _IO_acquire_lock (fp);
  if (_IO_vtable_offset (fp) != 0 || _IO_fwide (fp, -1) == -1)
    written = _IO_sputn (fp, (const char *) buf, request);
  _IO_release_lock (fp);
  /* We are guaranteed to have written all of the input, none of it, or
     some of it.  */
  if (written == request)
    return count;
  else if (written == EOF)
    return 0;
  else
    return written / size;
}
libc_hidden_def (_IO_fwrite)

#ifdef weak_alias
# include <stdio.h>
weak_alias (_IO_fwrite, fwrite)
libc_hidden_weak (fwrite)
# ifndef _IO_MTSAFE_IO
weak_alias (_IO_fwrite, fwrite_unlocked)
libc_hidden_weak (fwrite_unlocked)
# endif
#endif
