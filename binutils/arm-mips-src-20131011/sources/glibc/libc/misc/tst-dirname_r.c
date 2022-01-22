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

#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define BUF_LEN 32
#define TH_COUNT 2

char buf[BUF_LEN];
int th_ret[TH_COUNT];
pthread_t thr[TH_COUNT];

void *th_func(void *arg)
{
  int retval, i;
  pthread_t id = pthread_self();
  char out_buf[BUF_LEN];
  retval = dirname_r (buf, out_buf, BUF_LEN);
  if(retval == -1)
    goto exit;
  retval = strcmp (out_buf, arg);
  if (retval)
    printf ("dirname_r(\"%s\") should be \"%s\", but is \"%s\"\n",
	    buf, (char *)arg, out_buf);
  exit:
  for(i = 0; i < TH_COUNT; i++)
    {
      if(pthread_equal(id, thr[i]))
        {
          th_ret[i] = retval;
          pthread_exit (&th_ret[i]);
        }
    }
  return NULL;
}

static int
test (const char *input, const char *result)
{
  int retval, res = 0, i;
  memset (th_ret, 0, sizeof(int) * TH_COUNT);
  strcpy (buf, input);
  for(i = 0; i < TH_COUNT; i++)
    {
      retval = pthread_create (&thr[i], NULL, th_func, (void *) result);
      if (retval)
        {
          printf("Error creating new thread %d\n", i);
          return retval;
        }
    }

  for (i = 0; i < TH_COUNT; i++)
    {
      retval = pthread_join (thr[i], NULL);
      if(retval || th_ret[i])
        res = -1;
    }
  return res;
}

int
main (void)
{
  int result = 0;

  /* These are the examples given in XPG4.2.  */
  result |= test ("/usr/lib", "/usr");
  result |= test ("/usr/", "/");
  result |= test ("usr", ".");
  result |= test ("/", "/");
  result |= test (".", ".");
  result |= test ("..", ".");

  /* Some more tests.   */
  result |= test ("/usr/lib/", "/usr");
  result |= test ("/usr", "/");
  result |= test ("a//", ".");
  result |= test ("a////", ".");
  result |= test ("////usr", "/");
  result |= test ("////usr//", "/");
  result |= test ("//usr", "//");
  result |= test ("//usr//", "//");
  result |= test ("//", "//");

  /* Other Unix implementations behave like this.  */
  result |= test ("x///y", "x");
  result |= test ("x/////y", "x");

  return result != 0;
}
