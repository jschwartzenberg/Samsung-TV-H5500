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

#include <stdint.h>
# define TIMEVAL_TO_TIMESPEC(tv, ts) {                                   \
	        (ts)->tv_sec = (tv)->tv_sec;                                    \
	        (ts)->tv_nsec = (tv)->tv_usec * 1000;                           \
}
# define TIMESPEC_TO_TIMEVAL(tv, ts) {                                   \
	        (tv)->tv_sec = (ts)->tv_sec;                                    \
	        (tv)->tv_usec = (ts)->tv_nsec / 1000;                           \
}
# define TEMP_FAILURE_RETRY(expression) \
	  (__extension__							      \
	       ({ long int __result;						      \
			       do __result = (long int) (expression);				      \
			       while (__result == -1L && errno == EINTR);			      \
			       __result; }))

#define PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP \
	  { { 0, 0, 0, 0, 0, 0, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP, \
			      0, 0, 0, 0 } }
#define PTHREAD_MUTEX_NORMAL 0
