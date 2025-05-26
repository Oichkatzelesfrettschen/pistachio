/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
 * Copyright (c) 1992 Helsinki University of Technology
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND HELSINKI UNIVERSITY OF TECHNOLOGY ALLOW FREE USE
 * OF THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * HELSINKI UNIVERSITY OF TECHNOLOGY DISCLAIM ANY LIABILITY OF ANY KIND
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
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
 * 07-Dec-92  Ian Dall (ian) at University of Adelaide
 *	Make IGN_CARR macro use a mask "ign_carr_mask" so that the lines
 *	to be used can be changed from kdb. This is still a croc!
 *
 * 09-Aug-92  Johannes Helander (jvh) at Helsinki University of Technology
 *	Added carrier detect code [Ian Dall (idall@eleceng.adelaide.edu.au)].
 *
 * 11-May-92  Tatu Ylonen (ylo) at Helsinki University of Technology
 *	Created.
 *
 * $Log: com.c,v $
 *
 * Compressed history:
 * 1992/04/08  jvh
 * 	Experiment with using boot_putc in cnputc.
 *
 * 1991/10/24 jvh
 * 	Removed cominit call from comopen. Instead called from
 * 	machine_init(). Actually initialized by first printf (very
 * 	early).
 *
 * 91/03/08 ylo
 * 	Fixed comparam looping too long
 * 	Fixed comparam clobbering the last output character
 * 	Fixed the problem with ioctl changing modes without allowing 
 * 	output to drain.
 * 91/03/06 ylo
 * 	Don't wait for output to drain in comparam, it didn't work anyway.
 * 	Use sploff in cnputc, as otherwise the interrupt routine will turn it
 * 	off again.
 * 
 * 91/03/06 ylo
 * 	Removed spl calls from cnputc, as cnputc is called way too early
 * 	to use spl's.
 * 	Enabled transmitter in cnputc
 * 	Changed comstart to enable/disable transmitter instead of messing up
 * 	with imr.
 * 
 * 91/02/21 ylo
 * 	Fixed problems with tty modes not being set; also tried to make 
 * 	Comparam wait until the current character has been sent before 
 * 	changing modes
 * 
 * 91/02/09 kivinen
 * 	Added resetting errors in comintr_receive
 * 	Moved break checking to comintr_receive
 */
/*
 * 	File: ns532/PC532/com.c
 *	Author: Tatu Ylonen, Helsinki University of Technology 1992.
 *
 *   	com driver for pc532 (for SCN2692 DUARTS)
 */
/* 
 *   This could be further optimized by using the "timout mode" (interrupt on
 *   fifo full, use C/T to interrupt if no more characters coming).  This would
 *   reduce the number of interrupts by about a factor of four.  (But would
 *   also put more stringent requirements on interrupt latency.)
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <ns532/pic.h>
#include <device/tty.h>
#include <ns532/pmap.h>
#include <ns532/thread.h>
#include <ns532/machparam.h>
#include <device/errno.h>
#include <device/io_req.h>
#include <kern/assert.h>
#include "com.h"
#include "mach_kdb.h"

#define VERSION		"0.2"

#define CONSOLEPORT	0		/* number of default console tty */

#define DUART_BASE	0x28000000	/* base address for duarts */
#define DUART_SIZE	8		/* size of duart in address space */

#define DUART_MR	0      	/* relative to port base */
#define DUART_STAT	1
#define DUART_CSR	1
#define DUART_CR	2
#define DUART_DATA	3

#define DUART_ACR	4   	/* relative to duart base */
#define DUART_IPCR	4
#define DUART_IMR	5
#define DUART_ISR	5
#define DUART_OPCR	13

#define MR1_ODD		0x04
#define MR1_EVEN	0x00
#define MR1_7BITS	0x02
#define MR1_8BITS	0x03
#define MR1_WITHPARITY	0x00
#define MR1_NOPARITY	0x10
#define MR1_RXRTS	0x80

