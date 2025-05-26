/* 
 * $RCSfile: xmed_i.h,v $
 *
 * x-kernel v3.2
 *
 * Copyright (c) 1991  Arizona Board of Regents
 *
 *
 * $Revision: 1.4 $
 * $Date: 1993/02/18 22:19:26 $
 */

#ifndef xmed_i_h
#define xmed_i_h

#include <sys/types.h>

/*
 * Medusa Board Control and Status Registers.
 */
typedef struct {
    u_int reset_misc;
    u_int reset_fifo;
    u_int reset_fddi;
    u_int test_fcg;
    u_int test_elm;
    u_int test_mac;
    u_int interrupt_mask;
    u_int interrupt_event;
    u_int reserved;
    u_int misc_output;
    u_int status_synch_fifo;
    u_int status_buff_fifo;
    u_int misc_input;
} med_reg_set_t;

/*
 * Elasticity Buffer and Link Management (ELM) Interface.
 * (MC68837 chip)
 */
typedef struct {
    u_int cntrl_a;
    u_int cntrl_b;
    u_int intr_mask;
    u_int xmit_vector;
    u_int vector_length;
    u_int le_threshold;
    u_int a_max;
    u_int ls_max;
    u_int tb_min;
    u_int t_out;
    u_int reserved;
    u_int lc_short;
    u_int t_scrub;
    u_int ns_max;
    u_int tpc_load_value;
    u_int tne_load_value;
    u_int status_a;
    u_int status_b;
    u_int tpc;
    u_int tne;
    u_int clk_div;
    u_int bist_signature;
    u_int rcv_vector;
    u_int intr_event;
    u_int viol_syn_ctr;
    u_int min_idle_ctr;
    u_int link_err_ctr;
} elm_reg_set_t;

/*
 * Media Access Controller (MAC) Interface.
 * (MC68838 chip)
 */
typedef struct {
    u_int cntrl_a;
    u_int cntrl_b;
    u_int intr_mask_a;
    u_int intr_mask_b;
    u_int reserved1[12];
    u_int msa;
    u_int mla_a;
    u_int mla_b;
    u_int mla_c;
    u_int t_req;
    union {
	u_int tvx_value;
	u_int t_max;
    } u1;
    u_int reserved2[10];
    u_int frame_ct;
    union {
	u_int lost_ct;
	u_int error_ct;
    } u2;
    u_int intr_event_a;
    u_int intr_event_b;
    u_int rx_status;
    u_int tx_status;
    u_int t_neg_a;
    u_int t_neg_b;
    u_int info_reg_a;
    u_int info_reg_b;
    u_int bist_signature;
    u_int tvx_timer;
    u_int trt_timer_a;
    u_int trt_timer_b;
    u_int tht_timer_a;
    union {
	u_int tht_timer_b;
	u_int sentcount;
    } u3;
    u_int pkt_request;
    u_int rc_crc_a;
    u_int rc_crc_b;
    u_int tx_crc_a;
    u_int tx_crc_b;
} mac_reg_set_t;

/*
 * Medusa Memory Map:
 */
#define SPACE	u_int space[0x400000 / sizeof(int)]

typedef volatile struct xmed_board {
    u_int sti_rom[0x0800000 / sizeof(int)];
    union {
	elm_reg_set_t elm_reg;
	SPACE;
    } s1;
    union {
	mac_reg_set_t mac_reg;
	SPACE;
    } s2;
    union {
	med_reg_set_t med_reg;
	SPACE;
    } s3;
    union {
	u_int chk_sum_reg;
	SPACE;
    } s4;
    u_int reserved1[0x0400000 / sizeof(int)];
    u_int reserved2[0x0400000 / sizeof(int)];
    union {
	u_int *tx_ready_reg;
	SPACE;
    } s5;
    union {
	u_int *tx_free_reg;
	SPACE;
    } s6;
    union {
	u_int *rx_ready_reg;
	SPACE;
    } s7;
    union {
	u_int *rx_free_reg;
	SPACE;
    } s8;
    u_int vram_std[0x800000 / sizeof(int)];
    u_int vram_chk[0x800000 / sizeof(int)];
} xmed_board_t;

