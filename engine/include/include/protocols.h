/*
 * trace variable and init function declarations
 *
 * Warning: this file is generated from graph.comp and is overwritten
 * every time 'make compose' is run
 */

#include <upi.h>

extern int traceethdrvp;
extern int traceethp;
extern int tracearpp;
extern int tracevnetp;
extern int traceipp;
extern int traceicmpp;
extern int tracetcpp;
extern int traceudpp;
extern int tracexksocketp;
extern int traceudptestp;
extern void ethdrv_init( XObj );
extern void eth_init( XObj );
extern void arp_init( XObj );
extern void vnet_init( XObj );
extern void ip_init( XObj );
extern void icmp_init( XObj );
extern void tcp_init( XObj );
extern void udp_init( XObj );
extern void xksocket_init( XObj );
extern void udptest_init( XObj );