#define CR_BASE		0x00
#define CR_RESETPTR	(0x10|CR_BASE)
#define CR_RESETRX	(0x20|CR_BASE)
#define CR_RESETTX	(0x30|CR_BASE)
#define CR_RESETERR	(0x40|CR_BASE)
#define CR_RESETBRK	(0x50|CR_BASE)
#define CR_DISABLETX	0x08
#define CR_ENABLETX	0x04
#define CR_DISABLERX	0x02
#define CR_ENABLERX	0x01
#define CR_DISABLE	(CR_DISABLETX|CR_DISABLERX)
#define CR_ENABLE	(CR_ENABLETX|CR_ENABLERX)

#define IMR_BASE	0x91
#define IMR_TXRDYA	0x01
#define IMR_TXRDYB	0x10

#define ISR_INPUTCHG	0x80  /* input port change */
#define ISR_BBRK	0x40  /* ch b change in break */
#define ISR_BRXRDYFULL	0x20  /* ch b rx rdy or full */
#define ISR_BTXRDY	0x10  /* ch b tx rdy */
#define ISR_CNTRDY	0x08  /* counter ready */
#define ISR_ABRK	0x04  /* ch a change in break */
#define ISR_ARXRYFULL	0x02  /* ch a rx rdy or full */
#define ISR_ATXRDY	0x01  /* ch a tx rdy */

#define IPCR_IP2	0x04
#define IPCR_IP3	0x08
#define IPCR_DIP2	0x40
#define IPCR_DIP3	0x80

#define RX_RDY		0x01		/* mask for rxrdy in STAT */
#define FFULL		0x02		/* mask for fifo full in STAT */
#define TX_RDY		0x04		/* mask for txrdy in STAT */
#define TX_EMPTY	0x08		/* transmitter empty */
#define RBREAK		0x80		/* mask for received break in STAT */
#define FRAMING_ERROR	0x40		/* framing error */
#define PARITY_ERROR	0x20		/* parity error */
#define OVERRUN_ERROR	0x10		/* overrun error */

int comstart(),comstop(),comparam(),comgetstat();

extern int ttrstrt();
extern char *io_map();

int console_tty=CONSOLEPORT;  /* number of console tty */

char *combase=NULL;	/* com port virtual address */
int compolling=0;	/* force polling mode */
int compolledchar = -1; /* last character received in polled mode */
struct tty comttys[NCOM];

#define COM_DEFAULT_FLAGS	(TF_ANYP|TF_NOHANG|TF_ECHO|TF_CRMOD)


#if 1
/* Some units (without modems connected) we don't want to look
 * for carrier detect on.
 * Also, the UX server is broken so that at the moment, we don't
 * want to look for carrier on any lines lest it hang the server.
 */
int ign_carr_mask = 0xffffffff;
#define IGN_CARR(unit) (ign_carr_mask & (1 << (unit)))
#else
/*
 * From Ian Dall 92/08/07:
 * I have added code to a) interprete the carrier detect properly and
 * b) ignore carrier on ports which the IGN_CARR macro says to. It is
 * configured to ignore carrier on the console but not other ports.
 * This whole feature is turnable off by writing 0 the the global
 * variable ign_carr. This is a crock and the driver needs lots of
 * work. I have no plans to do more on it in the immediate future.
 */
int ign_carr = 1;
#define IGN_CARR(unit) (ign_carr? (unit) == CONSOLEPORT: 0)
#endif

/* This initializes the com driver. */

cominit()
{
  int unit;

  if(combase != NULL)
      return;

  combase=io_map(DUART_BASE,NCOM*DUART_SIZE);
  for (unit=0;unit < NCOM;unit++)
    {
      comttys[unit].t_addr = combase + unit*DUART_SIZE;
      comttys[unit].t_ispeed=comttys[unit].t_ospeed=B9600;
      comttys[unit].t_flags=COM_DEFAULT_FLAGS;
      comttys[unit].t_dev=unit;
      comttys[unit].t_state=0;
    }
  for (unit=0;unit < NCOM;unit+=2)
    {
      comttys[unit].t_addr[DUART_CR]=CR_DISABLE;
      comttys[unit].t_addr[DUART_IMR]=IMR_BASE;
      comttys[unit].t_addr[DUART_OPCR]=0x30;
      comttys[unit].t_addr[DUART_ACR]=0xbc;
    }
  for (unit=0;unit < NCOM;unit++)
    {
      comparam(unit);
    }
  printf("PC532 COM Driver Version %s\n", VERSION);
}

