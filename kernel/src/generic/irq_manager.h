#ifndef __IRQ_MANAGER_H__
#define __IRQ_MANAGER_H__

#include INC_GLUE(idt.h)
#include INC_GLUE(intctrl.h)

class irq_manager_t {
public:
    void register_irq(word_t hwirq, word_t prio, void (*handler)())
    {
        /* priority currently unused */
        idt.add_gate(0x20 + hwirq, idt_t::interrupt, handler);
    }

    void unmask(word_t hwirq)
    {
        get_interrupt_ctrl()->unmask(hwirq);
    }
};

extern irq_manager_t irq_manager;

#endif /* __IRQ_MANAGER_H__ */