#undef SPACE

/* make accessing the unions more convenient: */

#define elm		s1.elm_reg
#define mac		s2.mac_reg
#define med		s3.med_reg
#define chk_sum		s4.chk_sum_reg
#define tx_ready	s5.tx_ready_reg
#define tx_free		s6.tx_free_reg
#define rx_ready	s7.rx_ready_reg
#define rx_free		s8.rx_free_reg

/*
 * Bitmask definitions for the Interrupt Event Register
 */
#define IER_TX_READY_NON_FULL	0x80
#define IER_TX_FREE_NON_EMPTY	0x40
#define IER_RX_READY_NON_EMPTY	0x20
#define IER_RX_FREE_NON_FULL	0x10
#define IER_ELM_INTERRUPT	0x08
#define IER_MAC_INTERRUPT	0x04

/*
 * Bitmask definitions for the Interrupt Mask Register
 */
#define IMR_TX_READY_NON_FULL	0x80
#define IMR_TX_FREE_NON_EMPTY	0x40
#define IMR_RX_READY_NON_EMPTY	0x20
#define IMR_RX_FREE_NON_FULL	0x10
#define IMR_ELM_INTERRUPT	0x08
#define IMR_MAC_INTERRUPT	0x04

/*
 * Bitmask definitions for the FIFO Reset Register
 */
#define RST_TX_READY	0x80
#define RST_TX_FREE	0x40
#define RST_RX_READY	0x20
#define RST_RX_FREE	0x10
#define RST_TX_SYN	0x08
#define RST_RX_SYN	0x04

/*
 * Bitmask definitions for the Misc RESET register
 */
#define RST_FDDI_CONTROLLER	0x80
#define RST_SERIAL_CONTROLLER	0x40

/*
 * Bitmask definitions for the Misc Output register
 */
#define SERIALRX_TEST	0x80

/*
 * Bitmask definitions for the buffer FIFO status register 
 */
#define BSR_TX_READY_NON_FULL	0x80
#define BSR_TX_READY_NON_EMPTY	0x40
#define BSR_TX_FREE_NON_FULL	0x20
#define BSR_TX_FREE_NON_EMPTY	0x10
#define BSR_RX_READY_NON_FULL	0x08
#define BSR_RX_READY_NON_EMPTY	0x04
#define BSR_RX_FREE_NON_FULL	0x02
#define BSR_RX_FREE_NON_EMPTY	0x01

/*
 * Bitmask definitions for the FIFO queues:
 */
#define FIFO_NON_EMPTY		0x80000000
#define FIFO_LENGTH_MASK	0x00001fff

/*
 * Configuration parameters
 */
#define MAC_MSA_CONFIG		0x0000
#define MAC_T_REQ_CONFIG	0xff3c
#define MAC_T_MAX_CONFIG	0x80e0
#define MAC_CNTRL_A_CONFIG	0x8000
#define MAC_CNTRL_B_CONFIG	0x0000

/*
 * TX Status masks
 */
#define MAC_RING_OPERATIONAL	0x0800

/*
 * MAC_CNTRL_A definitions
 */
#define MAC_ON			0x8000
#define MAC_SETBIT5		0x4000
#define MAC_SETBIT4		0x2000
#define MAC_REVERSE_ADDR	0x1000
#define MAC_FLUSH_SA47		0x0800
#define MAC_COPYALL		0x0600
#define MAC_COPYOWN		0x0100
#define MAC_COPY_EXTRA_SMT	0x00C0
#define MAC_COPY_IND_LLC	0x0020
#define MAC_COPY_GROUP_LLC	0x0010
#define MAC_DISABLE_BRDCST	0x0008
#define MAC_RUN_BIST		0x0004
#define MAC_RX_PARITY		0x0002
#define MAC_NOTE_ALL_FRAMES	0x0001

/*
 * MAC_CNTRL_B defines
 */
#define MAC_RING_PURGE		0x8000
#define MAC_FDX_MODE		0x4000
#define MAC_BRIDGE_STRIP	0x2000
#define MAC_TX_PARITY_ON	0x1000
#define MAC_REPEAT_ONLY		0x0800
#define MAC_LOSE_CLAIM		0x0400
#define MAC_RESET_FIELD		0x0300
#define MAC_FSI_BEACON		0x0080
#define MAC_DELAY_TOKEN		0x0040
#define MAC_IGNORE_SACAM	0x0020

