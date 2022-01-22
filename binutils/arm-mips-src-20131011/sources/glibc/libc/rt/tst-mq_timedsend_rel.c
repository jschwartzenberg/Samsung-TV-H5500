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
#include <fcntl.h>
#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define TST_MQ_NAME "/mq_timesend_test"
#define TST_MQ_MESSAGE_PRIORITY 0
#define TST_MQ_MAX_MSGLEN 100
#define TST_MQ_MAX_MSGNUM 2 
#define TST_MQ_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define TST_MQ_TIMEOUT (5) 
#define TST_MQ_RECEIVER_SLEEPTIME (3) 

#define _FLUSH fflush(0) 
#define MQ_TIMEDSEND mq_timedsend_relative

#define _GIGA 1000000000
#define _TIMESPEC_DELTA(t1,t2,delta)				\
{								\
	delta.tv_sec = (t1.tv_sec - 1) - t2.tv_sec;		\
	delta.tv_nsec = (t1.tv_nsec + _GIGA) - t2.tv_nsec;	\
	delta.tv_sec += delta.tv_nsec / _GIGA;			\
	delta.tv_nsec = delta.tv_nsec % _GIGA;			\
}
#define _TIMESPEC_CHECK_NEGATIVE(t_delta)                                    \
     ((t_delta.tv_sec < 0) || ((t_delta.tv_sec==0) && (t_delta.tv_nsec < 0)))
