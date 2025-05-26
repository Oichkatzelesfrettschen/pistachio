/*
 * protocol initialization calls
 *
 * Warning: this file is generated from graph.comp and is overwritten
 * every time 'make compose' is run
 */

#include "upi.h"
#include "x_util.h"
#include "compose.h"
#include "protocols.h"


XObj		protl_tab[11+1];
static XObj	argv[11];


void
build_pgraph_dev()
{
}


void
build_pgraph()
{

  /*
   *  building protocol SIMETH
   */
  protl_tab[0] = xCreateProtl(simeth_init, "simeth", "",
#ifdef XK_DEBUG
			&tracesimethp,
#else
			0,
#endif
				0, argv);
  if ( protl_tab[0] == ERR_XOBJ ) {
      Kabort("Could not create simeth protocol");
  }


  /*
   *  building protocol ETH
   */
  argv[0] = protl_tab[0];
  protl_tab[1] = xCreateProtl(eth_init, "eth", "",
#ifdef XK_DEBUG
			&traceethp,
#else
			0,
#endif
				1, argv);
  if ( protl_tab[1] == ERR_XOBJ ) {
      Kabort("Could not create eth protocol");
  }


  /*
   *  building protocol ARP
   */
  argv[0] = protl_tab[1];
  protl_tab[2] = xCreateProtl(arp_init, "arp", "",
#ifdef XK_DEBUG
			&tracearpp,
#else
			0,
#endif
				1, argv);
  if ( protl_tab[2] == ERR_XOBJ ) {
      Kabort("Could not create arp protocol");
  }


  /*
   *  building protocol VNET
   */
  argv[0] = protl_tab[1];
  argv[1] = protl_tab[2];
  protl_tab[3] = xCreateProtl(vnet_init, "vnet", "",
#ifdef XK_DEBUG
			&tracevnetp,
#else
			0,
#endif
				2, argv);
  if ( protl_tab[3] == ERR_XOBJ ) {
      Kabort("Could not create vnet protocol");
  }


  /*
   *  building protocol IP
   */
  argv[0] = protl_tab[3];
  protl_tab[4] = xCreateProtl(ip_init, "ip", "",
#ifdef XK_DEBUG
			&traceipp,
#else
			0,
#endif
				1, argv);
  if ( protl_tab[4] == ERR_XOBJ ) {
      Kabort("Could not create ip protocol");
  }


  /*
   *  building protocol ICMP
   */
  argv[0] = protl_tab[4];
  protl_tab[5] = xCreateProtl(icmp_init, "icmp", "",
#ifdef XK_DEBUG
			&traceicmpp,
#else
			0,
#endif
				1, argv);
  if ( protl_tab[5] == ERR_XOBJ ) {
      Kabort("Could not create icmp protocol");
  }


  /*
   *  building protocol UDP
   */
  argv[0] = protl_tab[4];
  protl_tab[6] = xCreateProtl(udp_init, "udp", "",
#ifdef XK_DEBUG
			&traceudpp,
#else
			0,
#endif
				1, argv);
  if ( protl_tab[6] == ERR_XOBJ ) {
      Kabort("Could not create udp protocol");
  }


  /*
   *  building protocol BLAST
   */
  argv[0] = protl_tab[4];
  protl_tab[7] = xCreateProtl(blast_init, "blast", "ip",
#ifdef XK_DEBUG
			&traceblastp,
#else
			0,
#endif
				1, argv);
  if ( protl_tab[7] == ERR_XOBJ ) {
      Kabort("Could not create blast protocol");
  }


  /*
   *  building protocol BLAST
   */
  argv[0] = protl_tab[3];
  protl_tab[8] = xCreateProtl(blast_init, "blast", "vnet",
#ifdef XK_DEBUG
			&traceblastp,
#else
			0,
#endif
				1, argv);
  if ( protl_tab[8] == ERR_XOBJ ) {
      Kabort("Could not create blast protocol");
  }


  /*
   *  building protocol VMUX
   */
  argv[0] = protl_tab[8];
  argv[1] = protl_tab[7];
  protl_tab[9] = xCreateProtl(vmux_init, "vmux", "",
#ifdef XK_DEBUG
			&tracevmuxp,
#else
			0,
#endif
				2, argv);
  if ( protl_tab[9] == ERR_XOBJ ) {
      Kabort("Could not create vmux protocol");
  }


  /*
   *  building protocol CHAN
   */
  argv[0] = protl_tab[9];
  protl_tab[10] = xCreateProtl(chan_init, "chan", "",
#ifdef XK_DEBUG
			&tracechanp,
#else
			0,
#endif
				1, argv);
  if ( protl_tab[10] == ERR_XOBJ ) {
      Kabort("Could not create chan protocol");
  }

}
