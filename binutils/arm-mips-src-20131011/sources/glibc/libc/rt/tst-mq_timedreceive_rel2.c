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
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define TST_MQ_NAME "/mq_timedreceive_test2"
#define TST_MQ_MESSAGE_PRIORITY 0
#define TST_MQ_MAX_MSGLEN 100
#define TST_MQ_MAX_MSGNUM 10
#define TST_MQ_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define TST_MQ_TIMEOUT (2)

#define MQ_TIMEDRECEIVE mq_timedreceive_relative

#define _FLUSH { fflush(stdout); fflush(stderr); }
#define _QUOTES(a) #a

static void
alrm_handler (int sig)
{
}

static int do_test (void) 
{
        
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
    /* alarm signal and timer  */
    struct sigaction sa = { .sa_handler = alrm_handler, .sa_flags = 0 };
    struct itimerval it = { .it_value = { .tv_sec = 1 } };
    /* return values */
    int timedreceive_errno;
    int errcode;
    int status = 0;

    /* Form the queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = TST_MQ_MAX_MSGNUM;
    attr.mq_msgsize = TST_MQ_MAX_MSGLEN;
    attr.mq_curmsgs = 0;    

    /* Set relative timeout */
    rel_timeout.tv_sec = TST_MQ_TIMEOUT;
    rel_timeout.tv_nsec = 0;

    /* Open message queue */
    mq_unlink(TST_MQ_NAME);
    mqd = mq_open(TST_MQ_NAME, O_RDWR | O_CREAT | O_EXCL, TST_MQ_MODE, &attr);
    /* Clear errno */
    errno = 0;

    if( mqd != (mqd_t)-1 )
    {
	/* 1) Test invalid queue descriptor */
        msg_len = MQ_TIMEDRECEIVE (0, msg_buff, TST_MQ_MAX_MSGLEN,
                                    &msg_prio, &rel_timeout);
	timedreceive_errno = errno;
        if ( (msg_len==-1) && (timedreceive_errno == EBADF))
        {
            printf(" Expected error to receive with invalid queue descriptor\n"); 
        }
        else
        {
	    if (timedreceive_errno != EBADF)
	    {
                perror(" FAILED unexpected errno with invalid queue descriptor");
	    }
	    if (msg_len != -1)
		printf(" FAILED unexpected return=%d with invalid queue descriptor\n", msg_len); 
            status = 1;
        }
	_FLUSH;
	errno = 0;

	/* 2) Test message length less than max. message size attribute*/
        msg_len = MQ_TIMEDRECEIVE (mqd, msg_buff, TST_MQ_MAX_MSGLEN-1,
                                    &msg_prio, &rel_timeout);
	timedreceive_errno = errno;
        if ( (msg_len==-1) && (timedreceive_errno == EMSGSIZE))
        {
            printf(" Expected error to receive with too small meassage length\n"); 
        }
        else
        {
            if (timedreceive_errno != EAGAIN)
	    {
                perror(" FAILED unexpected errno with too small meassage length"); 
            }
            if (msg_len != -1)
                printf(" FAILED unexpected return=%d with too small meassage length\n", msg_len); 
            status = 1;
        }
	_FLUSH;
	errno = 0;

	/* 3) Test timed out return receiving from empty queue*/
        attr.mq_flags = 0;
        mq_setattr(mqd, &attr, &attr);
        msg_len = MQ_TIMEDRECEIVE (mqd, msg_buff, TST_MQ_MAX_MSGLEN,
                                    &msg_prio, &rel_timeout);
	timedreceive_errno = errno;
        if ( (msg_len==-1) && (timedreceive_errno == ETIMEDOUT))
        {
            printf(" Expected error to receive from empty queue\n"); 
        }
        else
        {
            if (timedreceive_errno != ETIMEDOUT)
	    {
                perror(" FAILED unexpected errno receiving from empty queue"); 
            }
            if (msg_len != -1)
                printf(" FAILED unexpected return=%d receiving from empty queue\n", msg_len); 
            status = 1;
        }
	_FLUSH;
	errno = 0;
	
        /* 4) Test immediate return recieving from empty queue in non-blocking mode */
	attr.mq_flags = O_NONBLOCK;
        mq_setattr(mqd, &attr, &attr);
        msg_len = MQ_TIMEDRECEIVE (mqd, msg_buff, TST_MQ_MAX_MSGLEN,
                                    &msg_prio, &rel_timeout);
	timedreceive_errno = errno;
        if ( (msg_len==-1) && (timedreceive_errno == EAGAIN))
        {
            printf(" Expected error to receive from empty queue in non-blocking mode\n"); 
        }
        else
        {
            if (timedreceive_errno != EAGAIN)
	    {
		perror(" FAILED unexpected errno receiving from empty queue in non-blocking mode");
	    }
            if (msg_len != -1)
                printf(" FAILED unexpected return=%d receiving from empty queue in non-blocking mode\n", msg_len); 
	    status = 1;
	}
	_FLUSH;
        errno = 0;
        
	/* 5) Test NULL timeout - mqreceive() must be called */
        attr.mq_flags = 0;
        mq_setattr(mqd, &attr, &attr);
	/* send meassage */
        mq_send(mqd, message, message_len, TST_MQ_MESSAGE_PRIORITY);
        
	msg_len = MQ_TIMEDRECEIVE (mqd, msg_buff, TST_MQ_MAX_MSGLEN,
                                    &msg_prio, 0);
        timedreceive_errno = errno;
        if ( (msg_len==message_len) && (timedreceive_errno == 0))
        {
            printf(" Expected result to receive with NULL timeout \n");
        }
        else
        {
            if (timedreceive_errno != 0)
            {
                perror(" FAILED unexpected errno receiving with NULL timeout");
                _FLUSH;
            }
            if (msg_len != message_len)
                printf(" FAILED unexpected return=%d receiving with NULL timeout\n", msg_len);
	    status = 1;
        }
	 _FLUSH;
        errno = 0;

	/* 6) Test interrrupt by signal  */
	attr.mq_flags = 0;
        mq_setattr(mqd, &attr, &attr);
	
	sigemptyset (&sa.sa_mask);
	sigaction (SIGALRM, &sa, NULL);
	setitimer (ITIMER_REAL, &it, NULL);
	
        msg_len = MQ_TIMEDRECEIVE (mqd, msg_buff, TST_MQ_MAX_MSGLEN,
                                    &msg_prio, 0);
        timedreceive_errno = errno;
        if ( (msg_len==-1) && (timedreceive_errno == EINTR))
        {
            printf(" Expected error to be interrupted by a signal \n");
        }
        else
        {
            if (timedreceive_errno != EINTR)
            {
                perror(" FAILED unexpected errno to be interrupted by a signal");
                _FLUSH;
            }
            if (msg_len != -1)
                printf(" FAILED unexpected return=%d to be interrupted by a signal\n", msg_len);
	    status = 1;
        }

	/* Close the message queue */
	errcode = mq_close(mqd);
	if(errcode)
	{
	    printf(" FAILED to close queue");
	    status = 1;
	}
	errcode = mq_unlink(TST_MQ_NAME);
	if(errcode)
	{
	    printf(" FAILED to remove queue");
	    status = 1;
	}
    }
    else
    {
        /* Failed to open a message queue */
        printf(" FAILED to open queue "); 
        status = 1;
    }
    
    return status;
}

#define TIMEOUT 5
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
