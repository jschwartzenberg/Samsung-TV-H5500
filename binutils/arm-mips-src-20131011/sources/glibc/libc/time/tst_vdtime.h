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

#ifndef __VD_TIME_H
#define __VD_TIME_H

#define VD_VALUES_NUM 4

/* Conversions hour <-> min <-> sec <-> msec <-> usec */
#define MIN_IN_HOUR	(60L)
#define SEC_IN_MIN	(60L)
#define SEC_IN_HOUR	(SEC_IN_MIN*MIN_IN_HOUR)
#define MSEC_IN_SEC	(1000L)
#define MSEC_IN_MIN	(MSEC_IN_SEC*SEC_IN_MIN)
#define USEC_IN_MSEC	(1000L)
#define USEC_IN_SEC	(USEC_IN_MSEC*MSEC_IN_SEC)
#define MSEC2SEC_REM(msec)  \
	( msec - (msec/MSEC_IN_SEC)*MSEC_IN_SEC )

/* Uncomment for debug print */
#define VD_TST_VERBOSE

typedef struct vd_time_struct {
    long long	msec_offset;
    int 	min_tz;
    int		min_dst;
    int		chsum;
} _vdtime_t;

/* Read the /proc/vd_time content */
int _vd_time_read(_vdtime_t* vd_s)
{
    FILE *vdt_fp;
    _vdtime_t user_vdt;
    int num;

    vdt_fp = fopen("/proc/vd_time", "r");
    if (vdt_fp==NULL)
    {
        perror("Can not open /proc/vd_time:");
        return -1;
    }
    num = fscanf(vdt_fp, "%lld %d %d %d", &user_vdt.msec_offset,
	&user_vdt.min_tz, &user_vdt.min_dst, &user_vdt.chsum);
    if (num != VD_VALUES_NUM)
    {
        printf("Failed to scanf - %d values found\n", num);
        return -1;
    }
    #ifdef VD_TST_VERBOSE
    printf("/proc/vd_time successfully read: %lld %d %d %d\n",
	user_vdt.msec_offset, user_vdt.min_tz,
	user_vdt.min_dst, user_vdt.chsum);
    #endif

    vd_s->msec_offset = user_vdt.msec_offset;
    vd_s->min_tz = user_vdt.min_tz;
    vd_s->min_dst = user_vdt.min_dst;
    vd_s->chsum = user_vdt.chsum;

    fclose(vdt_fp);
    return 0;
}

/* Write the /proc/vd_time content */
int _vd_time_write(_vdtime_t vd_s)
{
    FILE *vdt_fp;
    int num;

    vdt_fp = fopen("/proc/vd_time", "w");
    if (vdt_fp==NULL)
    {
        perror("Can not open /proc/vd_time:");
        return -1;
    }
    errno = 0;

    num = fprintf(vdt_fp, "%lld %d %d %d", vd_s.msec_offset,
	vd_s.min_tz, vd_s.min_dst, vd_s.chsum);
    if (num < 0)
    {
        printf("Failed to fprintf - %d returned\n", num);
        return -1;
    }
    #ifdef VD_TST_VERBOSE
    printf("/proc/vd_time successfully wrote: %lld %d %d %d\n",
	vd_s.msec_offset, vd_s.min_tz, vd_s.min_dst, vd_s.chsum);
    #endif

    fclose(vdt_fp);
    return 0;
}


#endif
