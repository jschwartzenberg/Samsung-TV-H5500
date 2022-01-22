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

#define TST_MQ_NAME "/mq_timedsend_test2"
#define TST_MQ_MESSAGE_PRIORITY 0
#define TST_MQ_MAX_MSGLEN 100
#define TST_MQ_MAX_MSGNUM 10
#define TST_MQ_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define TST_MQ_TIMEOUT (2)

#define MQ_TIMEDSEND mq_timedsend_relative

#define _FLUSH { fflush(stdout); fflush(stderr); }
#define _QUOTES(a) #a

static void
alrm_handler (int sig)
{
}

static int do_test (void) 
{
    int i;
    /* queue properties */
    mqd_t mqd;
    struct mq_attr attr;
    /* send message properties */
    char message[TST_MQ_MAX_MSGLEN-1];
    int message_len = TST_MQ_MAX_MSGLEN-1;
    char long_message[TST_MQ_MAX_MSGLEN+1];
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
    int timedsend_errno;
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
        errcode = MQ_TIMEDSEND (0, message, message_len,
                                    TST_MQ_MESSAGE_PRIORITY, &rel_timeout);
	timedsend_errno = errno;
        if ( (errcode==-1) && (timedsend_errno == EBADF))
        {
            printf(" Expected error to send with invalid queue descriptor\n"); 
        }
        else
        {
	    if (timedsend_errno != EBADF)
	    {
                perror(" FAILED unexpected errno with invalid queue descriptor");
	    }
	    if (errcode != -1)
		printf(" FAILED unexpected return=%d with invalid queue descriptor\n", errcode); 
            status = 1;
        }
	_FLUSH;
	errno = 0;

	/* 2) Test message length greater than the mq_msgsize attribute of the message queue */
        errcode = MQ_TIMEDSEND (mqd, long_message, TST_MQ_MAX_MSGLEN+1,
                                    TST_MQ_MESSAGE_PRIORITY, &rel_timeout);
	timedsend_errno = errno;
        if ( (errcode==-1) && (timedsend_errno == EMSGSIZE))
        {
            printf(" Expected error to send with too big meassage length\n"); 
        }
        else
        {
            if (timedsend_errno != EAGAIN)
	    {
                perror(" FAILED unexpected errno with too big meassage length"); 
            }
            if (errcode != -1)
                printf(" FAILED unexpected return=%d with too small meassage length\n", errcode); 
            status = 1;
        }
	_FLUSH;
	errno = 0;

	/* 3) Test timed out return sending to full queue*/
        attr.mq_flags = 0;
        mq_setattr(mqd, &attr, &attr);
	/* fill up queue with messages */
	for (i = 0; i < TST_MQ_MAX_MSGNUM; i++ )
	    errcode = MQ_TIMEDSEND(mqd, message, message_len, TST_MQ_MESSAGE_PRIORITY,&rel_timeout);

        errcode = MQ_TIMEDSEND (mqd, message, message_len,
                                    TST_MQ_MESSAGE_PRIORITY, &rel_timeout);
	timedsend_errno = errno;
        if ( (errcode==-1) && (timedsend_errno == ETIMEDOUT))
        {
            printf(" Expected error to send to full queue\n"); 
        }
        else
        {
            if (timedsend_errno != ETIMEDOUT)
	    {
                perror(" FAILED unexpected errno sending to full queue"); 
            }
            if (errcode != -1)
                printf(" FAILED unexpected return=%d sending to full queue\n", errcode); 
            status = 1;
        }
	_FLUSH;
	errno = 0;
	
        /* 4) Test immediate return sending to full queue in non-blocking mode */
	attr.mq_flags = O_NONBLOCK;
        mq_setattr(mqd, &attr, &attr);
        errcode = MQ_TIMEDSEND (mqd, message, message_len,
                                    TST_MQ_MESSAGE_PRIORITY, &rel_timeout);
	timedsend_errno = errno;
        if ( (errcode==-1) && (timedsend_errno == EAGAIN))
        {
            printf(" Expected error to send to full queue in non-blocking mode\n"); 
        }
        else
        {
            if (timedsend_errno != EAGAIN)
	    {
		perror(" FAILED unexpected errno sending to full queue in non-blocking mode");
	    }
            if (errcode != -1)
                printf(" FAILED unexpected return=%d sending to full queue in non-blocking mode\n", errcode); 
	    status = 1;
	}
	_FLUSH;
        errno = 0;
        
	/* 5) Test NULL timeout - mqsend() must be called */
        attr.mq_flags = 0;
        mq_setattr(mqd, &attr, &attr);
	/* receive one meassage to free space in queue */
        msg_len = mq_receive(mqd, msg_buff, TST_MQ_MAX_MSGLEN, &msg_prio); 
        
	errcode = MQ_TIMEDSEND (mqd, message, message_len,
                                    TST_MQ_MESSAGE_PRIORITY, 0);
        timedsend_errno = errno;
        if ( (msg_len==message_len) && (timedsend_errno == 0))
        {
            printf(" Expected result to send with NULL timeout \n");
        }
        else
        {
            if (timedsend_errno != 0)
            {
                perror(" FAILED unexpected errno sending with NULL timeout");
                _FLUSH;
            }
            if (msg_len != message_len)
                printf(" FAILED unexpected return=%d sending with NULL timeout\n", errcode);
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
	
        errcode = MQ_TIMEDSEND (mqd, message, message_len,
                                    TST_MQ_MESSAGE_PRIORITY, 0);
        timedsend_errno = errno;
        if ( (errcode==-1) && (timedsend_errno == EINTR))
        {
            printf(" Expected error to be interrupted by a signal \n");
        }
        else
        {
            if (timedsend_errno != EINTR)
            {
                perror(" FAILED unexpected errno to be interrupted by a signal");
                _FLUSH;
            }
            if (errcode != -1)
                printf(" FAILED unexpected return=%d to be interrupted by a signal\n", errcode);
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
