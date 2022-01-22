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
#include <error.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
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
#define _TIMESPEC_ADD(t_start,delta,t_end)                            					\
{                                                                       				\
	t_end.tv_sec = t_start.tv_sec + delta.tv_sec + (t_start.tv_nsec + delta.tv_nsec) / _GIGA;	\
	t_end.tv_nsec = (t_start.tv_nsec + delta.tv_nsec ) % _GIGA;					\
}
#define _TIMESPEC_CHECK_NEGATIVE(t_delta)                               \
     ((t_delta.tv_sec < 0) || ((t_delta.tv_sec==0) && (t_delta.tv_nsec < 0)))

#define NWRITERS 15
#define WRITETRIES 10
#define NREADERS 15
#define READTRIES 15

#define CLOCK_SHIFT 20
#define TIMEOUT 1000000
#define DELAY   1000000

#ifndef INIT
# define INIT PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP
#endif

static pthread_rwlock_t lock = INIT;


static void *
writer_thread (void *nr)
{
  struct timespec ts,now;
  struct timespec delay;
  struct timespec ts1,ts2,ts_real;
  struct timespec ts_max_delay,ts_rel,ts_overtime;
  int n;

  delay.tv_sec = 0;
  delay.tv_nsec = DELAY;

  for (n = 0; n < WRITETRIES; ++n)
    {
      int e;
      do
	{
	  struct timeval tv;
	  (void) gettimeofday (&tv, NULL);
	  TIMEVAL_TO_TIMESPEC (&tv, &now);

	  ts.tv_nsec = 2 * TIMEOUT;
	  ts.tv_sec = 0;
	  printf ("writer thread %ld tries again\n", (long int) nr);
	  now.tv_sec -= CLOCK_SHIFT;
	  clock_settime(CLOCK_REALTIME, &now);

          /* Start time */
          if (clock_gettime (CLOCK_REALTIME, &ts1) != 0)
          {
              puts ("first clock_gettime failed");
              exit (1);
          }

	  e = pthread_rwlock_timedwrlock_relative (&lock, &ts);
	  if (e != 0 && e != ETIMEDOUT)
	    {
	      puts ("timedwrlock failed");
	      exit (1);
	    }

          /* End time */
          if (clock_gettime (CLOCK_REALTIME, &ts2) != 0)
          {
              puts ("second clock_gettime failed");
              exit (1);
          }

          /* Checks, that end time is later than start time */
          _TIMESPEC_DELTA(ts2,ts1,ts_real)
          if (_TIMESPEC_CHECK_NEGATIVE(ts_real) ) {
              puts ("pthread_rwlock_timedwrlock_relative() has bad real runtime");
              exit(1);
          }

          /* Checks, that function performs within a timeout */
	  ts_max_delay.tv_sec = 0;
          ts_max_delay.tv_nsec = MAX_DELAY;
          _TIMESPEC_ADD(ts,ts_max_delay,ts_rel)
	  _TIMESPEC_DELTA(ts_rel,ts_real,ts_overtime)
	  if (_TIMESPEC_CHECK_NEGATIVE(ts_overtime) )  {
              puts ("pthread_rwlock_timedwrlock_relative() was delayed");
              exit(1);
          }
	}
      while (e == ETIMEDOUT);

      printf ("writer thread %ld succeeded\n", (long int) nr);

      nanosleep (&delay, NULL);

      if (pthread_rwlock_unlock (&lock) != 0)
	{
	  puts ("unlock for writer failed");
	  exit (1);
	}

      printf ("writer thread %ld released\n", (long int) nr);
    }

  return NULL;
}


static void *
reader_thread (void *nr)
{
  struct timespec ts, now;
  struct timespec delay;
  struct timespec ts1,ts2,ts_real;
  struct timespec ts_max_delay,ts_rel,ts_overtime;
  int n;

  delay.tv_sec = 0;
  delay.tv_nsec = DELAY;

  for (n = 0; n < READTRIES; ++n)
    {
      int e;
      do
	{
	  struct timeval tv;
	  (void) gettimeofday (&tv, NULL);
	  TIMEVAL_TO_TIMESPEC (&tv, &now);

	  ts.tv_sec = 0;
	  ts.tv_nsec = TIMEOUT;
	  printf ("reader thread %ld tries again\n", (long int) nr);
	  now.tv_sec -= CLOCK_SHIFT;
	  clock_settime(CLOCK_REALTIME, &now);

          /* Start time */
          if (clock_gettime (CLOCK_REALTIME, &ts1) != 0)
          {
              puts ("first clock_gettime failed");
              exit (1);
          }

	  e = pthread_rwlock_timedrdlock_relative (&lock, &ts);
	  if (e != 0 && e != ETIMEDOUT)
	    {
	      puts ("timedrdlock failed");
	      exit (1);
	    }

          /* End time */
          if (clock_gettime (CLOCK_REALTIME, &ts2) != 0)
          {
              puts ("second clock_gettime failed");
              exit (1);
          }

          /* Checks, that end time is later than start time */
          _TIMESPEC_DELTA(ts2,ts1,ts_real)
          if (_TIMESPEC_CHECK_NEGATIVE(ts_real) ) {
              puts ("pthread_rwlock_timedrdlock_relative() has bad real runtime");
              exit(1);
          }

	  /* Checks, that function performs within a timeout */
	  ts_max_delay.tv_sec = 0;
          ts_max_delay.tv_nsec = MAX_DELAY;
          _TIMESPEC_ADD(ts,ts_max_delay,ts_rel)
	  _TIMESPEC_DELTA(ts_rel,ts_real,ts_overtime)
	  if (_TIMESPEC_CHECK_NEGATIVE(ts_overtime) )  {
              puts ("pthread_rwlock_timedrdlock_relative() was delayed");
              exit(1);
          }
	}
      while (e == ETIMEDOUT);

      printf ("reader thread %ld succeeded\n", (long int) nr);

      nanosleep (&delay, NULL);

      if (pthread_rwlock_unlock (&lock) != 0)
	{
	  puts ("unlock for reader failed");
	  exit (1);
	}

      printf ("reader thread %ld released\n", (long int) nr);
    }

  return NULL;
}


static int
do_test (void)
{
  pthread_t thwr[NWRITERS];
  pthread_t thrd[NREADERS];
  int n;
  void *res;

  /* Make standard error the same as standard output.  */
  dup2 (1, 2);

  /* Make sure we see all message, even those on stdout.  */
  setvbuf (stdout, NULL, _IONBF, 0);

  for (n = 0; n < NWRITERS; ++n)
    if (pthread_create (&thwr[n], NULL, writer_thread,
			(void *) (long int) n) != 0)
      {
	puts ("writer create failed");
	exit (1);
      }

  for (n = 0; n < NREADERS; ++n)
    if (pthread_create (&thrd[n], NULL, reader_thread,
			(void *) (long int) n) != 0)
      {
	puts ("reader create failed");
	exit (1);
      }

  /* Wait for all the threads.  */
  for (n = 0; n < NWRITERS; ++n)
    if (pthread_join (thwr[n], &res) != 0)
      {
	puts ("writer join failed");
	exit (1);
      }
  for (n = 0; n < NREADERS; ++n)
    if (pthread_join (thrd[n], &res) != 0)
      {
	puts ("reader join failed");
	exit (1);
      }

  return 0;
}

#undef TIMEOUT
#define TIMEOUT 30
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