comintr(irq,old_ipl,thstate)
int irq;
int old_ipl;
struct ns532_saved_state *thstate;
{
  switch (irq)
    {
    case IR_DUAR3: /* receive interrupt on duart 3 */
      comintr_receive(&comttys[7],thstate);
      comintr_receive(&comttys[6],thstate);
      break;
    case IR_DUA3: /* other interrupt on duart 3 */
      comintr_other(&comttys[6],&comttys[7]);
      break;
    case IR_DUAR2:
      comintr_receive(&comttys[5],thstate);
      comintr_receive(&comttys[4],thstate);
      break;
    case IR_DUA2:
      comintr_other(&comttys[4],&comttys[5]);
      break;
    case IR_DUAR1:
      comintr_receive(&comttys[3],thstate);
      comintr_receive(&comttys[2],thstate);
      break;
    case IR_DUA1:
      comintr_other(&comttys[2],&comttys[3]);
      break;
    case IR_DUAR0:
      comintr_receive(&comttys[1],thstate);
      comintr_receive(&comttys[0],thstate);
      break;
    case IR_DUA0:
      comintr_other(&comttys[0],&comttys[1]);
      break;
    default:
      printf("comintr called for non-uart device %d\n",irq);
      break;
    }
}

comintr_receive(tp,thstate)
struct tty *tp;
struct ns532_thread_state *thstate;
{
  char ch,status;
  int goterror;
  
  while ((status=tp->t_addr[DUART_STAT]) & RX_RDY)
    {
      goterror=0;
      if (status & RBREAK)
	{
	  goterror=1;
	  ch=tp->t_addr[DUART_DATA];  /* remove the "break character" */
	  printf("com%d: received break\n",minor(tp->t_dev),ch);
#if	MACH_KDB
	  if (minor(tp->t_dev) == console_tty)
	    {
	      kdb_kbd_trap(thstate);
	      continue;
	    }
#endif	MACH_KDB
	  if (tp->t_state & TS_ISOPEN)
	    (*linesw[tp->t_line].l_rint)(tp->t_breakc,tp);
	  continue;
	}
      /* XXXXXXX ----- how do we pass errors to higher levels???? */
      /* how about ior->io_error? //jvh */
      if (status & FRAMING_ERROR)
	{
	  goterror=1;
	  printf("com%d: framing error\n",minor(tp->t_dev));
	}
      if (status & PARITY_ERROR)
	{
	  goterror=1;
	  printf("com%d: parity error\n",minor(tp->t_dev));
	}
      if (status & OVERRUN_ERROR)
	{
	  goterror=1;
	  printf("com%d: fifo overflow\n",minor(tp->t_dev));
	}
      if (goterror)
	tp->t_addr[DUART_CR]=CR_RESETERR;
      ch=tp->t_addr[DUART_DATA];
      if (compolling && minor(tp->t_dev) == console_tty)
	compolledchar=ch;
      else
	{
	  if (tp->t_state & TS_ISOPEN)
	    {
	      (*linesw[tp->t_line].l_rint)(ch,tp);
	    }
	}
      break;
    }
}

comintr_other(tpa,tpb)
struct tty *tpa,*tpb;
{
  unsigned char isr,status,ipcr;

  isr=tpa->t_addr[DUART_ISR];
  if (isr & ISR_ATXRDY)
    {
      tpa->t_state &= ~TS_BUSY;
      comdostart(tpa);
    }
  if (isr & ISR_BTXRDY)
    {
      tpb->t_state &= ~TS_BUSY;
      comdostart(tpb);
    }
  if (isr & ISR_INPUTCHG)
    {
      ipcr=tpa->t_addr[DUART_IPCR];
      if ((tpa->t_state & TS_ISOPEN) && (ipcr & IPCR_DIP3))
	{
	  printf("com%d: modem status change: ipcr=0x%x\n",
		 minor(tpa->t_dev),ipcr);
	  (*linesw[tpa->t_line].l_modem)(tpa, IGN_CARR(tpa->t_dev) || (ipcr & IPCR_IP3) == 0); 
	     /* port A DCD */
	}
      if ((tpb->t_state & TS_ISOPEN) && (ipcr & IPCR_DIP2))
	{
	  printf("com%d: modem status change: ipcr=0x%x\n",
		 minor(tpb->t_dev),ipcr);
	  (*linesw[tpb->t_line].l_modem)(tpb,IGN_CARR(tpb->t_dev) || (ipcr & IPCR_IP2) == 0); 
	     /* port B DCD */
	}
    }
}

