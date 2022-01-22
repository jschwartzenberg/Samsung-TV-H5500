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
#include <unistd.h>

#include "tst_vdtime.h"


#define VD_OFFSET_SEC	3

#define VD_SLEEP_SEC	5
#define VD_EPS_SEC	1
#define VD_DELTA_SEC	(VD_SLEEP_SEC + VD_EPS_SEC)


/*
    The test is intended to check vd_get/settimeofday functions
    to read and write time value properly.
    It also measure timebreaks via system and via vd time
    and check that they are correct and consistent
*/

static int test_vd_time (int sec_offset)
{
    int status;
    struct timeval sys_tv0, sys_tv1, sys_diff;
    long long vd_msec0, vd_sec1, vd_msec1,
	      vd_msec_diff, sys_vd_msec_diff;

    /* 1) Get current system time */
    errno = 0;
    status = gettimeofday(&sys_tv0, (struct timezone *)(NULL));
    if (status)
    {
	printf("gettimeofday() returned %d, errno = %d\n", status, errno);
	return 1;
    }
    #ifdef VD_TST_VERBOSE
    printf ( "system time: %s", ctime( (time_t*)(&sys_tv0.tv_sec) ));
    #endif

    /* 2) Calculate vd time offseted */
    vd_msec0 = (((long long)sys_tv0.tv_sec + sec_offset)) * MSEC_IN_SEC;
	     +  ((long long)sys_tv0.tv_usec) / USEC_IN_MSEC;
    #ifdef VD_TST_VERBOSE
	printf ( "vd time to be set in msec: %lld\n", vd_msec0);
    #endif

    /* 3) Set new vd time */
    errno = 0;
    status = vd_settimeofday(vd_msec0, 0, 0);
    if (status)
    {
	printf("vd_settimeofday() returned %d, errno = %d\n", status, errno);
	return 1;
    }
    #ifdef VD_TST_VERBOSE
    {
	time_t tt0 = vd_msec0/MSEC_IN_SEC;
	printf ( "set vd time: %s", ctime( &tt0 ));
    }
    #endif

    #ifdef VD_TST_VERBOSE
    printf ( "Sleep for %d sec\n", VD_SLEEP_SEC);
    #endif
    sleep(VD_SLEEP_SEC);

    /* 4) Get new values of vd_time and system time */
    errno = 0;
    vd_sec1 = vd_gettimeofday();
    vd_msec1 = vd_sec1 * MSEC_IN_SEC;
    if (vd_msec1 < 0)
    {
	printf("vd_gettimeofday() returned %lld, errno = %d\n", vd_msec1, errno);
	return 1;
    }
    #ifdef VD_TST_VERBOSE
    {
	time_t tt1 = (time_t)vd_sec1;
	printf ( "get vd time: %s", ctime( &tt1 ));
    }
    #endif

    errno = 0;
    status = gettimeofday(&sys_tv1, (struct timezone *)(NULL));
    if (status)
    {
	printf("gettimeofday() returned %d, errno = %d\n", status, errno);
	return 1;
    }
    #ifdef VD_TST_VERBOSE
    printf ( "system time: %s", ctime( (time_t*)(&sys_tv1.tv_sec) ));
    #endif

    /* 5) Check measured time breaks for correctness */
    timersub(&sys_tv1, &sys_tv0, &sys_diff);
    if (sys_diff.tv_sec > VD_DELTA_SEC)
    {
	printf("Error: system time changed on %ld sec, %ld usec\n",
	    sys_diff.tv_sec, sys_diff.tv_usec);
	return 1;
    }
    #ifdef VD_TST_VERBOSE
    printf ( "system time passed: %ld sec, %ld usec \n",
	sys_diff.tv_sec, sys_diff.tv_usec );
    #endif

    vd_msec_diff = vd_msec1 - vd_msec0;
    if (vd_msec_diff/MSEC_IN_SEC > VD_DELTA_SEC)
    {
	printf("Error: vd time changed on %lld sec, %lld msec\n",
		vd_msec_diff/MSEC_IN_SEC, MSEC2SEC_REM(vd_msec_diff));
	return 1;
    }
    #ifdef VD_TST_VERBOSE
    printf ( "    vd time passed: %lld sec, %lld msec \n",
		vd_msec_diff/MSEC_IN_SEC, MSEC2SEC_REM(vd_msec_diff));
    #endif

    /* 6) Check that measured time breaks are consistent */
    sys_vd_msec_diff = vd_msec_diff -
	( sys_diff.tv_sec * MSEC_IN_SEC + sys_diff.tv_usec / USEC_IN_MSEC);
    if (sys_vd_msec_diff > (VD_EPS_SEC * MSEC_IN_SEC) )
    {
	printf("Error: inconsistent vd timebreak (%lld sec, %lld msec)",
	    vd_msec_diff/MSEC_IN_SEC, MSEC2SEC_REM(vd_msec_diff));
	printf("and system timebreak (%ld sec, %ld msec)\n",
	    sys_diff.tv_sec, sys_diff.tv_usec/USEC_IN_MSEC);
	return 1;
    }
    #ifdef VD_TST_VERBOSE
    printf ( "time breaks differ: %lld msec \n", sys_vd_msec_diff );
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
    printf ( " --- Test positive offset --- \n");
    #endif
    status = test_vd_time( VD_OFFSET_SEC);
    if (status)
    {
	_vd_time_write(curr_vd);
	return status;
    }

    /* Test neagtive offset */
    #ifdef VD_TST_VERBOSE
    printf ( " --- Test negative offset --- \n");
    #endif
    status = test_vd_time(-VD_OFFSET_SEC);
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
#define TIMEOUT (VD_DELTA_SEC*2)
#define TEST_FUNCTION do_test()
#include "../test-skeleton.c"
