/*	$NetBSD: if_tokensubr.c,v 1.61.14.1 2017/02/05 05:46:51 snj Exp $	*/

/*
 * Copyright (c) 1982, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: NetBSD: if_fddisubr.c,v 1.2 1995/08/19 04:35:29 cgd Exp
 */

/*
 * Copyright (c) 1997-1999 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Onno van der Linden.
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

/*
 * Copyright (c) 1995
 *	Matt Thomas.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of its contributors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: NetBSD: if_fddisubr.c,v 1.2 1995/08/19 04:35:29 cgd Exp
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: if_tokensubr.c,v 1.61.14.1 2017/02/05 05:46:51 snj Exp $");

#include "opt_inet.h"
#include "opt_atalk.h"
#include "opt_iso.h"
#include "opt_gateway.h"


#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/syslog.h>

#include <sys/cpu.h>

#include <net/if.h>
#include <net/netisr.h>
#include <net/route.h>
#include <net/if_llc.h>
#include <net/if_dl.h>
#include <net/if_types.h>

#include <net/bpf.h>

#include <net/if_ether.h>
#include <net/if_token.h>

#include "carp.h"
#if NCARP > 0
#include <netinet/ip_carp.h>
#endif

#ifdef INET
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/if_inarp.h>
#endif


#ifdef DECNET
#include <netdnet/dn.h>
#endif

#ifdef ISO
#include <netiso/argo_debug.h>
#include <netiso/iso.h>
#include <netiso/iso_var.h>
#include <netiso/iso_snpac.h>
#endif


#define senderr(e) { error = (e); goto bad;}

#if defined(__bsdi__) || defined(__NetBSD__)
#define	RTALLOC1(a, b)			rtalloc1(a, b)
#define	ARPRESOLVE(a, b, c, d, e, f)	arpresolve(a, b, c, d, e)
#define	TYPEHTONS(t)			(t)
#elif defined(__FreeBSD__)
#define	RTALLOC1(a, b)			rtalloc1(a, b, 0UL)
#define	ARPRESOLVE(a, b, c, d, e, f)	arpresolve(a, b, c, d, e, f)
#define	TYPEHTONS(t)			(htons(t))
#endif

#define RCF_ALLROUTES (2 << 8) | TOKEN_RCF_FRAME2 | TOKEN_RCF_BROADCAST_ALL
#define RCF_SINGLEROUTE (2 << 8) | TOKEN_RCF_FRAME2 | TOKEN_RCF_BROADCAST_SINGLE

static int	token_output(struct ifnet *, struct mbuf *,
			     const struct sockaddr *, struct rtentry *);
static void	token_input(struct ifnet *, struct mbuf *);

/*
 * Token Ring output routine.
 * Encapsulate a packet of type family for the local net.
 * Assumes that ifp is actually pointer to arphdr structure.
 * XXX route info has to go into the same mbuf as the header
 */
static int
token_output(struct ifnet *ifp0, struct mbuf *m0, const struct sockaddr *dst,
    struct rtentry *rt0)
{
	uint16_t etype;
	int error = 0;
	u_char edst[ISO88025_ADDR_LEN];
	struct mbuf *m = m0;
	struct rtentry *rt;
	struct mbuf *mcopy = NULL;
	struct token_header *trh;
#ifdef INET
	struct arphdr *ah = (struct arphdr *)ifp0;
#endif /* INET */
	struct token_rif *rif = NULL;
	struct token_rif bcastrif;
	struct ifnet *ifp = ifp0;
	size_t riflen = 0;
	ALTQ_DECL(struct altq_pktattr pktattr;)

#if NCARP > 0
	if (ifp->if_type == IFT_CARP) {
		struct ifaddr *ifa;

		/* loop back if this is going to the carp interface */
		if (dst != NULL && ifp0->if_link_state == LINK_STATE_UP &&
		    (ifa = ifa_ifwithaddr(dst)) != NULL &&
		    ifa->ifa_ifp == ifp0)
			return (looutput(ifp0, m, dst, rt0));

		ifp = ifp->if_carpdev;
		ah = (struct arphdr *)ifp;

		if ((ifp0->if_flags & (IFF_UP|IFF_RUNNING)) !=
		    (IFF_UP|IFF_RUNNING))
			senderr(ENETDOWN);
	}
#endif /* NCARP > 0 */

