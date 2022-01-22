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
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "tst_vdtime.h"

#define VD_TZ		(+9)
#define VD_DST		(+1)

#define VD_TZ_MIN	(MIN_IN_HOUR*VD_TZ)
#define VD_DST_MIN	(MIN_IN_HOUR*VD_DST)

#define VD_EPS_SEC	1

/*
    The test is intended to check vd_localtime function
*/

static int test_vd_localtime (int min_tz, int min_dst)
{
    int status;
    struct timeval sys_tv;
    long long vd_msec, vd_local_sec, vd_local_msec,
	      vd_utc_sec, vd_utc_msec,
	      expec_local_msec, local_msec_diff;

    /* 1) Get current system time */
    errno = 0;
    status = gettimeofday(&sys_tv, (struct timezone *)(NULL));
    if (status)
    {
	printf("gettimeofday() returned %d, errno = %d\n", status, errno);
	return 1;
    }
    #ifdef VD_TST_VERBOSE
    printf ( "system time: %s", ctime( (time_t*)(&sys_tv.tv_sec) ));
    #endif

    /* 2) Calculate vd time equal to system time */
    vd_msec = ((long long)sys_tv.tv_sec + 24L*60L*60L) * MSEC_IN_SEC
	    + ((long long)sys_tv.tv_usec) / USEC_IN_MSEC;
    #ifdef VD_TST_VERBOSE
	printf ( "vd time to be set in msec: %lld\n", vd_msec);
    #endif

    /* 3) Set new vd time */
    errno = 0;
    status = vd_settimeofday(vd_msec, min_tz, min_dst);
    if (status)
    {
	printf("vd_settimeofday() returned %d, errno = %d\n", status, errno);
	return 1;
    }
    #ifdef VD_TST_VERBOSE
    {
	time_t tt = vd_msec/MSEC_IN_SEC;
	printf ( "set vd time: %s", ctime( &tt ));
    }
    #endif

    /* 4) Get current utc vd_time */
    errno = 0;
    vd_utc_sec = vd_gettimeofday();
    if (vd_utc_sec < 0)
    {
	printf("vd_gettimeofday() returned %lld, errno = %d\n", vd_utc_sec, errno);
	return 1;
    }
    vd_utc_msec = vd_utc_sec * MSEC_IN_SEC;
    #ifdef VD_TST_VERBOSE
    {
	time_t tt = (time_t)vd_utc_sec;
	printf ( "  utc vd time: %s", ctime( &tt ));
    }
    #endif

    /* 5) Get current local vd_time */
    errno = 0;
    vd_local_sec = vd_localtime();
    if (vd_local_sec < 0)
    {
	printf("vd_localtime() returned %lld, errno = %d\n", vd_local_sec, errno);
	return 1;
    }
    vd_local_msec = vd_local_sec * MSEC_IN_SEC;
    #ifdef VD_TST_VERBOSE
    {
	time_t tt = (time_t)vd_local_sec;
	printf ( "local vd time: %s", ctime( &tt ));
    }
    #endif

    /* 6) Compute expected local time */
    expec_local_msec = vd_utc_msec + MSEC_IN_MIN * (min_tz + min_dst);
    #ifdef VD_TST_VERBOSE
    {
	time_t expec_tt = expec_local_msec/MSEC_IN_SEC;
	printf ( "expected time: %s", ctime( &expec_tt ));
    }
    #endif

    /* 7) Check that difference between expected
	    and got local time is not too big */
    local_msec_diff = labs(vd_local_msec - expec_local_msec);
    if (local_msec_diff > (VD_EPS_SEC * MSEC_IN_SEC) )
    {
	printf("Error: got local time (%lld sec, %lld msec) ",
	    vd_local_msec/MSEC_IN_SEC, MSEC2SEC_REM(vd_local_msec));
	printf("is differ from expected (%lld sec, %lld msec)\n",
	    expec_local_msec/MSEC_IN_SEC, MSEC2SEC_REM(expec_local_msec));
	return 1;
    }
    #ifdef VD_TST_VERBOSE
    printf ( "local time and expected difference: %lld msec \n", local_msec_diff );
    #endif

    return 0;
}


static int do_test (void)
{
    int status;
    _vdtime_t curr_vd;

    /* Store the /proc/vdtime content */
    status = _vd_time_read(&curr_vd);
    if (status) return status;

    /* Test positive offset */
    #ifdef VD_TST_VERBOSE
    printf ( " --- Test positive tz&dst offset --- \n");
    #endif
    status = test_vd_localtime( VD_TZ_MIN, VD_DST_MIN );
    if (status)
    {
	_vd_time_write(curr_vd);
	return status;
    }

    /* Test neagtive offset */
    #ifdef VD_TST_VERBOSE
    printf ( " --- Test negative tz&dst offset --- \n");
    #endif
    status = test_vd_localtime( -VD_TZ_MIN, VD_DST_MIN );
    if (status)
    {
	_vd_time_write(curr_vd);
	return status;
    }

    /* Set the original /proc/vdtime back */
    status = _vd_time_write(curr_vd);
    if (status) return status;

    return 0;

}
#define TEST_FUNCTION do_test()
#include "../test-skeleton.c"
