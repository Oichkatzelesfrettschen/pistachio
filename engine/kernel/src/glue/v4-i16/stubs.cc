#include <debug.h>
#include INC_GLUE(config.h)
#include INC_ARCH(types.h)

/**
 * Switch to a new stack and instruction pointer in real mode.
 *
 * The calling convention expects the target SP in AX and the
 * jump address in DX.  Only used by the tiny i16 port for tests.
 */
extern "C" void context_switch(word_t sp, void (*ip)())
{
    asm volatile(
        "mov %0, %%sp\n\t"
        "jmp *%1\n\t"
        :
        : "r"(sp), "r"(ip)
    );
}

/**
 * Minimal interrupt wrapper that immediately returns with IRET.
 */
extern "C" void interrupt_entry()
{
    asm volatile("iret");
}
