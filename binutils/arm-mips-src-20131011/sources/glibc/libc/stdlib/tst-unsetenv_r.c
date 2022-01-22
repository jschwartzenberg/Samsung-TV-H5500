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

/* Test case for unsetenv_r function */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define NUM_ITER  5
#define BUF_LEN   32
#define VAR_NAME  "FOO"

int stop_var = 0;

void *t_func_set(void *s);
void *t_func_unset(void *s);
int get_and_compare(const char* name, const char* val);

int main(void)
{
  pthread_t t1,t2;
  unsigned int c1 = 0, c2 = 0; /* Occurences of foo and "", respectively */
  const char *foo = {"foo"};
  int env_len = 0, i = 0, ret = 0;
  ret = pthread_create (&t1, NULL, &t_func_set, (void *) foo);
  if(ret)
    {
      perror ("pthread_create");
      goto exit;
    }

  ret = pthread_create (&t2, NULL, &t_func_unset, NULL);
  if(ret){
    perror ("pthread_create");
    goto exit;
  }
  sleep (1);

  for (i = 0 ; i < NUM_ITER; i++)
    {
      ret = get_and_compare (VAR_NAME, foo);
      if (ret == 1)
        c1++;
      else if (ret == 2)
        c2++;
      else if (ret == -1)
        {
          printf ("get_and_compare failed\n");
          goto exit;
        }
      ret = 0;
    }

  pthread_join (t1, NULL);
  pthread_join (t2, NULL);
  ret = get_and_compare (VAR_NAME, foo);
  if (ret == -1)
    {
      printf("get_and_compare failed\n");
      goto exit;
    }
  else if (ret == 1)
    c1++;
  else if (ret == 2)
    c2++;
  ret = 0;
exit:
  return ret;
}

void *t_func_set (void *s)
{
  int cnt = 10000;
  int ret;
  while (cnt--)
    {
      ret = setenv_r (VAR_NAME, (char*) s, 1);
      if (ret)
        perror ("setenv_r");
    }
  return NULL;
}

void *t_func_unset (void *s)
{
  int cnt = 10000;
  int ret;
  while (cnt--)
    {
      ret = unsetenv_r (VAR_NAME);
      if(ret)
        perror ("unsetenv_r");
    }
  return NULL;
}

int get_and_compare (const char* name, const char* val)
{
  int ret = 0;
  char buf[BUF_LEN] = "\0";
  ret = getenv_r (name, buf, BUF_LEN - 1);
  if(ret && errno != ENOENT)
    {
      perror ("getenv_r");
      ret = -1;
    }
  else
  if (!strcmp(buf, val))
    ret = 1;
  else
  if (!strcmp(buf, ""))/* ENOENT case */
    ret = 2;
  else ret = -1;
  return ret;
}
