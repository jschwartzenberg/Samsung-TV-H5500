/* Copyright (C) 2012-2013 Free Software Foundation, Inc.
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

/* Reentrant version of getenv function */

#include <endian.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "env_r.h"

/* Reentrant version of getenv function */

int
getenv_r (const char *name, char *buf, int buflen)
{
  int i, len, olen;

  LOCK;

  if (__environ == NULL || name[0] == '\0')
    goto error;

  len = strlen (name);
  for (i = 0; environ[i] != NULL; i++)
    {
      if ((strncmp(name, environ[i], len) == 0) && (environ[i][len] == '='))
        {
          olen = strlen (&environ[i][len+1]);
          if (olen >= buflen)
            {
              UNLOCK;
              __set_errno (ENOSPC);
              return -1;
            }
          strcpy (buf, &environ[i][len+1]);
          UNLOCK;
          return 0;
        }
      }

error:
  UNLOCK;
  __set_errno (ENOENT);
  return -1;
}
# undef getenv_r
weak_alias (__getenv_r, getenv_r)
