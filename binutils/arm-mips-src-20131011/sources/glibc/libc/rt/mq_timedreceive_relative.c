/* Copyright (C) 2013 Free Software Foundation, Inc.
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
#include <mqueue.h>
#include <time.h>
#include <stddef.h>
#include <sysdep.h>
#include <sys/syscall.h>

#include "vd_debug.h"

extern int vd_relative_debug;

#define _GIGA 1000000000
#define _TIMESPEC_SUM(t1,t2,sum)                       	\
{                                                       \
        sum.tv_sec = t2.tv_sec + t1.tv_sec;		\
        sum.tv_nsec = t2.tv_nsec + t1.tv_nsec;       	\
        sum.tv_sec += sum.tv_nsec / _GIGA;              \
        sum.tv_nsec = sum.tv_nsec % _GIGA;              \
}
#define _TIMESPEC_DELTA(t1,t2,delta)                            \
{                                                               \
        delta.tv_sec = (t1.tv_sec - 1) - t2.tv_sec;             \
        delta.tv_nsec = (t1.tv_nsec + _GIGA) - t2.tv_nsec;      \
        delta.tv_sec += delta.tv_nsec / _GIGA;                  \
        delta.tv_nsec = delta.tv_nsec % _GIGA;                  \
}
#define _TIMESPEC_CHECK_NEGATIVE(t_delta)                       \
     ((t_delta.tv_sec < 0) || ((t_delta.tv_sec==0) && (t_delta.tv_nsec < 0)))

#define __mq_getattr(a,b)       INLINE_SYSCALL(mq_getsetattr,3,a,NULL,b);
#define __mq_setattr(a,b,c)	INLINE_SYSCALL(mq_getsetattr,3,a,b,c)
#define __mq_receive(a,b,c,d)	INLINE_SYSCALL(mq_timedreceive,5,a,b,c,d,NULL)

/* Receive the oldest from highest priority messages in message queue
   MQDES, stop waiting if ABS_TIMEOUT expires.  */
ssize_t
mq_timedreceive_relative (mqd_t mqdes, char *__restrict msg_ptr, size_t msg_len,
		 unsigned int *__restrict msg_prio,
		 const struct timespec *__restrict rel_timeout)
{
    struct timespec start_time;
    struct timespec end_time;
    struct timespec expected_end_time;
    struct timespec end_time_delta;
    struct mq_attr old_attr;
    struct mq_attr new_attr;
    ssize_t nbytes;
    int error;

    /* VD debug message */
    vd_check_debug (&vd_relative_debug, "VD_RELATIVE_DEBUG");
    vd_out_debug (&vd_relative_debug, REL_DBG_MQ_TIMEDRECEIVE_RELATIVE, "mq_timedreceive_relative");

    /* Get start time of receieving in monotonic clocks (independent on abs time) */
    __clock_gettime(CLOCK_MONOTONIC, &start_time);

    /* Get queue attributes, return error if it fails*/
    error = __mq_getattr(mqdes, &old_attr);
    if (error)
    {
        return error;
    }

    /* If the queue is in nonblocking mode just receive and return */
    if ((old_attr.mq_flags & O_NONBLOCK) || (rel_timeout==NULL))
    {
        return __mq_receive(mqdes, msg_ptr, msg_len, msg_prio);
    }

    /* Change attributes of message queue into nonblocking mode */
    new_attr = old_attr;
    new_attr.mq_flags = O_NONBLOCK;
    __mq_setattr(mqdes, &new_attr, &old_attr);

    /* Calc expected timeout in monotonic clocks */
    _TIMESPEC_SUM(start_time, (*rel_timeout), expected_end_time);

    /* Try to receive message until success or timeout expired */
    while(1)
    {
        errno = 0;
	nbytes = __mq_receive (mqdes, msg_ptr, msg_len, msg_prio);
        __clock_gettime(CLOCK_MONOTONIC, &end_time);

        /* If queue is empty check errno */
        if (errno == EAGAIN)
        {
	    _TIMESPEC_DELTA(expected_end_time, end_time, end_time_delta);
            /* If timeout passed stop receiving */
            if (_TIMESPEC_CHECK_NEGATIVE(end_time_delta))
            {
                errno = ETIMEDOUT;
                break;
            }
            else
            {
	       continue;
            }
        }
        /* Otherwise stop receiving */
        else
        {
	    break;
        }
    }

    /* Revert attributes of message queue to old values */
    __mq_setattr(mqdes, &old_attr, &old_attr);

    /* Return number of received bytes or -1 on error */
    return nbytes;

}
strong_alias (mq_timedreceive_relative, __mq_timedreceive_relative)