/*
 * MAC_RX_STATUS defines
 */
#define MAC_RC_FSM_STATE	0xe000
#define MAC_R_FLAG		0x1000
#define MAC_E_FLAG		0x0800
#define MAC_FSM_STATE		0x0700
#define MAC_N_FLAG		0x0080
#define MAC_FR_PARS_STATE	0x0070
#define MAC_L_FLAG		0x0008
#define MAC_H_FLAG		0x0004
#define MAC_M_FLAG		0x0002
#define MAC_A_FLAG		0x0001

/*
 * MAC TX_STATUS defines
 */
#define MAC_TX_FSM_STATE	0xf000
#define MAC_RING_OP		0x0800
#define MAC_PURGING		0x0400
#define MAC_FLD_SEQ_STATE	0x03c0
#define MAC_FLD_CNT_STATE	0x0038
#define MAC_LATE_CT		0x0007

/*
 * Defines for ELM_CNTRL_A
 */
#define ELM_NOISE_TIMER		0x4000
#define ELM_TNE_16BIT		0x2000
#define ELM_TPC_16BIT		0x1000
#define ELM_REQ_SCRUB		0x0800
#define ELM_VSYM_CTR_INTRS	0x0200
#define ELM_MINI_CTR_INTRS	0x0100
#define ELM_FCG_LOOP_CNTRL	0x0080
#define ELM_FOT_OFF		0x0040
#define ELM_EB_LOC_LOOP		0x0020
#define ELM_LOC_LOOP		0x0010
#define ELM_SC_BYPASS		0x0008
#define ELM_SC_REM_LOOP		0x0004
#define ELM_RF_DISABLE		0x0002
#define ELM_RUN_BIST		0x0001

/*
 * Defines for ELM_CNTRL_B
 */
#define ELM_CONFG_CNTRL		0x8000
#define ELM_MATCH_LS		0x7800
#define ELM_MAINT_LS		0x0700
#define ELM_CLASS_S		0x0080
#define ELM_PC_LOOP		0x0060
#define ELM_PC_JOIN		0x0010
#define ELM_LONG		0x0008
#define ELM_PC_MAINT		0x0004
#define ELM_PCM_CNTRL		0x0003

/*
 * Defines for ELM_STATUS_A
 */
#define ELM_SIGNAL_DETECT	0x0400
#define ELM_PREV_LINE_ST	0x0300
#define ELM_LINE_ST		0x00e0
#define ELM_LSM_STATE		0x0010
#define ELM_UNKN_LINE_ST	0x0008
#define ELM_SYM_PR_CTR		0x0007

/*
 * Defines for ELM_STATUS_B
 */
#define ELM_RF_STATE		0xc000
#define ELM_PCI_STATE		0x3000
#define ELM_PCI_SCRUB		0x0800
#define ELM_PCM_STATE		0x0780
#define ELM_PCM_SIGNALLING	0x0040
#define ELM_LSF			0x0020
#define ELM_RCF			0x0010
#define ELM_TCF			0x0008
#define ELM_BREAK_REASON	0x0007

/*
 * Defines for ELM interrupt EVENT
 */
#define ELM_NP_ERR		0x8000
#define ELM_LSD			0x4000
#define ELM_LE_CTR		0x2000
#define ELM_MINI_CTR		0x1000
#define ELM_VSYM_CTR		0x0800
#define ELM_PHYINV		0x0400
#define ELM_EBUF_ERR		0x0200
#define ELM_TNE_EXPIRED		0x0100
#define ELM_TPC_EXPIRED		0x0080
#define ELM_PCM_ENABLED		0x0040
#define ELM_PCM_BROKE		0x0020
#define ELM_SELF_TEST		0x0010
#define ELM_TRACE_PROP		0x0008
#define ELM_PCM_CODE		0x0004
#define ELM_LS_MATCH		0x0002
#define ELM_PARITY_ERR		0x0001

/*
 * Defines for PCM Control
 */
