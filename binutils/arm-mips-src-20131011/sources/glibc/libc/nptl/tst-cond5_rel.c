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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>


static pthread_mutex_t mut;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


static int
do_test (void)
{
  pthread_mutexattr_t ma;
  int err;
  struct timespec ts;
  struct timeval tv;

  if (pthread_mutexattr_init (&ma) != 0)
    {
      puts ("mutexattr_init failed");
      exit (1);
    }

  if (pthread_mutexattr_settype (&ma, PTHREAD_MUTEX_ERRORCHECK) != 0)
    {
      puts ("mutexattr_settype failed");
      exit (1);
    }

  if (pthread_mutex_init (&mut, &ma) != 0)
    {
      puts ("mutex_init failed");
      exit (1);
    }

  /* Get the mutex.  */
  if (pthread_mutex_lock (&mut) != 0)
    {
      puts ("mutex_lock failed");
      exit (1);
    }

  ts.tv_sec = 0;
  ts.tv_nsec = 500000000;
  err = pthread_cond_timedwait_relative (&cond, &mut, &ts);
  if (err == 0)
    {
      /* This could in theory happen but here without any signal and
	 additional waiter it should not.  */
      puts ("cond_timedwait succeeded");
      exit (1);
    }
  else if (err != ETIMEDOUT)
    {
      printf ("cond_timedwait returned with %s\n", strerror (err));
      exit (1);
    }

  err = pthread_mutex_unlock (&mut);
  if (err != 0)
    {
      printf ("mutex_unlock failed: %s\n", strerror (err));
      exit (1);
    }

  return 0;
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