#ifdef	MACH_KERNEL
int comopen(dev,flag,ior)
dev_t dev;
int flag;
io_req_t ior;
#else	MACH_KERNEL
int comopen(dev,flag)
int dev,flag;
#endif	MACH_KERNEL
{
  int unit=minor(dev);
  int oldpri;
  struct tty *tp;

  assert(combase != NULL);

  if (unit >= NCOM)
    return ENXIO;
  tp = &comttys[unit];

  if (!(tp->t_state & (TS_ISOPEN|TS_WOPEN)))
    {
      ttychars(tp);
      tp->t_dev = dev;
      tp->t_oproc = comstart;
#ifdef	MACH_KERNEL
      tp->t_stop=comstop;
      tp->t_getstat=comgetstat;
#endif	MACH_KERNEL
      if (tp->t_ispeed == 0)
	{
	  tp->t_ispeed = B9600;
	  tp->t_ospeed = B9600;
	  tp->t_flags = COM_DEFAULT_FLAGS;
	  tp->t_state &= ~TS_BUSY;
	}
      comparam(unit);
    }
  printf("com%d: modem status: ipcr=0x%x\n",
	 minor(tp->t_dev),comttys[unit&~1].t_addr[DUART_IPCR]);
  if (unit & 1)
    (*linesw[tp->t_line].l_modem)(tp, IGN_CARR(unit)
				  || (comttys[unit&~1].t_addr[DUART_IPCR] & 
				      IPCR_IP2) == 0);
  else
    (*linesw[tp->t_line].l_modem)(tp, IGN_CARR(unit)
				  || (tp->t_addr[DUART_IPCR] & IPCR_IP3) == 0);
#ifdef	MACH_KERNEL
  return char_open(dev,tp,flag,ior);
#else	MACH_KERNEL
  return (*linesw[tp->t_line].l_open)(dev,tp);
#endif	MACH_KERNEL
}

comclose(dev,flag)
int dev,flag;
{
  struct tty *tp = &comttys[minor(dev)];

#ifndef	MACH_KERNEL
  (*linesw[tp->t_line].l_close)(tp);
#endif
  ttyclose(tp);
  if (tp->t_state & TS_HUPCLS && (tp->t_state & TS_ISOPEN) == 0)
    {
      tp->t_ispeed = 0;
      comhangup(minor(dev));
    }
  return 0;
}

int comread(dev,uio)
int dev;
struct uio *uio;
{
  struct tty *tp = &comttys[minor(dev)];

  return ((*linesw[tp->t_line].l_read)(tp,uio));
}

int comwrite(dev,uio)
int dev;
struct uio *uio;
{
  struct tty *tp = &comttys[minor(dev)];

  return ((*linesw[tp->t_line].l_write)(tp,uio));
}

#ifdef	MACH_KERNEL
comportdeath(dev,port)
dev_t dev;
mach_port_t port;
{
  struct tty *tp = &comttys[minor(dev)];
  
  return tty_portdeath(tp,port);
}

io_return_t comgetstat(dev,flavor,data,count)
dev_t dev;
int flavor;
int *data;
unsigned int *count;
{
  struct tty *tp = &comttys[minor(dev)];
  
  return tty_get_status(tp,flavor,data,count);
}

