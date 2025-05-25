#include <stdio.h>
#include "xmed_i.h"

/*
 * Print offset of registers to check whether header file
 * definitions match actual hardware:
 */
int
main()
{
    xmed_board_t *xd;
#   define OFF(r)	((char*)&xd->r - (char*)xd)
#   define R(r)		printf(#r"=%x\n", OFF(r))

    /* Medusa Miscellaneous Stuff: */
    R(chk_sum);
    R(tx_ready);
    R(tx_free);
    R(rx_ready);
    R(rx_free);
    R(vram_std);
    R(vram_chk);

    /* Medusa registers: */
    R(med.reset_misc);
    R(med.reset_fifo);
    R(med.reset_fddi);
    R(med.test_fcg);
    R(med.test_elm);
    R(med.test_mac);
    R(med.interrupt_mask);
    R(med.interrupt_event);
    R(med.misc_input);
    R(med.status_synch_fifo);
    R(med.status_buff_fifo);
    R(med.misc_output);

    /* ELM registers: */
    R(elm.cntrl_a);
    R(elm.cntrl_b);
    R(elm.intr_mask);
    R(elm.xmit_vector);
    R(elm.vector_length);
    R(elm.le_threshold);
    R(elm.a_max);
    R(elm.ls_max);
    R(elm.tb_min);
    R(elm.t_out);
    R(elm.lc_short);
    R(elm.t_scrub);
    R(elm.ns_max);
    R(elm.tpc_load_value);
    R(elm.tne_load_value);
    R(elm.status_a);
    R(elm.status_b);
    R(elm.tpc);
    R(elm.tne);
    R(elm.clk_div);
    R(elm.bist_signature);
    R(elm.rcv_vector);
    R(elm.intr_event);
    R(elm.viol_syn_ctr);
    R(elm.min_idle_ctr);
    R(elm.link_err_ctr);
    
    /* MAC registers: */
    R(mac.cntrl_a);
    R(mac.cntrl_b);
    R(mac.intr_mask_a);
    R(mac.intr_mask_b);
    R(mac.msa);
    R(mac.mla_a);
    R(mac.mla_b);
    R(mac.mla_c);
    R(mac.t_req);
    R(mac.u1.tvx_value);
    R(mac.u1.t_max);
    R(mac.frame_ct);
    R(mac.u2.lost_ct);
    R(mac.u2.error_ct);
    R(mac.intr_event_a);
    R(mac.intr_event_b);
    R(mac.rx_status);
    R(mac.tx_status);
    R(mac.t_neg_a);
    R(mac.t_neg_b);
    R(mac.info_reg_a);
    R(mac.info_reg_b);
    R(mac.bist_signature);
    R(mac.tvx_timer);
    R(mac.trt_timer_a);
    R(mac.trt_timer_b);
    R(mac.tht_timer_a);
    R(mac.u3.tht_timer_b);
    R(mac.u3.sentcount);
    R(mac.pkt_request);
    R(mac.rc_crc_a);
    R(mac.rc_crc_b);
    R(mac.tx_crc_a);
    R(mac.tx_crc_b);

    return 0;
} /* main */
