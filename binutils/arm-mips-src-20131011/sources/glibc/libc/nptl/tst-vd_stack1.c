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

#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>

#define STACK_2M			2097152
#define PTHREAD_STACK_MIN	16384
void *thread_func(void *arg){
	sleep(1);
	return 0;
}

static int do_test()
{
    /* Warning: error checking removed to keep the example small */
    pthread_attr_t attr, cur_attr;
    pthread_t th;
    int ret;
    size_t stack_size;
    void *stack_addr;
    struct rlimit rlim;

    /*	Test 1 */
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN - 1);/* Set small value to see if VD eglibc overrides it */
   pthread_attr_getstacksize(&attr, &stack_size);
   if(stack_size != STACK_2M)
	return 1;
   pthread_attr_destroy(&attr);
   return 0;
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