	if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING))
		senderr(ENETDOWN);
	if ((rt = rt0)) {
		if ((rt->rt_flags & RTF_UP) == 0) {
			if ((rt0 = rt = RTALLOC1(dst, 1)))
				rt->rt_refcnt--;
			else
				senderr(EHOSTUNREACH);
		}
		if (rt->rt_flags & RTF_GATEWAY) {
			if (rt->rt_gwroute == 0)
				goto lookup;
			if (((rt = rt->rt_gwroute)->rt_flags & RTF_UP) == 0) {
				rtfree(rt); rt = rt0;
			lookup: rt->rt_gwroute = RTALLOC1(rt->rt_gateway, 1);
				if ((rt = rt->rt_gwroute) == 0)
					senderr(EHOSTUNREACH);
			}
		}
		if (rt->rt_flags & RTF_REJECT)
			if (rt->rt_rmx.rmx_expire == 0 ||
			    time_second < rt->rt_rmx.rmx_expire)
				senderr(rt == rt0 ? EHOSTDOWN : EHOSTUNREACH);
	}

	/*
	 * If the queueing discipline needs packet classification,
	 * do it before prepending link headers.
	 */
	IFQ_CLASSIFY(&ifp->if_snd, m, dst->sa_family, &pktattr);

	switch (dst->sa_family) {

#ifdef INET
	case AF_INET:
		if (m->m_flags & M_BCAST) {
			if (ifp->if_flags & IFF_LINK0) {
				if (ifp->if_flags & IFF_LINK1)
					bcastrif.tr_rcf = htons(RCF_ALLROUTES);
				else
					bcastrif.tr_rcf = htons(RCF_SINGLEROUTE);
				rif = &bcastrif;
				riflen = sizeof(rif->tr_rcf);
			}
			memcpy(edst, tokenbroadcastaddr, sizeof(edst));
		}
/*
 * XXX m->m_flags & M_MCAST   IEEE802_MAP_IP_MULTICAST ??
 */
		else {
			if (!ARPRESOLVE(ifp, rt, m, dst, edst, rt0))
				return (0);	/* if not yet resolved */
			rif = TOKEN_RIF((struct llinfo_arp *) rt->rt_llinfo);
			riflen = (ntohs(rif->tr_rcf) & TOKEN_RCF_LEN_MASK) >> 8;
		}
		/* If broadcasting on a simplex interface, loopback a copy. */
		if ((m->m_flags & M_BCAST) && (ifp->if_flags & IFF_SIMPLEX))
			mcopy = m_copy(m, 0, (int)M_COPYALL);
		etype = htons(ETHERTYPE_IP);
		break;
	case AF_ARP:
/*
 * XXX source routing, assume m->m_data contains the useful stuff
 */
		ah = mtod(m, struct arphdr *);
		ah->ar_hrd = htons(ARPHRD_IEEE802);

		switch (ntohs(ah->ar_op)) {
		case ARPOP_REVREQUEST:
		case ARPOP_REVREPLY:
			etype = htons(ETHERTYPE_REVARP);
			break;

		case ARPOP_REQUEST:
		case ARPOP_REPLY:
		default:
			etype = htons(ETHERTYPE_ARP);
		}

		if (m->m_flags & M_BCAST) {
			if (ifp->if_flags & IFF_LINK0) {
				if (ifp->if_flags & IFF_LINK1)
					bcastrif.tr_rcf = htons(RCF_ALLROUTES);
				else
					bcastrif.tr_rcf = htons(RCF_SINGLEROUTE);
				rif = &bcastrif;
				riflen = sizeof(rif->tr_rcf);
			}
			memcpy(edst, tokenbroadcastaddr, sizeof(edst));
		}
		else {
			void *tha = ar_tha(ah);
			if (tha == NULL) {
				m_freem(m);
				return 0;
			}
			memcpy(edst, tha, sizeof(edst));
			trh = (struct token_header *)M_TRHSTART(m);
			trh->token_ac = TOKEN_AC;
			trh->token_fc = TOKEN_FC;
			if (trh->token_shost[0] & TOKEN_RI_PRESENT) {
				struct token_rif *trrif;

				trrif = TOKEN_RIF(trh);
				riflen = (ntohs(trrif->tr_rcf) & TOKEN_RCF_LEN_MASK) >> 8;
			}
			memcpy((void *)trh->token_dhost, (void *)edst,
			    sizeof (edst));
			memcpy((void *)trh->token_shost, CLLADDR(ifp->if_sadl),
			    sizeof(trh->token_shost));
			if (riflen != 0)
				trh->token_shost[0] |= TOKEN_RI_PRESENT;
/*
 * compare (m->m_data - m->m_pktdat) with (sizeof(struct token_header) + riflen + ...
 */
			m->m_len += (sizeof(*trh) + riflen + LLC_SNAPFRAMELEN);
			m->m_data -= (sizeof(*trh) + riflen + LLC_SNAPFRAMELEN);
			m->m_pkthdr.len += (sizeof(*trh) + riflen + LLC_SNAPFRAMELEN);
			goto send;
		}
		break;
#endif
#ifdef	ISO
	case AF_ISO: {
		int	snpalen;
		struct	llc *l;
		const struct sockaddr_dl *sdl;

		if (rt && (sdl = satocsdl(rt->rt_gateway)) &&
		    sdl->sdl_family == AF_LINK && sdl->sdl_alen > 0) {
			memcpy(edst, CLLADDR(sdl), sizeof(edst));
		}
		else if ((error = iso_snparesolve(ifp,
		    (const struct sockaddr_iso *)dst, (char *)edst, &snpalen)))
			goto bad; /* Not resolved */
		/* If broadcasting on a simplex interface, loopback a copy. */
		if (*edst & 1)
			m->m_flags |= (M_BCAST|M_MCAST);
		if ((m->m_flags & M_BCAST) && (ifp->if_flags & IFF_SIMPLEX) &&
		    (mcopy = m_copy(m, 0, (int)M_COPYALL))) {
			M_PREPEND(mcopy, sizeof (*trh), M_DONTWAIT);
			if (mcopy) {
				trh = mtod(mcopy, struct token_header *);
				bcopy((void *)edst,
				    (void *)trh->token_dhost, sizeof (edst));
				bcopy(CLLADDR(ifp->if_sadl),
				    (void *)trh->token_shost, sizeof (edst));
			}
		}
		M_PREPEND(m, 3, M_DONTWAIT);
		if (m == NULL)
			return (0);
		etype = 0;
		l = mtod(m, struct llc *);
		l->llc_dsap = l->llc_ssap = LLC_ISO_LSAP;
		l->llc_control = LLC_UI;
#if defined(__FreeBSD__)
		IFDEBUG(D_ETHER)
			int i;
			printf("token_output: sending pkt to: ");
			for (i=0; i < ISO88025_ADDR_LEN; i++)
				printf("%x ", edst[i] & 0xff);
			printf("\n");
		ENDDEBUG
#endif
		} break;
#endif /* ISO */

	case AF_UNSPEC:
	{
		const struct ether_header *eh;
		eh = (const struct ether_header *)dst->sa_data;
		memcpy(edst, eh->ether_dhost, sizeof(edst));
		if (*edst & 1)
			m->m_flags |= (M_BCAST|M_MCAST);
		etype = TYPEHTONS(eh->ether_type);
		if (m->m_flags & M_BCAST) {
			if (ifp->if_flags & IFF_LINK0) {
				if (ifp->if_flags & IFF_LINK1)
					bcastrif.tr_rcf = htons(RCF_ALLROUTES);
				else
					bcastrif.tr_rcf = htons(RCF_SINGLEROUTE);
				rif = &bcastrif;
				riflen = sizeof(bcastrif.tr_rcf);
			}
		}
		break;
	}

	default:
		printf("%s: can't handle af%d\n", ifp->if_xname,
		    dst->sa_family);
		senderr(EAFNOSUPPORT);
	}


	if (mcopy)
		(void) looutput(ifp, mcopy, dst, rt);
	if (etype != 0) {
		struct llc *l;
		M_PREPEND(m, LLC_SNAPFRAMELEN, M_DONTWAIT);
		if (m == 0)
			senderr(ENOBUFS);
		l = mtod(m, struct llc *);
		l->llc_control = LLC_UI;
		l->llc_dsap = l->llc_ssap = LLC_SNAP_LSAP;
		l->llc_snap.org_code[0] = l->llc_snap.org_code[1] =
		    l->llc_snap.org_code[2] = 0;
		memcpy((void *) &l->llc_snap.ether_type, (void *) &etype,
		    sizeof(uint16_t));
	}

	/*
	 * Add local net header.  If no space in first mbuf,
	 * allocate another.
	 */

	M_PREPEND(m, (riflen + sizeof (*trh)), M_DONTWAIT);
	if (m == 0)
		senderr(ENOBUFS);
	trh = mtod(m, struct token_header *);
	trh->token_ac = TOKEN_AC;
	trh->token_fc = TOKEN_FC;
	memcpy((void *)trh->token_dhost, (void *)edst, sizeof (edst));
	memcpy((void *)trh->token_shost, CLLADDR(ifp->if_sadl),
	    sizeof(trh->token_shost));

	if (riflen != 0) {
		struct token_rif *trrif;

		trh->token_shost[0] |= TOKEN_RI_PRESENT;
		trrif = TOKEN_RIF(trh);
		memcpy(trrif, rif, riflen);
	}
