/*	$NetBSD: tlsa_52.h,v 1.1.1.1.2.2.4.1 2014/12/31 11:59:00 msaitoh Exp $	*/

/*
 * Copyright (C) 2012, 2014  Internet Systems Consortium, Inc. ("ISC")
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

#ifndef GENERIC_TLSA_52_H
#define GENERIC_TLSA_52_H 1

/*!
 *  \brief per rfc6698.txt
 */
typedef struct dns_rdata_tlsa {
	dns_rdatacommon_t	common;
	isc_mem_t		*mctx;
	isc_uint8_t		usage;
	isc_uint8_t		selector;
	isc_uint8_t		match;
	isc_uint16_t		length;
	unsigned char		*data;
} dns_rdata_tlsa_t;

#endif /* GENERIC_TLSA_52_H */
