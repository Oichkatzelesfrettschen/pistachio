/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	ether_io.c,v $
 * Revision 2.1  92/04/21  17:10:44  rwd
 * BSDSS
 * 
 *
 */

#include <map_ether.h>
#include <ns.h>
#include <inet.h>

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/zalloc.h>
#include <sys/synch.h>
#include <sys/proc.h>

#include <sys/time.h>

#include <net/if.h>
#include <net/netisr.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>



#if NS
#include <netns/ns.h>
#include <netns/ns_if.h>
#endif


#include <uxkern/device_reply_hdlr.h>
#include <uxkern/device_utils.h>

#include <uxkern/import_mach.h>
#include <device/device_types.h>
#include <device/net_status.h>
#include <sys/user.h>
#include <sys/kernel.h>

int debug_netcode = 0;
/*
 * Interface descriptor
 *	NOTE: most of the code outside will believe this to be simply
 *	a "struct ifnet".  That is ok, since it is exactly what ethernets
 *	do in Unix anyway.  The other information is, on the other hand
 *	our own business.
 */
struct ether_softc {
	struct	arpcom es_ac;	/* same as ethernets */
#define es_if	es_ac.ac_if	/* a struct ifnet used outside */
#define es_addr	es_ac.ac_enaddr	/* hardware address, 6 bytes ok for ethers */
#define es_ip	es_ac.ac_ipaddr	/* copy of IP address */
	mach_port_t es_port;	/* where to send requests */
	mach_port_t es_reply_port;	/* where to receive data */
	char	es_name[IFNAMSIZ];
				/* name lives here */
	struct ether_softc *es_link;
				/* link in list of reply ports */
};

struct ether_softc *es_list = 0;
				/* list of all ethernet interfaces,
				   to find by reply port. */

/*
 * Ethernet trigger routine.
 */
ether_start(ifp)
	struct ifnet *ifp;
{
	int type, s, error;
	u_char edst[6];
	struct in_addr idst;
	register struct ether_softc *es = (struct ether_softc *)ifp;
	register struct mbuf *m;
	register struct ether_header *eh;
	int usetrailers;

	IF_DEQUEUE(&ifp->if_snd, m);
	if (m == 0)
		return (0);

	/*
	 * Send message to interface
	 */
	{
	    unsigned int  totlen;
	    char *data_addr;
	    register struct mbuf *m1;

	    totlen = 0;
	    for (m1 = m;
		 m1;
		 m1 = m1->m_next)
		totlen += m1->m_len;

	    if (m->m_next == 0) {
		/*
		 * All data in one chunk
		 */
		data_addr = mtod(m, char *);
xxx("ether_start:", data_addr, totlen);
		(void) device_write_request(es->es_port, MACH_PORT_NULL,
					    0,	/* mode */
					    0,	/* recnum */
					    data_addr,
					    totlen);
	    }
	    else {
		/*
		 * Must allocate contiguous memory and
		 * copy mbuf string into it.
		 */
		(void) vm_allocate(mach_task_self(), (vm_offset_t *)&data_addr,
				   totlen, TRUE);
if(debug_netcode) printf("ether_start: copy\n");
		(void) m_copydata(m, 0, totlen, data_addr);
		(void) device_write_request(es->es_port, MACH_PORT_NULL,
					    0,	/* mode */
					    0,	/* recnum */
					    data_addr,
					    totlen);
		(void) vm_deallocate(mach_task_self(), data_addr, totlen);
	    }
	}
	m_freem(m);	/* sent */
	return (0);

    bad:
	m_freem(m);
	return (error);
}

int net_input_dispose();	/* forward */
extern struct mbuf *mclgetx();
zone_t	net_msg_zone;

extern mach_port_t device_server_port;

mach_port_t	net_reply_port_set;

