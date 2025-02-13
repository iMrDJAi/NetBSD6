/*	$NetBSD: rpz.c,v 1.2.8.1.4.1 2014/12/31 11:58:46 msaitoh Exp $	*/

/*
 * Copyright (C) 2011-2013  Internet Systems Consortium, Inc. ("ISC")
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

/* Id */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define USAGE "usage: nsip | nsdname\n"

int
main(int argc, char **argv)
{
	if (argc != 2) {
		fputs(USAGE, stderr);
		return (1);
	}

	if (!strcasecmp(argv[1], "nsip")) {
#ifdef ENABLE_RPZ_NSIP
		return (0);
#else
		return (1);
#endif
	    }

	if (!strcasecmp(argv[1], "nsdname")) {
#ifdef ENABLE_RPZ_NSDNAME
		return (0);
#else
		return (1);
#endif
	    }

	    fputs(USAGE, stderr);
	    return (1);
}
