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

#define TST_MQ_NAME "/mq_timedreceive_test"
#define TST_MQ_MESSAGE_PRIORITY 0
#define TST_MQ_MAX_MSGLEN 100
#define TST_MQ_MAX_MSGNUM 10
#define TST_MQ_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define TST_MQ_TIMEOUT (5) 
#define TST_MQ_SENDER_SLEEPTIME (3) 

#define _FLUSH fflush(0) 
#define MQ_TIMEDRECEIVE mq_timedreceive_relative

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
    struct timespec t_start_receive;
    struct timespec t_end_receive;
    /* return values */
    int errcode;
    int status = 0;
    int child_status = 0;

    /* Form the queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = TST_MQ_MAX_MSGNUM;
    attr.mq_msgsize = TST_MQ_MAX_MSGLEN;
    attr.mq_curmsgs = 0;    

    /* Open message queue */
    printf(" Receiver: opening queue...\n"); _FLUSH;
    mq_unlink(TST_MQ_NAME);
    mqd = mq_open(TST_MQ_NAME, O_RDONLY | O_CREAT, TST_MQ_MODE, &attr);

    if( mqd == (mqd_t)-1 )
    {
        /* Failed to open a message queue */
        perror(" Receiver: FAILED to open "); _FLUSH;
        status = 1;
	return status;
    }
    
    /* Create child sender process */
    childPID = fork();

    if(childPID >= 0) // fork was successful
    {
        if(childPID == 0) // Child process - sender
        {
	    /* return values */
            int errcode;
            int status = 0;
	    
            printf(" Sender: opening queue...\n"); _FLUSH;
            mqd = mq_open(TST_MQ_NAME, O_WRONLY | O_CREAT, TST_MQ_MODE, &attr);
              
            if( mqd != (mqd_t)-1 )
            {
               
                printf(" Sender: sleep before send for %d secs ... \n",
				TST_MQ_SENDER_SLEEPTIME);  _FLUSH;
                sleep(TST_MQ_SENDER_SLEEPTIME);

                printf(" Sender: sending message... \n");  _FLUSH;
                errcode = mq_send(mqd, message, message_len, TST_MQ_MESSAGE_PRIORITY);
                if(errcode)
                {
                    perror(" Sender: FAILED to send ");
                    status = 1;
                }

                printf(" Sender: closing queue... \n"); _FLUSH;
                errcode = mq_close(mqd);
                if(errcode)
                {
                    perror(" Sender: FAILED to close ");
                    status = 1;
                }
            
            }
            else
            {
	        /* Failed to open a message queue */
                perror(" Sender: FAILED to open ");
                status = 1;
            }
            exit(status); 
                
        }
        else //Parent process - receiver
	{
	     /* Revert queue into blocking mode */
            attr.mq_flags = 0;
            mq_setattr(mqd, &attr, &attr);

	    printf(" Receiver: receiving message for %d secs... \n", 
			TST_MQ_TIMEOUT); _FLUSH;

	    rel_timeout.tv_sec = TST_MQ_TIMEOUT; 
	    rel_timeout.tv_nsec = 0;
	  
	    /* Measure time spent for receiving */
	    clock_gettime( CLOCK_REALTIME, &t_start_receive);

	    msg_len = MQ_TIMEDRECEIVE (mqd, msg_buff, TST_MQ_MAX_MSGLEN, 
					&msg_prio, &rel_timeout);

	    clock_gettime( CLOCK_REALTIME, &t_end_receive);

	    if(msg_len < 0)
	    {
		/* Receive was unsuccessful */
		if(errno == ETIMEDOUT)
		{
		    _TIMESPEC_DELTA(t_end_receive, t_start_receive, real_timeout)                            
		    printf(" Receiver: FAILED: recieving timed out in %d secs %d nanosecs\n", 
				(int)real_timeout.tv_sec, (int)real_timeout.tv_nsec);
		    status = 1;

		}
		else
		{
		    perror(" Receiver: FAILED to receive ");
		    status = 1;
		}
	    }
	    else
	    {
		/* Receive was successful */
		/* Check real time spent for receiving */
		_TIMESPEC_DELTA(t_end_receive, t_start_receive, real_timeout)
		_TIMESPEC_DELTA(rel_timeout, real_timeout, timeout_delta)
		if ((_TIMESPEC_CHECK_NEGATIVE(timeout_delta)) || 
				(real_timeout.tv_sec<0) || (real_timeout.tv_nsec<0))
		{
		   printf(" Receiver: FAILED: received with bad real timeout = %d secs %d nanosecs\n",
				 (int)real_timeout.tv_sec, (int)real_timeout.tv_nsec);
		   printf(" Start = %d, %d\n", (int)t_start_receive.tv_sec, (int)t_start_receive.tv_nsec);
		   printf(" End = %d, %d\n", (int)t_end_receive.tv_sec, (int)t_end_receive.tv_nsec);
		   status = 1;
		}
		else
		   printf(" Receiver: received in %d secs %d nanosecs\n",
					 (int)real_timeout.tv_sec, (int)real_timeout.tv_nsec);
	    }
	    
	    /* Wait for child(sender) process to exit */		    
	    wait(&child_status);
	    status |= child_status;

	    /* Close the message queue */
	    printf(" Receiver: closing queue... \n"); _FLUSH;
	    errcode = mq_close(mqd);
	    if(errcode)
	    {  
		perror(" Receiver: FAILED to close ");
		status = 1;                   
	    }           
	    printf(" Receiver: removing queue... \n"); _FLUSH;
	    errcode = mq_unlink(TST_MQ_NAME);
	    if(errcode)
	    {
		perror(" Receiver: FAILED to remove ");
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