static int do_test (void)
{
        
    pid_t childPID;
    int i;
    /* queue properties */
    mqd_t mqd;
    struct mq_attr attr;
    /* send message properties */
    char message[TST_MQ_MAX_MSGLEN-1];
    int message_len = TST_MQ_MAX_MSGLEN-1;
    /* receive message properties */
    char msg_buff[TST_MQ_MAX_MSGLEN];
    ssize_t msg_len;
    unsigned msg_prio;
    /* time measurement related veriables */
    struct timespec rel_timeout;
    struct timespec real_timeout;
    struct timespec timeout_delta;
    struct timespec t_start_send;
    struct timespec t_end_send;
    /* return values */
    int status = 0;
    int child_status = 0;
    int errcode;

    /* Form the queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = TST_MQ_MAX_MSGNUM;
    attr.mq_msgsize = TST_MQ_MAX_MSGLEN;
    attr.mq_curmsgs = 0; 
  
    printf(" Sender: opening queue...\n"); _FLUSH;
    mq_unlink(TST_MQ_NAME);
    mqd = mq_open(TST_MQ_NAME, O_WRONLY | O_CREAT, TST_MQ_MODE, &attr);

    if( mqd != (mqd_t)-1 )
    {
	for (i = 0; i<TST_MQ_MAX_MSGNUM; i++)
        {
            printf(" Sender: sending message #%d... \n", i);  _FLUSH;
            errcode = MQ_TIMEDSEND(mqd, message, message_len, TST_MQ_MESSAGE_PRIORITY,0);
            if(errcode)
            {
            	perror(" Sender: FAILED to send ");
                status = 1;
            }
	}
    }
    else
    {
	/* Failed to open a message queue */
	perror(" Sender: FAILED to open "); _FLUSH;
	status = 1;
	return status;
    }
    
    /* Create new process */
    childPID = fork();

    if(childPID >= 0) // fork was successful
    {
        if(childPID == 0) // Child process - receiver
        {
	    /* return values */
            int status = 0;
	
	    printf(" Receiver: opening queue...\n"); _FLUSH;
            mqd = mq_open(TST_MQ_NAME, O_RDONLY | O_CREAT, TST_MQ_MODE, &attr);
	    if( mqd != (mqd_t)-1 )
            {
		printf(" Receiver: sleep before receive for %d secs ... \n",
			    TST_MQ_RECEIVER_SLEEPTIME);  _FLUSH;
                sleep(TST_MQ_RECEIVER_SLEEPTIME);
		
		printf(" Receiver: receiving message... \n"); _FLUSH;
                msg_len = mq_receive(mqd, msg_buff, TST_MQ_MAX_MSGLEN, &msg_prio);
                if(msg_len < 0)
                {
                     perror(" Receiver: FAILED to receive "); _FLUSH;
                     status = 1;
                }
                else
		{
                     printf(" Receiver: received %d bytes\n", msg_len);  _FLUSH;
		}
	    }
	    else
            {
                /* Failed to open a message queue */
                perror(" Receiver: FAILED to open "); _FLUSH;
                status = 1;
            }
            exit(status);
	}
        else //Parent process - sender
	{
	    /* Revert queue into blocking mode */
	    attr.mq_flags = 0;
	    mq_setattr(mqd, &attr, &attr);
	
            printf(" Sender: sending message #%d for %d secs... \n", 
		TST_MQ_MAX_MSGNUM, TST_MQ_TIMEOUT); _FLUSH;

            rel_timeout.tv_sec = TST_MQ_TIMEOUT;
            rel_timeout.tv_nsec = 0;
	    
	    /* Measure time spent for sending */ 
            clock_gettime( CLOCK_REALTIME, &t_start_send);

            errcode = MQ_TIMEDSEND (mqd, message, message_len, 
					TST_MQ_MESSAGE_PRIORITY, &rel_timeout);
	    clock_gettime( CLOCK_REALTIME, &t_end_send);

	    if(errcode)
            {
               /* Send was unsuccessful */    
		if(errno == ETIMEDOUT)
		{
		    _TIMESPEC_DELTA(t_end_send, t_start_send, real_timeout)                 
		    printf(" Sender: FAILED: sending timed out in %d secs %d nanosecs\n",
				(int)real_timeout.tv_sec, (int)real_timeout.tv_nsec); _FLUSH;
		    status = 1;

		}
		else
		{
	            perror(" Sender: FAILED to send "); _FLUSH;
		    status = 1;
		}
            }
	    else
	    {
		/* Send was successful */
		/* Check real time spent for sending */
		_TIMESPEC_DELTA(t_end_send, t_start_send, real_timeout)
		_TIMESPEC_DELTA(rel_timeout, real_timeout, timeout_delta)
		if ((_TIMESPEC_CHECK_NEGATIVE(timeout_delta)) ||
			(real_timeout.tv_sec<0) || (real_timeout.tv_nsec<0))
		{
		   printf(" Sender: FAILED: send with bad real timeout = %d secs %d nanosecs\n",
				(int)real_timeout.tv_sec, (int)real_timeout.tv_nsec); _FLUSH;
		   printf(" Start = %d, %d\n", (int)t_start_send.tv_sec, (int)t_start_send.tv_nsec); _FLUSH;
		   printf(" End = %d, %d\n", (int)t_end_send.tv_sec, (int)t_end_send.tv_nsec); _FLUSH;
		   status = 1;
		}
		else
		   printf(" Sender: send message #%d in %d secs %d nanosecs\n", TST_MQ_MAX_MSGNUM, 
				(int)real_timeout.tv_sec, (int)real_timeout.tv_nsec); _FLUSH;
	   }

	    /* Wait for child(receiver) process to exit */
	    wait(&child_status);
	    status |= child_status;

	    /* Close the message queue */
	    printf(" Sender: closing queue... \n"); _FLUSH;
	    errcode = mq_close(mqd);
	    if(errcode)
	    {
		perror(" Sender: FAILED to close "); _FLUSH;
		status = 1;
	    }

	    printf(" Sender: removing queue... \n"); _FLUSH;
	    errcode = mq_unlink(TST_MQ_NAME);
	    if(errcode)
	    {
		perror(" Sender: FAILED to remove "); _FLUSH;
		status = 1;
	    }

	    printf("\n");    
	    return status;
	}
    }
    else // Failed to fork
    {
        perror(" FAILED to fork ");
        return 1;
    }
    return 0;
}

#define TIMEOUT 7
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
