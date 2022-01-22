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
#include <semaphore.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
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

#define SHIFT_VAL 60

static int
do_test (void)
{
  sem_t s;
  struct timespec ts, then;
  struct timeval tv;

  if (sem_init (&s, 0, 1) == -1)
    {
      puts ("sem_init failed");
      return 1;
    }

  if (TEMP_FAILURE_RETRY (sem_wait (&s)) == -1)
    {
      puts ("sem_wait failed");
      return 1;
    }

  if (gettimeofday (&tv, NULL) != 0)
    {
      puts ("gettimeofday failed");
      return 1;
    }
  TIMEVAL_TO_TIMESPEC (&tv, &then);

  /* We wait for half a second.  */
  ts.tv_sec = 0;
  ts.tv_nsec = 500000000;
  if (ts.tv_nsec >= 1000000000)
    {
      ++ts.tv_sec;
      ts.tv_nsec -= 1000000000;
    }

 then.tv_sec -= SHIFT_VAL;

  if (clock_settime (CLOCK_REALTIME, &then))
      errno = 0;

  /* Start time */
  struct timespec ts1;
  if (clock_gettime (CLOCK_REALTIME, &ts1) != 0)
    {
      puts ("clock_gettime failed");
      return 1;
    }

  if (TEMP_FAILURE_RETRY (sem_timedwait_relative (&s, &ts)) != -1)
    {
      puts ("sem_timedwait succeeded");
      return 1;
    }
  if (errno != ETIMEDOUT)
    {
      printf ("sem_timedwait return errno = %d instead of ETIMEDOUT\n",
	      errno);
      return 1;
    }

  /* End time */
  struct timespec ts2;
  if (clock_gettime (CLOCK_REALTIME, &ts2) != 0)
    {
      puts ("clock_gettime failed");
      return 1;
    }

  struct timespec ts_real;
  struct timespec ts_delay;
  _TIMESPEC_DELTA(ts2,ts1,ts_real)
  _TIMESPEC_DELTA(ts_real,ts,ts_delay);

  /* Checks, that real time is more than timeout */
  if (ts_delay.tv_sec < 0
      || (ts_delay.tv_sec == 0 && ts_delay.tv_nsec < 0))
    {
      puts ("timeout too short");
      return 1;
    }

  /* Checks, that real time no more than timeout + MAX_DELAY */
  if (ts_delay.tv_sec > 0
      || (ts_delay.tv_sec == 0 && ts_delay.tv_nsec > MAX_DELAY ))
    {
      puts ("sem_timedwait_relative() was delayed");
      return 1;
    }

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
