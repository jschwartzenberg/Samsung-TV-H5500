/* Copyright (C) 2012-2013 Free Software Foundation, Inc.
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

#include <ftw.h>
#include <bits/libc-lock.h>

__libc_lock_define_initialized (static, ftw_lock)
__libc_lock_define_initialized (static, nftw_lock)

int ftw_r (const char *path, __ftw_func_t func, int descriptors)
{
  int ret;
  __libc_lock_lock (ftw_lock);
  ret = ftw (path, func, descriptors);
  __libc_lock_unlock (ftw_lock);
  return ret;
}

int nftw_r (const char *path, __nftw_func_t func, int descriptors, int flags)
{
  int ret;
  __libc_lock_lock (nftw_lock);
  ret = nftw (path, func, descriptors, flags);
  __libc_lock_unlock (nftw_lock);
  return ret;
}

