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


#define VD_BAD_ERRNO	EINVAL
#define VD_BAD_RES	(-1)
#define VD_GOOD_ERRNO	(0)
#define VD_GOOD_RES	(0)

#define VD_TIME_BOUND	 (LLONG_MAX)
#define VD_TZ_LBOUND	 (-12 * MIN_IN_HOUR)
#define VD_TZ_RBOUND	 (+14 * MIN_IN_HOUR)
#define VD_DST_LBOUND	 (0)
#define VD_DST_RBOUND	 (+6 * MIN_IN_HOUR)


#define VD_TIME_NORM	 ( VD_TIME_BOUND/2 )
#define VD_TZ_NORM	 ( (VD_TZ_LBOUND+VD_TZ_RBOUND)/2 )
#define VD_DST_NORM	 ( (VD_DST_LBOUND+VD_DST_RBOUND)/2 )

typedef struct _utctime_val {
    long long val;
    int is_bad;
    char name[128];
} utctime_val;

typedef struct _utcoff_val {
    int val;
    int is_bad;
    char name[128];
} utcoff_val;

typedef struct _dstoff_val {
    int val;
    int is_bad;
    char name[32];
} dstoff_val;

utctime_val test_utctime[] = {
 {  VD_TIME_NORM,   0, "normal"},
 {            0L,   0, "zero"},
 { -VD_TIME_NORM,   1, "negative"},
 { VD_TIME_BOUND,   0, "max"},
 { VD_TIME_BOUND+1, 1, "over max"}
};


utcoff_val test_utcoff[] = {
 { VD_TZ_NORM,     0, "normal"},
 { VD_TZ_LBOUND,   0, "left bound"},
 { VD_TZ_LBOUND-1, 1, "out l.bound"},
 { VD_TZ_RBOUND,   0, "right bound"},
 { VD_TZ_RBOUND+1, 1, "out r.bound"},
};


dstoff_val test_dstoff[] = {
 { VD_DST_NORM,     0, "normal"},
 { VD_DST_LBOUND,   0, "left bound"},
 { VD_DST_LBOUND-1, 1, "out l.bound"},
 { VD_DST_RBOUND,   0, "right bound"},
 { VD_DST_RBOUND+1, 1, "out r.bound"},
};

/*
    The test is intended to check vd_settimeofday()
    functions on bad values
*/

void print_fail(void)
{
	printf("   X   |");
}
void print_pass(void)
{
	printf("   o   |");
}

static int test_vd_settimeofay_badvals (utctime_val utctime, utcoff_val utcoff, dstoff_val dstoff)
{
    int cur_res, expec_res, test_res=0;
    int cur_errno, expec_errno;

    if ( utctime.is_bad | utcoff.is_bad | dstoff.is_bad )
    {
	expec_errno = VD_BAD_ERRNO;
	expec_res = VD_BAD_RES;
    }
    else
    {
	expec_errno = VD_GOOD_ERRNO;
	expec_res = VD_GOOD_RES;
    }

    printf(" %-10s | %-12s | %-12s ||", utctime.name, utcoff.name, dstoff.name);

    errno = 0;
    cur_res = vd_settimeofday(utctime.val, utcoff.val, dstoff.val );
    cur_errno = errno;

    if (cur_errno != expec_errno)
    {
	print_fail();
	test_res = 1;
    }
    else print_pass();

    if (cur_res != expec_res)
    {
	print_fail();
	test_res = 1;
    }
    else print_pass();

#ifdef VD_TST_VERBOSE
    if (test_res==1)
    {
	printf("\n +++ vd_settimeofday return %2d, errno set to %d", cur_res, cur_errno);
	printf("\n +++ expected:       return %2d, errno set to %d", expec_res, expec_errno);
    }
#endif

    printf("\n");
    return test_res;
}

static int do_test (void)
{
    int status, res = 0;
    _vdtime_t curr_vd;
    int n_utctime_val, n_utcoff_val, n_dstoff_val;
    int i_ut, i_uo, i_d;

    /* Store the /proc/vdtime content */
    status = _vd_time_read(&curr_vd);
    if (status) return status;

    n_utctime_val = sizeof(test_utctime)/sizeof(utctime_val);
    n_utcoff_val = sizeof(test_utcoff)/sizeof(utcoff_val);
    n_dstoff_val = sizeof(test_dstoff)/sizeof(dstoff_val);

    printf(" %-10s | %-12s | %-12s || %5s | %5s\n",
	"utctime", "utcoff", "dstoff", "errno", "return");
    printf(" -----------------------------");
    printf("------------------------------\n");

    /* Test different combinations of bad&good arguments */
    for (i_ut = 0; i_ut < n_utctime_val; i_ut++)
    {
	for (i_uo = 0; i_uo < n_utcoff_val; i_uo++)
	{
	    for (i_d = 0; i_d < n_dstoff_val; i_d++)
	    {
		res |= test_vd_settimeofay_badvals
		    (test_utctime[i_ut], test_utcoff[i_uo],
		     test_dstoff[i_d]);
	    }
	}
    }

    /* Set the original /proc/vdtime back */
    status = _vd_time_write(curr_vd);
    if (status) return status;

    return res;
}

#define TEST_FUNCTION do_test()
#include "../test-skeleton.c"
