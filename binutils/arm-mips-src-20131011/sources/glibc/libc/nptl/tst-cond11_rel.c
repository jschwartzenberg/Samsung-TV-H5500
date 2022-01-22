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
#include <time.h>
#include <unistd.h>

/* Check that relative-function didn't delay.  We use a limit of 0.1 secs.  */
#define MAX_DELAY 100000000

#define _GIGA 1000000000
#define _TIMESPEC_DELTA(t_end,t_start,delta)                            \
{                                                               	\
        delta.tv_sec = (t_end.tv_sec - 1) - t_start.tv_sec;             \
        delta.tv_nsec = (t_end.tv_nsec + _GIGA) - t_start.tv_nsec;      \
        delta.tv_sec += delta.tv_nsec / _GIGA;                  	\
        delta.tv_nsec = delta.tv_nsec % _GIGA;                  	\
}

#if defined _POSIX_CLOCK_SELECTION && _POSIX_CLOCK_SELECTION >= 0
static int
run_test (clockid_t cl)
{
  pthread_condattr_t condattr;
  pthread_cond_t cond;
  pthread_mutexattr_t mutattr;
  pthread_mutex_t mut;

  printf ("clock = %d\n", (int) cl);

  if (pthread_condattr_init (&condattr) != 0)
    {
      puts ("condattr_init failed");
      return 1;
    }

  if (pthread_condattr_setclock (&condattr, cl) != 0)
    {
      puts ("condattr_setclock failed");
      return 1;
    }

  clockid_t cl2;
  if (pthread_condattr_getclock (&condattr, &cl2) != 0)
    {
      puts ("condattr_getclock failed");
      return 1;
    }
  if (cl != cl2)
    {
      printf ("condattr_getclock returned wrong value: %d, expected %d\n",
	      (int) cl2, (int) cl);
      return 1;
    }

  if (pthread_cond_init (&cond, &condattr) != 0)
    {
      puts ("cond_init failed");
      return 1;
    }

  if (pthread_condattr_destroy (&condattr) != 0)
    {
      puts ("condattr_destroy failed");
      return 1;
    }

  if (pthread_mutexattr_init (&mutattr) != 0)
    {
      puts ("mutexattr_init failed");
      return 1;
    }

  if (pthread_mutexattr_settype (&mutattr, PTHREAD_MUTEX_ERRORCHECK) != 0)
    {
      puts ("mutexattr_settype failed");
      return 1;
    }

  if (pthread_mutex_init (&mut, &mutattr) != 0)
    {
      puts ("mutex_init failed");
      return 1;
    }

  if (pthread_mutexattr_destroy (&mutattr) != 0)
    {
      puts ("mutexattr_destroy failed");
      return 1;
    }

  if (pthread_mutex_lock (&mut) != 0)
    {
      puts ("mutex_lock failed");
      return 1;
    }

  if (pthread_mutex_lock (&mut) != EDEADLK)
    {
      puts ("2nd mutex_lock did not return EDEADLK");
      return 1;
    }

  struct timespec ts;
  ts.tv_sec = 1;
  ts.tv_nsec = 0;

  /* Start time */
  struct timespec ts1;
  if (clock_gettime (cl, &ts1) != 0)
    {
      puts ("first clock_gettime failed");
      return 1;
    }

  int e = pthread_cond_timedwait_relative (&cond, &mut, &ts);
  if (e == 0)
    {
      puts ("cond_timedwait succeeded");
      return 1;
    }
  else if (e != ETIMEDOUT)
    {
      puts ("cond_timedwait did not return ETIMEDOUT");
      return 1;
    }

  /* End time */
  struct timespec ts2;
  if (clock_gettime (cl, &ts2) != 0)
    {
      puts ("second clock_gettime failed");
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
      puts ("pthread_cond_timedwait_relative() was delayed");
      return 1;
    }

  if (pthread_mutex_unlock (&mut) != 0)
    {
      puts ("mutex_unlock failed");
      return 1;
    }

  if (pthread_mutex_destroy (&mut) != 0)
    {
      puts ("mutex_destroy failed");
      return 1;
    }

  if (pthread_cond_destroy (&cond) != 0)
    {
      puts ("cond_destroy failed");
      return 1;
    }

  return 0;
}
#endif


static int
do_test (void)
{
#if !defined _POSIX_CLOCK_SELECTION || _POSIX_CLOCK_SELECTION == -1

  puts ("_POSIX_CLOCK_SELECTION not supported, test skipped");
  return 0;

#else

  int res = run_test (CLOCK_REALTIME);

# if defined _POSIX_MONOTONIC_CLOCK && _POSIX_MONOTONIC_CLOCK >= 0
#  if _POSIX_MONOTONIC_CLOCK == 0
  int e = sysconf (_SC_MONOTONIC_CLOCK);
  if (e < 0)
    puts ("CLOCK_MONOTONIC not supported");
  else if (e == 0)
    {
      puts ("sysconf (_SC_MONOTONIC_CLOCK) must not return 0");
      res = 1;
    }
  else
#  endif
    res |= run_test (CLOCK_MONOTONIC);
# else
  puts ("_POSIX_MONOTONIC_CLOCK not defined");
# endif

  return res;
#endif
}

#define TIMEOUT 3
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
