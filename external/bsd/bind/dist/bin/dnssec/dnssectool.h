/*	$NetBSD: dnssectool.h,v 1.2.6.1.4.3 2015/11/17 19:31:08 bouyer Exp $	*/

/*
 * Copyright (C) 2004, 2007-2012, 2014  Internet Systems Consortium, Inc. ("ISC")
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

/* Id: dnssectool.h,v 1.33 2011/10/20 23:46:51 tbox Exp  */

#ifndef DNSSECTOOL_H
#define DNSSECTOOL_H 1

#include <isc/log.h>
#include <isc/stdtime.h>
#include <dns/rdatastruct.h>
#include <dst/dst.h>

#define check_dns_dbiterator_current(result) \
	check_result((result == DNS_R_NEWORIGIN) ? ISC_R_SUCCESS : result, \
		     "dns_dbiterator_current()")


typedef void (fatalcallback_t)(void);

ISC_PLATFORM_NORETURN_PRE void
fatal(const char *format, ...)
ISC_FORMAT_PRINTF(1, 2) ISC_PLATFORM_NORETURN_POST;

void
setfatalcallback(fatalcallback_t *callback);

void
check_result(isc_result_t result, const char *message);

void
vbprintf(int level, const char *fmt, ...) ISC_FORMAT_PRINTF(2, 3);

void
version(const char *program);

void
type_format(const dns_rdatatype_t type, char *cp, unsigned int size);
#define TYPE_FORMATSIZE 20

void
sig_format(dns_rdata_rrsig_t *sig, char *cp, unsigned int size);
#define SIG_FORMATSIZE (DNS_NAME_FORMATSIZE + DNS_SECALG_FORMATSIZE + sizeof("65535"))

void
setup_logging(int verbose, isc_mem_t *mctx, isc_log_t **logp);

void
cleanup_logging(isc_log_t **logp);

void
setup_entropy(isc_mem_t *mctx, const char *randomfile, isc_entropy_t **ectx);

void
cleanup_entropy(isc_entropy_t **ectx);

dns_ttl_t strtottl(const char *str);

isc_stdtime_t
strtotime(const char *str, isc_int64_t now, isc_int64_t base,
	  isc_boolean_t *setp);

dns_rdataclass_t
strtoclass(const char *str);

isc_result_t
try_dir(const char *dirname);

void
check_keyversion(dst_key_t *key, char *keystr);

void
set_keyversion(dst_key_t *key);

isc_boolean_t
key_collision(dst_key_t *key, dns_name_t *name, const char *dir,
	      isc_mem_t *mctx, isc_boolean_t *exact);

isc_boolean_t
is_delegation(dns_db_t *db, dns_dbversion_t *ver, dns_name_t *origin,
		      dns_name_t *name, dns_dbnode_t *node, isc_uint32_t *ttlp);

void
verifyzone(dns_db_t *db, dns_dbversion_t *ver,
		   dns_name_t *origin, isc_mem_t *mctx,
		   isc_boolean_t ignore_kskflag, isc_boolean_t keyset_kskonly);
#endif /* DNSSEC_DNSSECTOOL_H */
