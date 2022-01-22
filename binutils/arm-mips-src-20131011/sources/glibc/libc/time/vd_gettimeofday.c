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

/* Conversions sec <-> msec <-> usec */
#define MSEC_IN_SEC     (1000L)
#define USEC_IN_MSEC    (1000L)
#define USEC_IN_SEC     (USEC_IN_MSEC*MSEC_IN_SEC)

#define VD_VALUES_NUM 4

/* Get the time of day and timezone information from kernel vd_time */
long long
__vd_gettimeofday (void)
{
    FILE *vdtime_fp;
    struct timeval sys_tv;
    long long msec_offset, msec_utctime, sec_utctime;
    int min_tz, min_dst, chsum;
    int res;

    vdtime_fp = fopen("/proc/vd_time", "r");
    if (vdtime_fp==NULL)
    {
	fclose(vdtime_fp);
	fprintf(stderr,
	 "\n##### VD-GLIBC message, /proc/vd_time open error\n");
	return -1L;
    }

    res = fscanf(vdtime_fp, "%lld %d %d %d",
            &msec_offset, &min_tz, &min_dst, &chsum);
    if (res != VD_VALUES_NUM)
    {
        fclose(vdtime_fp);
	fprintf(stderr,
	 "\n##### VD-GLIBC message, read /proc/vd_time error\n");
        return res;
    }
    fclose(vdtime_fp);

    /* Get current system time */
    res = __gettimeofday(&sys_tv, (struct timezone *)NULL);
    if ( res )
    {
	fprintf(stderr,
	 "\n##### VD-GLIBC message, gettimeofday error\n");
	return -1L;
    }

    /* Calculate vd utc time in milliseconds */
    msec_utctime = ((long long)sys_tv.tv_sec * MSEC_IN_SEC
		 +  (long long)sys_tv.tv_usec / USEC_IN_MSEC) - msec_offset;
    sec_utctime = msec_utctime / MSEC_IN_SEC;

    return sec_utctime;
}
libc_hidden_def(__vd_gettimeofday)
weak_alias (__vd_gettimeofday, vd_gettimeofday)

