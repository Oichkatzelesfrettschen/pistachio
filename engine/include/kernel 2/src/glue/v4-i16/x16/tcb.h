/*********************************************************************
 * tcb stubs for i16
 *********************************************************************/
#ifndef __GLUE_V4_I16__X16__TCB_H__
#define __GLUE_V4_I16__X16__TCB_H__

class tcb_t {
public:
    static void initial_switch_to(tcb_t*) __attribute__((noreturn));
    void switch_to(tcb_t*);
};

inline void tcb_t::initial_switch_to(tcb_t*) { while (1); }
inline void tcb_t::switch_to(tcb_t*) {}

#endif /* !__GLUE_V4_I16__X16__TCB_H__ */
