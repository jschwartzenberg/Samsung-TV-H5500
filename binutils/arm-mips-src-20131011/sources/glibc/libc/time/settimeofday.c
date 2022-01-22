/* Copyright (C) 1991, 1995, 1996, 1997, 2013 Free Software Foundation, Inc.
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

#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Set the current time of day and timezone information.
   This call is restricted to the super-user.  */

/* Only vd_settimeofday is allowed to change system time
   The settimeofday do nothing and return error */
int
__settimeofday (tv, tz)
     const struct timeval *tv;
     const struct timezone *tz;
{
  pid_t pid;
  char tname[16];

  pid = getpid();
  prctl (PR_GET_NAME, (unsigned long)&tname);
  fprintf(stderr, "\n##### VD-GLIBC message, settimeofday was called :%s, pid:%d\n", tname, pid);

  return -1;
}
hidden_def(__settimeofday)
weak_alias (__settimeofday, settimeofday)
