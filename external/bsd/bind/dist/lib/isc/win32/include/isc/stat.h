/*	$NetBSD: stat.h,v 1.2.6.1.4.1 2014/12/31 11:59:07 msaitoh Exp $	*/

/*
 * Copyright (C) 2004, 2007, 2009, 2012  Internet Systems Consortium, Inc. ("ISC")
 * Copyright (C) 2000, 2001, 2003  Internet Software Consortium.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* Id: stat.h,v 1.9 2009/10/01 23:48:08 tbox Exp  */

#ifndef ISC_STAT_H
#define ISC_STAT_H 1

#include <sys/stat.h>

/*
 * Windows doesn't typedef this.
 */
typedef unsigned short mode_t;

/* open() under unix allows setting of read/write permissions
 * at the owner, group and other levels.  These don't exist in NT
 * We'll just map them all to the NT equivalent
 */

#define S_IREAD	_S_IREAD	/* read permission, owner */
#define S_IWRITE _S_IWRITE	/* write permission, owner */
#define S_IRUSR _S_IREAD	/* Owner read permission */
#define S_IWUSR _S_IWRITE	/* Owner write permission */
#define S_IRGRP _S_IREAD	/* Group read permission */
#define S_IWGRP _S_IWRITE	/* Group write permission */
#define S_IROTH _S_IREAD	/* Other read permission */
#define S_IWOTH _S_IWRITE	/* Other write permission */

#ifndef S_IFMT
# define S_IFMT   _S_IFMT
#endif
#ifndef S_IFDIR
# define S_IFDIR  _S_IFDIR
#endif
#ifndef S_IFCHR
# define S_IFCHR  _S_IFCHR
#endif
#ifndef S_IFREG
# define S_IFREG  _S_IFREG
#endif

#ifndef S_ISDIR
# define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
# define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#endif

#endif /* ISC_STAT_H */