void
net_input_thread()
{
	register struct ether_softc *es;
	register struct ifnet *ifp;
	struct mbuf *m;
	int	type, len;
	char *	addr;
	struct ether_header *eh;
	register net_rcv_msg_t msg = (net_rcv_msg_t)0;
	struct timeval time;
	struct proc *p;

	system_proc(&p, "NetworkInput");

	/*
	 * Wire this cthread to a kernel thread so we can
	 * up its priority
	 */
	cthread_wire();

	/*
	 * Make this thread high priority.
	 */
	set_thread_priority(mach_thread_self(), 3);

	p->p_ipl = -1;

	while (TRUE) {
	    if (p->p_ipl != -1)
		panic("net ipl != -1",p->p_ipl);

	    /*
	     * Allocate a message structure, and receive the next
	     * network message.
	     */
	    msg = (net_rcv_msg_t)zalloc(net_msg_zone);


	    if (mach_msg(&msg->msg_hdr, MACH_RCV_MSG,
			 0, 8192, net_reply_port_set,
		 MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL)
						!= MACH_MSG_SUCCESS)
		goto error;
if(debug_netcode) printf("netisr got pkt\n");

	    /*
	     * Find the interface that the message was received on.
	     */
	    for (es = es_list; es; es = es->es_link)
		if (msg->msg_hdr.msgh_local_port == es->es_reply_port)
		    break;
	    if (es == 0)
		goto error;

	    ifp = &es->es_if;
/*	    get_time(&time);
	    ifp->if_lastchange.tv_sec = time.tv_sec;	
	    ifp->if_lastchange.tv_usec = time.tv_usec;	
*/
	    if ((ifp->if_flags & IFF_UP) == 0)
		goto error;

	    /*
	     * Save the ethernet header.  Do not include it
	     * in the data passed to upper levels.
	     */
	    eh = (struct ether_header *)&msg->header[0];
if(debug_netcode) xxx("header:", eh, sizeof(*eh));
	    /*
	     * Get the packet address and length.
	     */
	    type = ((struct packet_header *)&msg->packet[0])->type;
	    addr = &msg->packet[0] + sizeof(struct packet_header);
	    len  = msg->packet_type.msgt_number
			 - sizeof(struct packet_header);
if(debug_netcode) {
	printf("len %d,", len);
	xxx("data:", addr, len);
    }

	    if (len == 0)
		goto error;

	    /*
	     * We need this
	     */

	    interrupt_enter(SPLIMP);

	    /*
	     * Wrap an mbuf around the data, excluding the
	     * local header.
	     */
	    m = mclgetx(net_input_dispose,
			(caddr_t)msg,
			addr,
			len,
			M_WAIT);
	    /*
	     * Prepend the interface pointer.  We know that
	     * we will get enough room by clobbering the
	     * ethernet header.
	     */
	    if(m == (struct mbuf *)NULL)
			panic("net_input_thread - no mbufs");
	    m->m_pkthdr.len = len;
	    m->m_flags |= M_PKTHDR;
	    m->m_pkthdr.rcvif = ifp;


	    /*
	     * Dispatch to protocol.
	     */
	    len = ntohs(eh->ether_type);
	    eh->ether_type = len;
/* XXX trailers XXX */
	    ether_input(ifp, eh, m);

	    interrupt_exit(SPLIMP);
	    
	    continue;
    error:
	    zfree(net_msg_zone, (vm_offset_t)msg);
	    continue;

	}
}

xxx(s,p,i)
	unsigned char *p;
{
	if(!debug_netcode) return;
	printf("%s", s);
	while (i--)
		printf(" %x", *p++);
	printf("\r\n");
}


net_input_dispose(msg)
	char *	msg;
{
    zfree(net_msg_zone, (vm_offset_t)msg);
}

/*ARGSUSED*/
kern_return_t
ether_write_reply(ifp_p, return_code, count)
	char *		ifp_p;
	kern_return_t	return_code;
	unsigned int	count;
{
}

/*
 * All interface setup is thru IOCTL.
 */
ether_ioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int	cmd;
	caddr_t	data;
{
	register struct ifaddr *ifa = (struct ifaddr *)data;
	register struct ether_softc *es = (struct ether_softc *)ifp;
	int s = splimp();
	int error = 0;

	switch (cmd) {
	    /*
	     * Set interface address (and mark interface up).
	     */
	    case SIOCSIFADDR:
	    case SIOCAIFADDR:
		switch (ifa->ifa_addr->sa_family) {
#if INET
		    case AF_INET:
			es->es_ip = IA_SIN(ifa)->sin_addr;
			arpwhohas(&es->es_ac, &IA_SIN(ifa)->sin_addr);
			break;
#endif
#if NS
		    case AF_NS:
		    {
			register struct ns_addr *ina = &(IA_SNS(ifa)->sns_addr);
			if (ns_nullhost(*ina))
			    ina->x_host = *(union ns_host *)es->es_addr;
			else {
			    bcopy(ina->x_host.c_host,
				  es->es_addr,
				  sizeof(es->es_addr));
			    /*
			     * Tell hardware to change address XXX
			     */
			}
			break;
		    }
#endif
		}
		break;

	    default:
		error = EINVAL;
		break;
	}
	splx(s);
	return (error);
}

ether_error()
{
}

/*
 * This function strategically plugs into ifunit(), and it is called
 * on a non-existant interface.  We try to look it up, and if successful
 * initialize a descriptor and call if_attach() with it.
 *
 * Name points to the full device name (including unit number).
 */
