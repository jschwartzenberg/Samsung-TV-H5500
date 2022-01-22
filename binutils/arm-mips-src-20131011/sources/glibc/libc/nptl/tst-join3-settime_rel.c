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
#include <sys/time.h>
#include <time.h>

#include "tst-common.h"

/* Check that relative-function didn't delay.  We use a limit of 0.1 secs.  */
#define MAX_DELAY 100000000

#define _GIGA 1000000000
#define _TIMESPEC_DELTA(t_end,t_start,delta)                            \
{                                                                       \
        delta.tv_sec = (t_end.tv_sec - 1) - t_start.tv_sec;             \
        delta.tv_nsec = (t_end.tv_nsec + _GIGA) - t_start.tv_nsec;      \
        delta.tv_sec += delta.tv_nsec / _GIGA;                          \
        delta.tv_nsec = delta.tv_nsec % _GIGA;                          \
}

#define CLOCK_SHIFT	20
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


static void *
tf (void *arg)
{
  if (pthread_mutex_lock (&lock) != 0)
    {
      puts ("child: mutex_lock failed");
      return NULL;
    }

  return (void *) 42l;
}


static int
do_test (void)
{
  pthread_t th;
  struct timespec ts1,ts2,ts_real,ts_delay;

  if (pthread_mutex_lock (&lock) != 0)
    {
      puts ("mutex_lock failed");
      exit (1);
    }

  if (pthread_create (&th, NULL, tf, NULL) != 0)
    {
      puts ("mutex_create failed");
      exit (1);
    }

  void *status;
  struct timespec ts,now;
  struct timeval tv;
  (void) gettimeofday (&tv, NULL);
  TIMEVAL_TO_TIMESPEC (&tv, &now);
  ts.tv_sec = 0;
  ts.tv_nsec = 200000000;
  now.tv_sec -= CLOCK_SHIFT;
  clock_settime(CLOCK_REALTIME, &now);

  /* Start time */
  if (clock_gettime (CLOCK_REALTIME, &ts1) != 0)
  {
      puts ("first clock_gettime failed");
      exit (1);
  }

  int val = pthread_timedjoin_np_relative (th, &status, &ts);
  if (val == 0)
    {
      puts ("1st timedjoin succeeded");
      exit (1);
    }
  else if (val != ETIMEDOUT)
    {
      puts ("1st timedjoin didn't return ETIMEDOUT");
      exit (1);
    }

  if (pthread_mutex_unlock (&lock) != 0)
    {
      puts ("mutex_unlock failed");
      exit (1);
    }

  /* End time */
  if (clock_gettime (CLOCK_REALTIME, &ts2) != 0)
  {
      puts ("second clock_gettime failed");
      exit (1);
  }

  _TIMESPEC_DELTA(ts2,ts1,ts_real)
  _TIMESPEC_DELTA(ts_real,ts,ts_delay);
  /* Checks, that real time is more than timeout */
  if (ts_delay.tv_sec < 0
      || (ts_delay.tv_sec == 0 && ts_delay.tv_nsec < 0))
  {
      puts ("timeout too short");
      exit(1);
  }

  /* Checks, that real time no more than timeout + MAX_DELAY */
  if (ts_delay.tv_sec > 0
      || (ts_delay.tv_sec == 0 && ts_delay.tv_nsec > MAX_DELAY ))
  {
      puts ("pthread_timedjoin_np_relative() was delayed");
      exit(1);
  }

  while (1)
    {
      (void) gettimeofday (&tv, NULL);
      TIMEVAL_TO_TIMESPEC (&tv, &now);

      ts.tv_nsec = 200000000;
      now.tv_sec -= CLOCK_SHIFT;
      clock_settime(CLOCK_REALTIME, &now);

      /* Start time */
      if (clock_gettime (CLOCK_REALTIME, &ts1) != 0)
      {
          puts ("first clock_gettime failed");
          exit (1);
      }

      val = pthread_timedjoin_np_relative (th, &status, &ts);
      if (val == 0)
	break;

      if (val != ETIMEDOUT)
	{
	  printf ("timedjoin returned %s (%d), expected only 0 or ETIMEDOUT\n",
		  strerror (val), val);
	  exit (1);
	}

      /* End time */
      if (clock_gettime (CLOCK_REALTIME, &ts2) != 0)
      {
          puts ("second clock_gettime failed");
          exit (1);
      }

      _TIMESPEC_DELTA(ts2,ts1,ts_real)
      _TIMESPEC_DELTA(ts_real,ts,ts_delay);
      /* Checks, that real time is more than timeout */
      if (ts_delay.tv_sec < 0
          || (ts_delay.tv_sec == 0 && ts_delay.tv_nsec < 0))
      {
          puts ("timeout too short");
          exit(1);
      }

      /* Checks, that real time no more than timeout + MAX_DELAY */
      if (ts_delay.tv_sec > 0
          || (ts_delay.tv_sec == 0 && ts_delay.tv_nsec > MAX_DELAY ))
      {
          puts ("pthread_timedjoin_np_relative() was delayed");
          exit(1);
      }
    }

  if (status != (void *) 42l)
    {
      printf ("return value %p, expected %p\n", status, (void *) 42l);
      exit (1);
    }

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