io_return_t comsetstat(dev,flavor,data,count)
dev_t dev;
int flavor;
int *data;
unsigned int *count;
{
  struct tty *tp = &comttys[minor(dev)];
  int oldispeed,oldospeed,oldflags,retval,x;

  oldispeed=tp->t_ispeed;
  oldospeed=tp->t_ospeed;
  oldflags=tp->t_flags;
  retval=tty_set_status(tp,flavor,data,count);
  if (flavor == TTY_STATUS &&
      (oldispeed != tp->t_ispeed ||
       oldospeed != tp->t_ospeed ||
       oldflags != tp->t_flags))
    {
      x=spltty();
      while (tp->t_outq.c_cc > 0)
	{
	  tp->t_state |= TS_ASLEEP;
	  sleep(&tp->t_outq,0);
	}
      comparam(minor(dev));
      splx(x);
      comstart(tp);
    }
  return retval;
}

#else	MACH_KERNEL

int comioctl(dev,cmd,addr,mode)
int dev;
int cmd;
caddr_t addr;
int mode;
{
  int error;
  int unit = minor(dev);
  struct tty *tp = &comttys[unit];
 
  printf("comioctl: dev=0x%x cmd=0x%x addr=0x%x mode=0x%x\n",
	 dev,cmd,addr,mode);
  error = (*linesw[tp->t_line].l_ioctl)(tp,cmd,addr,mode);
  if (error >= 0)
    return(error);
  error = ttioctl(tp,cmd,addr,mode);
  if (error >= 0)
    {
      if (cmd == TIOCSETP || cmd == TIOCSETN || cmd == TIOCLBIS ||
	  cmd == TIOCLBIC || cmd == TIOCLSET)
	{
	  comparam(unit);
	  comstart(tp);
	}
      return (error);
    }
  switch (cmd)
    {
    case TIOCSBRK:
      /* outb(LINE_CTL(dev_addr), inb(LINE_CTL(dev_addr)) | iSETBREAK); */
      break;
    case TIOCCBRK:
      /* outb(LINE_CTL(dev_addr), inb(LINE_CTL(dev_addr)) & ~iSETBREAK); */
      break;
    case TIOCSDTR:
      /* outb(MODEM_CTL(dev_addr), iOUT2|iDTR|iRTS); */
      break;
    case TIOCCDTR:
      /* outb(MODEM_CTL(dev_addr), iOUT2|iRTS); */
      break;
    case TIOCMSET:
    case TIOCMBIS:
    case TIOCMBIC:
    case TIOCMGET:
      uprintf("modem control not yet implemented\n");
      /* fall to next case */
    default:
      return ENOTTY;
    }
  return ENOTTY;
}

#endif	MACH_KERNEL

comparam(unit)
register int unit;
{
  struct tty *tp = &comttys[unit];
  static short baud_table[16]=
    {
      0,0,0,1,
      2,3,0,4,
      5,6,10,8,
      9,11,12,0
    };
  char mr1,baud;
  char *port=tp->t_addr;
  int ms,l;

  /* if (tp->t_ispeed == 0)
       hang up the line;
     else ...
  */

  baud = (baud_table[tp->t_ispeed] << 4) | baud_table[tp->t_ospeed];
  switch (tp->t_flags & (TF_ODDP | TF_EVENP))
    {
    case TF_ODDP:
      mr1=MR1_ODD|MR1_7BITS|MR1_WITHPARITY;
      break;
    case TF_EVENP:
      mr1=MR1_EVEN|MR1_7BITS|MR1_WITHPARITY;
      break;
    default:
      mr1=MR1_8BITS|MR1_NOPARITY;
      break;
    }

  port[DUART_CR]=CR_ENABLETX;  /* needed to make TX_EMPTY valid */
  for (l=0;l < 100000;l++)
    if (port[DUART_STAT] & TX_EMPTY)
      break;
  port[DUART_CR]=CR_DISABLE;
  port[DUART_CSR]=baud;
  port[DUART_CR]=CR_RESETPTR;
  port[DUART_MR]=mr1;  /* should we add MR1_RXRTS? */
  port[DUART_MR]=0x07;
  port[DUART_CR]=CR_RESETRX;
  port[DUART_CR]=CR_RESETTX;
  port[DUART_CR]=CR_RESETERR;
  port[DUART_CR]=CR_RESETBRK;
  port[DUART_CR]=CR_ENABLE;
}

