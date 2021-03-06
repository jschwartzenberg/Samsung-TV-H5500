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
#include <stdlib.h>
#include <atomic.h>
#include "pthreadP.h"

#include "vd_debug.h"

extern int vd_relative_debug;

static void
cleanup (void *arg)
{
  *(void **) arg = NULL;
}


int
pthread_timedjoin_np_relative (threadid, thread_return, reltime)
     pthread_t threadid;
     void **thread_return;
     const struct timespec *reltime;
{
  struct pthread *self;
  struct pthread *pd = (struct pthread *) threadid;
  int result;

  /* VD debug message */
  vd_check_debug (&vd_relative_debug, "VD_RELATIVE_DEBUG");
  vd_out_debug (&vd_relative_debug, REL_DBG_PTHREAD_TIMEDJOIN_NP_RELATIVE, "pthread_timedjoin_np_relative");

  /* Make sure the descriptor is valid.  */
  if (INVALID_NOT_TERMINATED_TD_P (pd))
    /* Not a valid thread handle.  */
    return ESRCH;

  /* Is the thread joinable?.  */
  if (IS_DETACHED (pd))
    /* We cannot wait for the thread.  */
    return EINVAL;

  self = THREAD_SELF;
  if (pd == self || self->joinid == pd)
    /* This is a deadlock situation.  The threads are waiting for each
       other to finish.  Note that this is a "may" error.  To be 100%
       sure we catch this error we would have to lock the data
       structures but it is not necessary.  In the unlikely case that
       two threads are really caught in this situation they will
       deadlock.  It is the programmer's problem to figure this
       out.  */
    return EDEADLK;

  /* Wait for the thread to finish.  If it is already locked something
     is wrong.  There can only be one waiter.  */
  if (__builtin_expect (atomic_compare_and_exchange_bool_acq (&pd->joinid,
							      self, NULL), 0))
    /* There is already somebody waiting for the thread.  */
    return EINVAL;


  /* During the wait we change to asynchronous cancellation.  If we
     are cancelled the thread we are waiting for must be marked as
     un-wait-ed for again.  */
  pthread_cleanup_push (cleanup, &pd->joinid);

  /* Switch to asynchronous cancellation.  */
  int oldtype = CANCEL_ASYNC ();


  /* Wait for the child.  */
  result = lll_timedwait_tid_relative (pd->tid, reltime);


  /* Restore cancellation mode.  */
  CANCEL_RESET (oldtype);

  /* Remove the handler.  */
  pthread_cleanup_pop (0);


  /* We might have timed out.  */
  if (result == 0)
    {
      /* Store the return value if the caller is interested.  */
      if (thread_return != NULL)
	*thread_return = pd->result;


      /* Free the TCB.  */
      __free_tcb (pd);
    }
  else
    pd->joinid = NULL;

  return result;
}