#ifdef INET
send:
#endif

#if NCARP > 0
	if (ifp0 != ifp && ifp0->if_type == IFT_CARP) {
		memcpy((void *)trh->token_shost, CLLADDR(ifp0->if_sadl),	    
		    sizeof(trh->token_shost));
	}
#endif /* NCARP > 0 */

	return ifq_enqueue(ifp, m ALTQ_COMMA ALTQ_DECL(&pktattr));
bad:
	if (m)
		m_freem(m);
	return (error);
}

/*
 * Process a received token ring packet;
 * the packet is in the mbuf chain m with
 * the token ring header.
 */
static void
token_input(struct ifnet *ifp, struct mbuf *m)
{
	struct ifqueue *inq;
	struct llc *l;
	struct token_header *trh;
	int s, lan_hdr_len;

	if ((ifp->if_flags & IFF_UP) == 0) {
		m_freem(m);
		return;
	}

	trh = mtod(m, struct token_header *);

	ifp->if_ibytes += m->m_pkthdr.len;
	if (memcmp(tokenbroadcastaddr, trh->token_dhost,
	    sizeof(tokenbroadcastaddr)) == 0)
		m->m_flags |= M_BCAST;
	else if (trh->token_dhost[0] & 1)
		m->m_flags |= M_MCAST;
	if (m->m_flags & (M_BCAST|M_MCAST))
		ifp->if_imcasts++;

	/* Skip past the Token Ring header and RIF. */
	lan_hdr_len = sizeof(struct token_header);
	if (trh->token_shost[0] & TOKEN_RI_PRESENT) {
		struct token_rif *trrif;

		trrif = TOKEN_RIF(trh);
		lan_hdr_len += (ntohs(trrif->tr_rcf) & TOKEN_RCF_LEN_MASK) >> 8;
	}

	l = (struct llc *)(mtod(m, uint8_t *) + lan_hdr_len);

	switch (l->llc_dsap) {
#if defined(INET) || defined(NS) || defined(DECNET)
	case LLC_SNAP_LSAP:
	{
		uint16_t etype;
		if (l->llc_control != LLC_UI || l->llc_ssap != LLC_SNAP_LSAP)
			goto dropanyway;
		if (l->llc_snap.org_code[0] != 0 ||
		    l->llc_snap.org_code[1] != 0 ||
		    l->llc_snap.org_code[2] != 0)
			goto dropanyway;
		etype = ntohs(l->llc_snap.ether_type);
		m_adj(m, lan_hdr_len + LLC_SNAPFRAMELEN);
#if NCARP > 0
		if (ifp->if_carp && ifp->if_type != IFT_CARP &&
		    (carp_input(m, (uint8_t *)&trh->token_shost,
		    (uint8_t *)&trh->token_dhost, l->llc_snap.ether_type) == 0))
			return;
#endif /* NCARP > 0 */

		switch (etype) {
#ifdef INET
		case ETHERTYPE_IP:
			schednetisr(NETISR_IP);
			inq = &ipintrq;
			break;

		case ETHERTYPE_ARP:
			schednetisr(NETISR_ARP);
			inq = &arpintrq;
			break;
#endif
#ifdef DECNET
		case ETHERTYPE_DECNET:
			schednetisr(NETISR_DECNET);
			inq = &decnetintrq;
			break;
#endif
		default:
			/*
			printf("token_input: unknown protocol 0x%x\n", etype);
			*/
			ifp->if_noproto++;
			goto dropanyway;
		}
		break;
	}
#endif /* INET || NS || DECNET */
#ifdef	ISO
	case LLC_ISO_LSAP:
		switch (l->llc_control) {
		case LLC_UI:
			/* LLC_UI_P forbidden in class 1 service */
			if ((l->llc_dsap == LLC_ISO_LSAP) &&
			    (l->llc_ssap == LLC_ISO_LSAP)) {

#if defined(__FreeBSD__)
				IFDEBUG(D_ETHER)
					printf("clnp packet");
				ENDDEBUG
#endif
				schednetisr(NETISR_ISO);
				inq = &clnlintrq;
				break;
			}
			goto dropanyway;

		case LLC_XID:
		case LLC_XID_P:
			if(m->m_len < LLC_XID_BASIC_MINLEN + lan_hdr_len)
				goto dropanyway;
			l->llc_window = 0;
			l->llc_fid = LLC_XID_FORMAT_BASIC;
			l->llc_class = LLC_XID_CLASS_I;
			l->llc_dsap = l->llc_ssap = 0;
			/* Fall through to */
		case LLC_TEST:
		case LLC_TEST_P:
		{
			struct sockaddr sa;
			struct ether_header *eh;
			int i;
			u_char c = l->llc_dsap;

			l->llc_dsap = l->llc_ssap;
			l->llc_ssap = c;
			if (m->m_flags & (M_BCAST | M_MCAST))
				bcopy(CLLADDR(ifp->if_sadl),
				    (void *)trh->token_dhost,
				    ISO88025_ADDR_LEN);
			sa.sa_family = AF_UNSPEC;
			sa.sa_len = sizeof(sa);
			eh = (struct ether_header *)sa.sa_data;
			for (i = 0; i < ISO88025_ADDR_LEN; i++) {
				eh->ether_shost[i] = trh->token_dhost[i];
				eh->ether_dhost[i] = trh->token_shost[i];
			}
			eh->ether_type = 0;
			m_adj(m, lan_hdr_len);
			ifp->if_output(ifp, m, &sa, NULL);
			return;
		}
		default:
			m_freem(m);
			return;
		}
		break;
#endif /* ISO */

	default:
		/* printf("token_input: unknown dsap 0x%x\n", l->llc_dsap); */
		ifp->if_noproto++;
#if defined(INET) || defined(NS) || defined(DECNET) || defined(ISO)
	dropanyway:
#endif
		m_freem(m);
		return;
	}

	s = splnet();
	if (IF_QFULL(inq)) {
		IF_DROP(inq);
		m_freem(m);
	}
	else
		IF_ENQUEUE(inq, m);
	splx(s);
}

/*
 * Perform common duties while attaching to interface list
 */
void
token_ifattach(struct ifnet *ifp, void *lla)
{

	ifp->if_type = IFT_ISO88025;
	ifp->if_hdrlen = 14;
	ifp->if_dlt = DLT_IEEE802;
	ifp->if_mtu = ISO88025_MTU;
	ifp->if_output = token_output;
	ifp->if_input = token_input;
	ifp->if_broadcastaddr = tokenbroadcastaddr;
#ifdef IFF_NOTRAILERS
	ifp->if_flags |= IFF_NOTRAILERS;
#endif

	if_set_sadl(ifp, lla, ISO88025_ADDR_LEN, true);

	bpf_attach(ifp, DLT_IEEE802, sizeof(struct token_header));
}

void
token_ifdetach(struct ifnet *ifp)
{

	bpf_detach(ifp);
#if 0	/* done in if_detach() */
	if_free_sadl(ifp);
#endif
}