comdostart(tp)
struct tty *tp;
{
  char nch=0;
  int oldpri,ch,unit;

  unit=minor(tp->t_dev);
  oldpri=spltty();
  if ((tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_FLUSH|TS_ISOPEN)) != TS_ISOPEN)
    goto done;
  if (tp->t_outq.c_cc <= TTLOWAT(tp))
    {
#ifdef	MACH_KERNEL
      tt_write_wakeup(tp);
#else	MACH_KERNEL
      if (tp->t_state & TS_ASLEEP)
	{
	  tp->t_state &= ~TS_ASLEEP;
	  wakeup ((caddr_t)&tp->t_outq);
	}
      if (tp->t_wsel)
	{
	  selwakeup(tp->t_wsel, tp->t_state&TS_WCOLL);
	  tp->t_wsel = 0;
	  tp->t_state &= ~TS_WCOLL;
	}
#endif	MACH_KERNEL
    }
  if (tp->t_outq.c_cc == 0)
    goto done;
  if (tp->t_flags & (RAW|LITOUT))
    {
      nch = ndqb(&tp->t_outq,0);
    }
  else
    {
      nch = ndqb(&tp->t_outq, 0200);
      if (nch == 0)
	{
	  nch = getc(&tp->t_outq);
	  /* printf("comdostart: sleeping for %d\n",(nch&0x7f)+6); */
	  timeout(ttrstrt,(caddr_t)tp,(nch&0x7f)+6);
	  tp->t_state |= TS_TIMEOUT;
	  nch=0;
	  goto done;
	}
    }
  if (nch != 0)
    {
      ch=getc(&tp->t_outq);
      if (ch != -1)
	{
	  tp->t_addr[DUART_CR]=CR_ENABLETX;
	  tp->t_addr[DUART_DATA]=ch;
	  tp->t_state |= TS_BUSY;
	}
    }
 done:
  if (!(tp->t_state & TS_BUSY))
    {
      tp->t_addr[DUART_CR]=CR_DISABLETX;
      if (tp->t_state & TS_ASLEEP)
	{
	  tp->t_state &= ~TS_ASLEEP;
	  wakeup(&tp->t_outq);
	}
    }
  splx(oldpri);
}

comstart(tp)
struct tty *tp;
{
  tp->t_state &= ~TS_FLUSH;
  comdostart(tp);
}

comhangup(tp)
struct tty *tp;
{
  tp->t_ispeed=0;
  comparam(tp);
}

#ifdef	MACH_KERNEL
comstop(tp, flag)
struct tty *tp;
int flag;
{
  if ((tp->t_state & TS_BUSY) && (tp->t_state & TS_TTSTOP) == 0)
    tp->t_state |= TS_FLUSH;
}
#endif	MACH_KERNEL

/* This outputs a single character in polled mode. */
cnputc(ch)
char ch;
{
  int x;

  if (combase == NULL) {
	  boot_putc(ch);
	  return;
  }
  if (ch == '\n')
    cnputc('\r');

  x = splhi();
  comttys[console_tty].t_addr[DUART_CR]=CR_ENABLETX;
  while (!(comttys[console_tty].t_addr[DUART_STAT] & TX_EMPTY))
    ;
  comttys[console_tty].t_addr[DUART_DATA]=ch;
  while (!(comttys[console_tty].t_addr[DUART_STAT] & TX_EMPTY))
    ;
  splx(x);
}

/* This should switch between polled and interrupt-driven mode??? */

cnpollc(on)
int on;
{
  if (on)
    compolling++;
  else
    compolling--;
  assert(compolling >= 0);
}

int cngetc()
{
  return cndogetc(TRUE);
}

int cnmaygetc()
{
  return cndogetc(FALSE);
}

int cndogetc(wait)
int wait;
{
  int ch;
  
  while (!(comttys[console_tty].t_addr[DUART_STAT] & RX_RDY) &&
	 compolledchar == -1)
    {
      if (!wait)
	return -1;
    }
  if (compolledchar != -1)
    {
      ch=compolledchar;
      compolledchar = -1;
      return ch;
    }
  return (unsigned char)comttys[console_tty].t_addr[DUART_DATA];
}
