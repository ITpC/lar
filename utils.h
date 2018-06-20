/*
 * Copyright Pavel Kraynyukhov 2018.
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 *             http://www.boost.org/LICENSE_1_0.txt)
 *
 *  $Id: lar:utils.h Jun 15, 2018 12:47:12 PM $
 *
 *  EMail: pavel.kraynyukhov@gmail.com
 */


#ifndef __UTILS_H__
#define __UTILS_H__
#include <unistd.h>

const size_t memavail()
{
  size_t pages=sysconf(_SC_AVPHYS_PAGES);
  size_t page_size=sysconf(_SC_PAGE_SIZE);
  return pages*page_size;
}


#endif /* __UTILS_H__ */

