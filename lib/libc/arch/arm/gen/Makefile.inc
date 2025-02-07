# $NetBSD: Makefile.inc,v 1.16.8.2 2012/07/31 08:13:09 martin Exp $

SRCS+=	alloca.S byte_swap_2.S byte_swap_4.S bswap64.c divsi3.S \
	fabs.c flt_rounds.c

# Common ieee754 constants and functions
SRCS+=	infinityf_ieee754.c infinity_ieee754.c infinityl_dbl_ieee754.c
SRCS+=	fpclassifyf_ieee754.c fpclassifyd_ieee754.c
SRCS+=	isfinitef_ieee754.c isfinited_ieee754.c
SRCS+=	isinff_ieee754.c isinfd_ieee754.c
SRCS+=	isnanf_ieee754.c isnand_ieee754.c
SRCS+=	signbitf_ieee754.c signbitd_ieee754.c

SRCS+=	nanf.c

SRCS+=	setjmp.S
SRCS+=	_setjmp.S
SRCS+=	sigsetjmp.S

SRCS+=	makecontext.c resumecontext.c swapcontext.S

SRCS+=	_lwp.c

LSRCS.arm.gen=	Lint_bswap16.c Lint_bswap32.c Lint_swapcontext.c
LSRCS+=		${LSRCS.arm.gen}
DPSRCS+=	${LSRCS.arm.gen}
CLEANFILES+=	${LSRCS.arm.gen}
