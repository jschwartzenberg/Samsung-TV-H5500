/* Copyright (C) 2013  Free Software Foundation, Inc.
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
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define VD_VALUES_NUM 4

/* Conversions hour <-> min,  sec <-> msec <-> usec */
#define MIN_IN_HOUR	 (60)
#define MSEC_IN_SEC	 (1000L)
#define USEC_IN_MSEC	 (1000L)
#define USEC_IN_SEC	 (USEC_IN_MSEC*MSEC_IN_SEC)

#define VD_TIME_LBOUND	 (0L)
#define VD_TIME_RBOUND	 (LLONG_MAX)
#define VD_TZ_LBOUND	 (-12 * MIN_IN_HOUR)
#define VD_TZ_RBOUND	 (+14 * MIN_IN_HOUR)
#define VD_DST_LBOUND	 (0)
#define VD_DST_RBOUND	 (+6 * MIN_IN_HOUR)

/* Calculate check sum for arbitary array */

/* Set the current time of day and timezone information to kernel vd_time.
   This call is restricted to the super-user.  */
int
__vd_settimeofday (utctime, utcoff, dstoff)
     long long utctime;
     int utcoff;
     int dstoff;
{
    int badval_flag = 0;
    FILE *vdtime_fp;
    struct timeval sys_tv;
    long long msec_offset;
    int i;
    int* int_p_msec_offset;
    int chsum = 0;
    int res;

    /* Check arguments ranges */
    if ( (utctime < VD_TIME_LBOUND) || (utctime > VD_TIME_RBOUND) )
    {
	fprintf(stderr,
	 "\n##### VD-GLIBC message, bad utctime value %lld\n", utctime );
	badval_flag = 1;
    }
    if ( (utcoff < VD_TZ_LBOUND) || (utcoff > VD_TZ_RBOUND) )
    {
	fprintf(stderr,
	 "\n##### VD-GLIBC message, bad utcoff value %d\n", utcoff);
	badval_flag = 1;
    }
    if ( (dstoff < VD_DST_LBOUND) || (dstoff > VD_DST_RBOUND) )
    {
	fprintf(stderr,
	 "\n##### VD-GLIBC message, bad dstoff value %d\n", dstoff);
	badval_flag = 1;
    }
    if (badval_flag)
    {
	errno = EINVAL;
	return -1;
    }

    /* Open /proc/vd_time for later write */
    vdtime_fp = fopen("/proc/vd_time", "w");
    if (vdtime_fp==NULL)
    {
	fprintf(stderr,
	 "\n##### VD-GLIBC message, /proc/vd_time open error\n");
	return -1;
    }

    /* Get current system time */
    res = __gettimeofday(&sys_tv, (struct timezone *)NULL);
    if ( res )
    {
	fclose(vdtime_fp);
	fprintf(stderr,
	 "\n##### VD-GLIBC message, gettimeofday error\n");
	return res;
    }

    /* Calculate vd time offset to system time in milliseconds */
    msec_offset = ((long long)sys_tv.tv_sec * MSEC_IN_SEC
	        +  (long long)sys_tv.tv_usec / USEC_IN_MSEC) - utctime;

    /* Calculate check sum */
    int_p_msec_offset = (int*)(&msec_offset);
    for (i = 0; i < sizeof(msec_offset)/sizeof(int) ; i++)
    {
        chsum ^= int_p_msec_offset[i];
    }
    chsum ^= utcoff;
    chsum ^= dstoff;

    /* Print new time in /proc/vdtime */
    res = fprintf(vdtime_fp, "%lld %d %d %d",
            msec_offset, utcoff, dstoff, chsum);
    if (res < 0)
    {
        fclose(vdtime_fp);
	fprintf(stderr,
	 "\n##### VD-GLIBC message, write to /proc/vd_time error\n");
        return -1;
    }

    fclose(vdtime_fp);
    return 0;
}
libc_hidden_def(__vd_settimeofday)
weak_alias (__vd_settimeofday, vd_settimeofday)

