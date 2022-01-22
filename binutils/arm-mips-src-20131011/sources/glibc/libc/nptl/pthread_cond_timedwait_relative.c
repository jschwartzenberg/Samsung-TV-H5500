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

#include <errno.h>
#include <pthread.h>
#include <pthreadP.h>

int
__pthread_cond_timedwait_relative (cond, mutex, reltime)
     pthread_cond_t *cond;
     pthread_mutex_t *mutex;
     const struct timespec *reltime;
{
  __set_errno (ENOSYS);
  return -1;
}

stub_warning(__pthread_cond_timedwait_relative)

versioned_symbol (libpthread, __pthread_cond_timedwait_relative, pthread_cond_timedwait_relative,
		  GLIBC_2_3_2);
