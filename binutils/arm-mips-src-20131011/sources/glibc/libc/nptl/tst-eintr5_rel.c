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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "eintr.c"


static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t c = PTHREAD_COND_INITIALIZER;


static void *
tf (void *arg)
{
  struct timespec ts;

  ts.tv_sec = 10000;
  ts.tv_nsec = 0;
  /* This call must never return.  */
  int e = pthread_cond_timedwait_relative (&c, &m, &ts);
  char buf[100];
  printf ("tf: cond_timedwait_relative returned: %s\n",
	  strerror_r (e, buf, sizeof (buf)));

  exit (1);
}


static int
do_test (void)
{
  setup_eintr (SIGUSR1, NULL);

  pthread_t th;
  char buf[100];
  int e = pthread_create (&th, NULL, tf, NULL);
  if (e != 0)
    {
      printf ("main: pthread_create failed: %s\n",
	      strerror_r (e, buf, sizeof (buf)));
      exit (1);
    }

  /* This call must never return.  */
  e = pthread_cond_wait (&c, &m);
  printf ("main: cond_wait returned: %s\n",
	  strerror_r (e, buf, sizeof (buf)));

  return 0;
}

#define EXPECTED_SIGNAL SIGALRM
#define TIMEOUT 3
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
