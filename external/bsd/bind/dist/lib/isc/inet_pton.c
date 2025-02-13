/*	$NetBSD: inet_pton.c,v 1.2.6.1.4.1 2014/12/31 11:59:03 msaitoh Exp $	*/

/*
 * Copyright (C) 2004, 2005, 2007, 2013, 2014  Internet Systems Consortium, Inc. ("ISC")
 * Copyright (C) 1996-2003  Internet Software Consortium.
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

/*! \file */

#if defined(LIBC_SCCS) && !defined(lint)
static char rcsid[] =
	"Id: inet_pton.c,v 1.19 2007/06/19 23:47:17 tbox Exp ";
#endif /* LIBC_SCCS and not lint */

#include <config.h>

#include <errno.h>
#include <string.h>

#include <isc/net.h>

/*% INT16 Size */
#define NS_INT16SZ	 2
/*% IPv4 Address Size */
#define NS_INADDRSZ	 4
/*% IPv6 Address Size */
#define NS_IN6ADDRSZ	16

/*
 * WARNING: Don't even consider trying to compile this on a system where
 * sizeof(int) < 4.  sizeof(int) > 4 is fine; all the world's not a VAX.
 */

static int inet_pton4(const char *src, unsigned char *dst);
static int inet_pton6(const char *src, unsigned char *dst);

/*% 
 *	convert from presentation format (which usually means ASCII printable)
 *	to network format (which is usually some kind of binary format).
 * \return
 *	1 if the address was valid for the specified address family
 *	0 if the address wasn't valid (`dst' is untouched in this case)
 *	-1 if some other error occurred (`dst' is untouched in this case, too)
 * \author
 *	Paul Vixie, 1996.
 */
int
isc_net_pton(int af, const char *src, void *dst) {
	switch (af) {
	case AF_INET:
		return (inet_pton4(src, dst));
	case AF_INET6:
		return (inet_pton6(src, dst));
	default:
		errno = EAFNOSUPPORT;
		return (-1);
	}
	/* NOTREACHED */
}

/*!\fn static int inet_pton4(const char *src, unsigned char *dst)
 * \brief
 *	like inet_aton() but without all the hexadecimal and shorthand.
 * \return
 *	1 if `src' is a valid dotted quad, else 0.
 * \note
 *	does not touch `dst' unless it's returning 1.
 * \author
 *	Paul Vixie, 1996.
 */
static int
inet_pton4(const char *src, unsigned char *dst) {
	static const char digits[] = "0123456789";
	int saw_digit, octets, ch;
	unsigned char tmp[NS_INADDRSZ], *tp;

	saw_digit = 0;
	octets = 0;
	*(tp = tmp) = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;

		if ((pch = strchr(digits, ch)) != NULL) {
			unsigned int new = *tp * 10;

			new += (int)(pch - digits);
			if (saw_digit && *tp == 0)
				return (0);
			if (new > 255)
				return (0);
			*tp = new;
			if (!saw_digit) {
				if (++octets > 4)
					return (0);
				saw_digit = 1;
			}
		} else if (ch == '.' && saw_digit) {
			if (octets == 4)
				return (0);
			*++tp = 0;
			saw_digit = 0;
		} else
			return (0);
	}
	if (octets < 4)
		return (0);
	memmove(dst, tmp, NS_INADDRSZ);
	return (1);
}

/*%
 *	convert presentation level address to network order binary form.
 * \return
 *	1 if `src' is a valid [RFC1884 2.2] address, else 0.
 * \note
 *	(1) does not touch `dst' unless it's returning 1.
 * \note
 *	(2) :: in a full address is silently ignored.
 * \author
 *	inspired by Mark Andrews.
 * \author
 *	Paul Vixie, 1996.
 */
static int
inet_pton6(const char *src, unsigned char *dst) {
	static const char xdigits_l[] = "0123456789abcdef",
			  xdigits_u[] = "0123456789ABCDEF";
	unsigned char tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
	const char *xdigits, *curtok;
	int ch, seen_xdigits;
	unsigned int val;

	memset((tp = tmp), '\0', NS_IN6ADDRSZ);
	endp = tp + NS_IN6ADDRSZ;
	colonp = NULL;
	/* Leading :: requires some special handling. */
	if (*src == ':')
		if (*++src != ':')
			return (0);
	curtok = src;
	seen_xdigits = 0;
	val = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;

		if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
			pch = strchr((xdigits = xdigits_u), ch);
		if (pch != NULL) {
			val <<= 4;
			val |= (pch - xdigits);
			if (++seen_xdigits > 4)
				return (0);
			continue;
		}
		if (ch == ':') {
			curtok = src;
			if (!seen_xdigits) {
				if (colonp)
					return (0);
				colonp = tp;
				continue;
			}
			if (tp + NS_INT16SZ > endp)
				return (0);
			*tp++ = (unsigned char) (val >> 8) & 0xff;
			*tp++ = (unsigned char) val & 0xff;
			seen_xdigits = 0;
			val = 0;
			continue;
		}
		if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) &&
		    inet_pton4(curtok, tp) > 0) {
			tp += NS_INADDRSZ;
			seen_xdigits = 0;
			break;	/* '\0' was seen by inet_pton4(). */
		}
		return (0);
	}
	if (seen_xdigits) {
		if (tp + NS_INT16SZ > endp)
			return (0);
		*tp++ = (unsigned char) (val >> 8) & 0xff;
		*tp++ = (unsigned char) val & 0xff;
	}
	if (colonp != NULL) {
		/*
		 * Since some memmove()'s erroneously fail to handle
		 * overlapping regions, we'll do the shift by hand.
		 */
		const int n = (int)(tp - colonp);
		int i;

		if (tp == endp)
			return (0);
		for (i = 1; i <= n; i++) {
			endp[- i] = colonp[n - i];
			colonp[n - i] = 0;
		}
		tp = endp;
	}
	if (tp != endp)
		return (0);
	memmove(dst, tmp, NS_IN6ADDRSZ);
	return (1);
}