#define ELM_PCM_CNTRL_NOP	0x0000
#define ELM_PCM_CNTRL_BREAK	0x0001
#define ELM_PCM_CNTRL_TRACE	0x0002
#define ELM_PCM_CNTRL_STOP	0x0003

/*
 * PCM States
 */
#define ELM_PCM_OFF		0x0000
#define ELM_PCM_BREAK		0x0080
#define ELM_PCM_TRACE		0x0100
#define ELM_PCM_CONNECT		0x0180
#define ELM_PCM_NEXT		0x0200
#define ELM_PCM_SIGNAL		0x0280
#define ELM_PCM_JOIN		0x0300
#define ELM_PCM_VERIFY		0x0380
#define ELM_PCM_ACTIVE		0x0400
#define ELM_PCM_MAINT		0x0480

/*
 * Configuration parameters for the ELM registers,
 * taken from the ELM data sheet, page 19.
 */
#define ELM_INTR_MASK_CONFIG	0x0000
#define ELM_LE_THRESHOLD_CONFIG 0x0001
#define ELM_A_MAX_CONFIG	0xfff6
#define ELM_LS_MAX_CONFIG	0xffff
#define ELM_TB_MIN_CONFIG	0xfc2f
#define ELM_T_OUT_CONFIG	0xeced
#define ELM_LC_SHORT_CONFIG	0xf676
#define ELM_T_SCRUB_CONFIG	0xff6d
#define ELM_NS_MAX_CONFIG	0xe796
#define ELM_CNTRL_A_CONFIG	0x0000

/*
 * PC_LOOP codes for ELM LCT
 */
#define ELM_PC_LOOP_NONE        0x0000
#define ELM_PC_LOOP_MAC         0x0020
#define ELM_PC_LOOP_IDLE        0x0040
#define ELM_PC_LOOP_REMOTE      0x0060

/*
 * Station type codes
 */
#define FDDI_STATION_A		0x00
#define FDDI_STATION_B		0x04
#define FDDI_STATION_S		0x02
#define FDDI_STATION_M		0x06

/*
 * Lengths of LCT tests
 */
#define FDDI_LCT_SHORT		0x00
#define FDDI_LCT_MEDIUM		0x02
#define FDDI_LCT_LONG		0x01
#define FDDI_LCT_EXTENDED	0x03

/*
 * MAC for LCT codes
 */
#define FDDI_MAC_LCT_NO		0x00
#define FDDI_MAC_LCT_YES	0x01

/*
 * Initial Board Register Values
 */
#define INIT_RESET_MISC		0xc0
#define INIT_RESET_FIFO		0xfc
#define INIT_RESET_FDDI		0xc0
#define INIT_TEST_FCG		0xc0
#define INIT_TEST_ELM		0x00
#define INIT_TEST_MAC		0x00
#define INIT_INTERRUPT_MASK	0x00
#define INIT_INTERRUPT_EVENT	0x00
#define INIT_MISC_OUTPUT	0xc0
#define INIT_MISC_INPUT		0xe0
#define INIT_STATUS_SYNCH_FIFO	0xa0
#define INIT_STATUS_BUFF_FIFO	0xaa
#define INIT_CHECKSUM_UNIT	0x00

#define XMED_INDIVIDUAL_ADDRESS(address)		\
  (((u_char)(address)[0] & 0x01) == 0)

#define XMED_BROADCAST_ADDRESS(a)			\
  ( ((a)[0] == 0xffffffff) &&				\
    (((a)[1] & 0xffff0000) == 0xffff0000) )

#define XMED_SAME_ADDRESS(a1,a2)			\
  ( ((((a1)[1] ^ (a2)[1]) & 0xffff0000) == 0) &&	\
    ((a1)[0] == (a2)[0]) )

#define XMED_STI_ID		0x280b31af	/* Medusa ROM graphics ID */
#define XMED_NET_ADDR_OFFSET	0x3ffe3		/* ROM offset of net addr */
#define XMED_NET_ADDR_LEN	6		/* length of net address  */

#define XMED_BUF_SIZE	8192	/* size of each Medusa buffer in bytes */

#endif /* xmed_i_h */