struct ifnet *
iface_find(name)
	char		*name;
{
	register struct ifnet	*ifp;
	register struct ether_softc *es;
	mach_port_t		if_port;
	mach_port_t		reply_port;
	struct net_status	if_stat;
	unsigned int		if_stat_count;
	char			if_addr[16];	/* XXX */
	unsigned int		if_addr_count;
	kern_return_t		rc;
	int			unit;

	rc = device_open(device_server_port,
			 0,		/* mode */
			 name,
			 &if_port);
	if (rc != D_SUCCESS)
	    return (0);
	/*
	 * Get status and address from interface.
	 */
	if_stat_count = NET_STATUS_COUNT;
	rc = device_get_status(if_port,
			NET_STATUS,
			(dev_status_t)&if_stat,
			&if_stat_count);
	if (rc != D_SUCCESS) {
	    (void) device_close(if_port);
	    (void) mach_port_deallocate(mach_task_self(), if_port);
	    return (0);
	}

#if	MAP_ETHER
	if (if_stat.mapped_size) {
		extern	struct ifnet *ether_map();	/* machdep */
		return ether_map(name, if_port, &if_stat);
	}
#endif	MAP_ETHER

	if_addr_count = sizeof(if_addr)/sizeof(int);
	rc = device_get_status(if_port,
			NET_ADDRESS,
			(dev_status_t)if_addr,
			&if_addr_count);
	if (rc != D_SUCCESS) {
	    (void) device_close(if_port);
	    (void) mach_port_deallocate(mach_task_self(), if_port);
	    return (0);
	}

	/*
	 * Byte-swap address into host format.
	 */
	{
	    register int 	i;
	    register int	*ip;

	    for (i = 0, ip = (int *)if_addr;
		 i < if_addr_count;
		 i++,ip++) {
		*ip = ntohl(*ip);
	    }
	}
	/*
	 * Allocate a receive port for the interface.
	 */
	reply_port = mach_reply_port();
	if (reply_port == MACH_PORT_NULL) {
	    printf("Failed to allocate reply port.\n");
	    (void) device_close(if_port);
	    (void) mach_port_deallocate(mach_task_self(), if_port);
	    return (0);
	}
	rc = mach_port_move_member(mach_task_self(),
				   reply_port, net_reply_port_set);
	if (rc != KERN_SUCCESS) {
  printf("Adding reply port to net_reply_port_set returns %d\n", rc);
	    (void) mach_port_mod_refs(mach_task_self(), reply_port,
				      MACH_PORT_RIGHT_RECEIVE, -1);
	    (void) device_close(if_port);
	    (void) mach_port_deallocate(mach_task_self(), if_port);
	    return (0);
	}

	/*
	 * Allocate an Ethernet interface structure.
	 */
	es = (struct ether_softc *)malloc(sizeof(struct ether_softc));
	bzero((caddr_t) es, sizeof(struct ether_softc));

	ifp = &es->es_if;

	/*
	 * Save interface name in a safe place.  Caller
	 * has ensured that it is shorter than IFNAMSIZ
	 * and ends in a digit.
	 */
	{
	    register char *dst, *src;
	    register char c;

	    dst = es->es_name;
	    for (src = name; ; src++) {
		c = *src;
		if (c >= '0' && c <= '9')
		    break;
		*dst++ = c;
	    }
	    unit = c - '0';
	}
	ifp->if_name =		es->es_name;
	ifp->if_unit =		unit;
	ifp->if_mtu =		if_stat.max_packet_size
					- if_stat.header_size;
	ifp->if_flags =		if_stat.flags;
	ifp->if_output =	ether_output;
	ifp->if_start =		ether_start;
	ifp->if_ioctl =		ether_ioctl;

	bcopy(if_addr, es->es_addr, sizeof(es->es_addr));

	es->es_port = if_port;
	es->es_reply_port = reply_port;

	/*
	 * Set up a filter to receive packets.  Take all,
	 * for the moment.
	 */
	{
	    filter_t	filter[2];

	    filter[0] = NETF_PUSHLIT;
	    filter[1] = (filter_t)TRUE;

	    rc = device_set_filter(if_port,
				   reply_port,
				   MACH_MSG_TYPE_MAKE_SEND,
				   1,		/* priority */
				   filter,
				   sizeof(filter)/sizeof(filter[0]));
	    if (rc != KERN_SUCCESS)
		printf("device_set_filter: %d\n", rc);
	}

	/*
	 * Attach ether_softc structure to reply_port list.
	 */
	es->es_link = es_list;
	es_list = es;

	if_attach(ifp);
	ifinit();    

	return (struct ifnet *)ifp;
}



void
netisr_init()
{
    kern_return_t	rc;
    net_msg_zone = zinit(8192,
			 1000*8192,
			 10*8192,
			 TRUE,
			 "incoming network messages");
    
    rc = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_PORT_SET,
			    &net_reply_port_set);
    if (rc != KERN_SUCCESS)
	panic("Allocating net reply port set returns %d", rc);
    
    ux_create_thread(net_input_thread);

}

