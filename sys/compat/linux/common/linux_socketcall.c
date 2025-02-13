/*	$NetBSD: linux_socketcall.c,v 1.39.40.1 2014/08/08 03:14:10 msaitoh Exp $	*/

/*-
 * Copyright (c) 1995, 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Frank van der Linden and Eric Haszlakiewicz.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: linux_socketcall.c,v 1.39.40.1 2014/08/08 03:14:10 msaitoh Exp $");

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/malloc.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/file.h>
#include <sys/filedesc.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/device.h>
#include <sys/ktrace.h>

#include <sys/syscallargs.h>

#include <compat/sys/socket.h>

#include <compat/linux/common/linux_types.h>
#include <compat/linux/common/linux_util.h>
#include <compat/linux/common/linux_signal.h>
#include <compat/linux/common/linux_ioctl.h>
#include <compat/linux/common/linux_socket.h>
#include <compat/linux/common/linux_socketcall.h>
#include <compat/linux/common/linux_sockio.h>

#include <compat/linux/linux_syscallargs.h>

#undef DPRINTF
#ifdef DEBUG_LINUX
#define DPRINTF(a)	uprintf a
#else
#define DPRINTF(a)
#endif

/* Used on: arm, i386, m68k, mips, ppc, sparc, sparc64 */
/* Not used on: alpha, amd64 */

/*
 * This file contains the linux_socketcall() multiplexer.  Arguments
 * for the various socket calls are on the user stack. A pointer
 * to them is the only thing that is passed.  We copyin the arguments
 * here so the individual functions can assume they are normal syscalls.
 */

/* The sizes of the arguments.  Used for copyin. */
static const struct {
	const char *name;
	int argsize;
} linux_socketcall[LINUX_MAX_SOCKETCALL+1] = {
	{"invalid",	-1},						/* 0 */
	{"socket",	sizeof(struct linux_sys_socket_args)},		/* 1 */
	{"bind",	sizeof(struct linux_sys_bind_args)},		/* 2 */
	{"connect",	sizeof(struct linux_sys_connect_args)},		/* 3 */
	{"listen",	sizeof(struct linux_sys_listen_args)},		/* 4 */
	{"accept",	sizeof(struct linux_sys_accept_args)},		/* 5 */
	{"getsockname",	sizeof(struct linux_sys_getsockname_args)},	/* 6 */
	{"getpeername",	sizeof(struct linux_sys_getpeername_args)},	/* 7 */
	{"socketpair",	sizeof(struct linux_sys_socketpair_args)},	/* 8 */
	{"send",	sizeof(struct linux_sys_send_args)},		/* 9 */
	{"recv",	sizeof(struct linux_sys_recv_args)},		/* 10 */
	{"sendto",	sizeof(struct linux_sys_sendto_args)},		/* 11 */
	{"recvfrom",	sizeof(struct linux_sys_recvfrom_args)},	/* 12 */
	{"shutdown",	sizeof(struct linux_sys_shutdown_args)},	/* 13 */
	{"setsockopt",	sizeof(struct linux_sys_setsockopt_args)},	/* 14 */
	{"getsockopt",	sizeof(struct linux_sys_getsockopt_args)},	/* 15 */
	{"sendmsg",	sizeof(struct linux_sys_sendmsg_args)},		/* 16 */
	{"recvmsg",	sizeof(struct linux_sys_recvmsg_args)},		/* 17 */
};

/*
 * Entry point to all Linux socket calls. Just check which call to
 * make and take appropriate action.
 */
int
linux_sys_socketcall(struct lwp *l, const struct linux_sys_socketcall_args *uap, register_t *retval)
{
	/* {
		syscallarg(int) what;
		syscallarg(void *) args;
	} */
	struct linux_socketcall_dummy_args lda;
	int error;

	if (SCARG(uap, what) <= 0 || SCARG(uap, what) > LINUX_MAX_SOCKETCALL)
		return ENOSYS;

	if ((error = copyin(SCARG(uap, args), &lda,
	    linux_socketcall[SCARG(uap, what)].argsize))) {
		DPRINTF(("copyin for %s failed %d\n",
		linux_socketcall[SCARG(uap, what)].name, error));
		return error;
	}

	ktrkuser(linux_socketcall[SCARG(uap, what)].name, &lda,
	    linux_socketcall[SCARG(uap, what)].argsize);

#ifdef DEBUG_LINUX
	/* dump the passed argument data */
	{
        	DPRINTF(("linux_socketcall('%s'): ",
		    linux_socketcall[SCARG(uap, what)].name));

		if (SCARG(uap, what) == LINUX_SYS_socket) {
			DPRINTF(("[dom %d type %d proto %d]\n",
				lda.dummy_ints[0],
				lda.dummy_ints[1],
				lda.dummy_ints[2]));
		} else {
			int i, sz;
			u_int8_t *data = (u_int8_t *)&lda.dummy_ints[1];

			sz = linux_socketcall[SCARG(uap, what)].argsize
			    - sizeof(lda.dummy_ints[0]);

			DPRINTF(("socket %d [", lda.dummy_ints[0]));
			for(i=0; i < sz; i++)
				DPRINTF(("%02x ", data[i]));
			DPRINTF(("]\n"));
		}
	}
#endif

	switch (SCARG(uap, what)) {
	case LINUX_SYS_socket:
		error = linux_sys_socket(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_bind:
		error = linux_sys_bind(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_connect:
		error = linux_sys_connect(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_listen:
		error = sys_listen(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_accept:
		error = linux_sys_accept(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_getsockname:
		error = linux_sys_getsockname(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_getpeername:
		error = linux_sys_getpeername(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_socketpair:
		error = linux_sys_socketpair(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_send:
		error = linux_sys_send(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_recv:
		error = linux_sys_recv(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_sendto:
		error = linux_sys_sendto(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_recvfrom:
		error = linux_sys_recvfrom(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_shutdown:
		error = sys_shutdown(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_setsockopt:
		error = linux_sys_setsockopt(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_getsockopt:
		error = linux_sys_getsockopt(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_sendmsg:
		error = linux_sys_sendmsg(l, (void *)&lda, retval);
		break;
	case LINUX_SYS_recvmsg:
		error = linux_sys_recvmsg(l, (void *)&lda, retval);
		break;
	default:
		error = ENOSYS;
		break;
	}

	DPRINTF(("sys_%s() = %d\n", linux_socketcall[SCARG(uap, what)].name,
	    error));
	return error;
}
