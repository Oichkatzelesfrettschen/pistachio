# C++17 Modernisation Plan

This document tracks the ongoing effort to convert every source and header in the repository to modern C++17.  Each refactored file must begin with the tag:

```cpp
// 2025 refactored to C++17
```

This short comment makes it easy to locate all modernised code and identify what still needs work.

## Refactoring Steps

For each file processed:

1. Rename any `.c` source to `.cpp` and `.h` header to `.hpp`.
2. Update all includes to use the new extensions.
3. Replace C constructs with C++17 idioms and remove obsolete keywords such as `register`.
4. Use `constexpr` or `inline` functions instead of macros and prefer `std::` types over manual implementations.
5. Comment the code thoroughly and run `clang-format` to keep the style consistent.
6. After a batch of changes run `pre-commit`, `pytest -q`, and `ctest --output-on-failure`.

## Task Breakdown

The following tasks process roughly 25 files each.  Continue creating additional tasks until all sources are converted.

### Task 1
- engine/apps/lib/rmgr/librmgr.c
- engine/apps/lib/l4malloc/libl4malloc.c
- engine/apps/lib/l4/libl4.c
- engine/apps/lib/ide/libide.c
- engine/apps/lib/io/arm-brutus-putc.c
- engine/apps/lib/io/arm-pleb-getc.c
- engine/apps/lib/io/print.c
- engine/apps/lib/io/x86-i586-putc.c
- engine/apps/lib/io/arm-pleb-putc.c
- engine/apps/lib/io/arm-dnard-getc.c
- engine/apps/lib/io/arm-brutus-getc.c
- engine/apps/lib/io/get_hex.c
- engine/apps/lib/io/arm-dnard-uart.h
- engine/apps/lib/io/arm-ep7211-uart.h
- engine/apps/lib/io/pleb-uart.h
- engine/apps/lib/io/arm-ep7211-getc.c
- engine/apps/lib/io/brutus-uart.h
- engine/apps/lib/io/arm-ep7211-putc.c
- engine/apps/lib/io/x86-i586-getc.c
- engine/apps/lib/io/mini-print.c
- engine/apps/lib/io/arm-dnard-putc.c
- engine/apps/serv/ide-driver/hd.h
- engine/apps/serv/ide-driver/req.c
- engine/apps/serv/ide-driver/ide.c
- engine/apps/serv/ide-driver/main.c

### Task 2
- engine/apps/arm-booter/elf.h
- engine/apps/arm-booter/elf.c
- engine/apps/arm-booter/main.c
- engine/apps/rmgr/include/l4/rmgr/rmgr.h
- engine/apps/rmgr/include/l4/rmgr/librmgr.h
- engine/apps/rmgr/include/l4/rmgr/server_proto.h
- engine/apps/rmgr/include/l4/l4.h
- engine/apps/rmgr/include/l4/x86-x0-32/kernel.h
- engine/apps/rmgr/include/l4/x86-x0-32/idt.h
- engine/apps/rmgr/include/l4/x86-x0-32/types.h
- engine/apps/rmgr/include/l4/x86-x0-32/kdebug.h
- engine/apps/rmgr/include/l4/x86-x0-32/compiler.h
- engine/apps/rmgr/include/l4/x86-x0-32/ipc.h
- engine/apps/rmgr/include/l4/x86-x0-32/syscalls.h
- engine/apps/rmgr/include/l4/x86-x0-32/linkage.h
- engine/apps/rmgr/include/l4/compiler.h
- engine/apps/rmgr/include/l4/linkage.h
- engine/apps/rmgr/include/flux/exec/elf.h
- engine/apps/rmgr/include/flux/exec/exec.h
- engine/apps/rmgr/include/flux/exec/a.out.h
- engine/apps/rmgr/include/flux/fdev/net.h
- engine/apps/rmgr/include/flux/fdev/bus.h
- engine/apps/rmgr/include/flux/fdev/char.h
- engine/apps/rmgr/include/flux/fdev/fdev.h
- engine/apps/rmgr/include/flux/fdev/serial.h

### Task 3
- engine/apps/rmgr/include/flux/fdev/blk.h
- engine/apps/rmgr/include/flux/fdev/error.h
- engine/apps/rmgr/include/flux/page.h
- engine/apps/rmgr/include/flux/tty.h
- engine/apps/rmgr/include/flux/inline.h
- engine/apps/rmgr/include/flux/c/signal.h
- engine/apps/rmgr/include/flux/c/sys/wait.h
- engine/apps/rmgr/include/flux/c/sys/signal.h
- engine/apps/rmgr/include/flux/c/sys/ioctl.h
- engine/apps/rmgr/include/flux/c/sys/reboot.h
- engine/apps/rmgr/include/flux/c/sys/time.h
- engine/apps/rmgr/include/flux/c/sys/types.h
- engine/apps/rmgr/include/flux/c/sys/stat.h
- engine/apps/rmgr/include/flux/c/sys/gmon.h
- engine/apps/rmgr/include/flux/c/sys/uio.h
- engine/apps/rmgr/include/flux/c/sys/termios.h
- engine/apps/rmgr/include/flux/c/sys/mman.h
- engine/apps/rmgr/include/flux/c/common.h
- engine/apps/rmgr/include/flux/c/assert.h
- engine/apps/rmgr/include/flux/c/setjmp.h
- engine/apps/rmgr/include/flux/c/string.h
- engine/apps/rmgr/include/flux/c/time.h
- engine/apps/rmgr/include/flux/c/unistd.h
- engine/apps/rmgr/include/flux/c/ctype.h
- engine/apps/rmgr/include/flux/c/alloca.h

### Task 4
- engine/apps/rmgr/include/flux/c/stdarg.h
- engine/apps/rmgr/include/flux/c/malloc.h
- engine/apps/rmgr/include/flux/c/errno.h
- engine/apps/rmgr/include/flux/c/limits.h
- engine/apps/rmgr/include/flux/c/stddef.h
- engine/apps/rmgr/include/flux/c/stdlib.h
- engine/apps/rmgr/include/flux/c/fcntl.h
- engine/apps/rmgr/include/flux/c/endian.h
- engine/apps/rmgr/include/flux/c/termios.h
- engine/apps/rmgr/include/flux/c/a.out.h
- engine/apps/rmgr/include/flux/c/stdio.h
- engine/apps/rmgr/include/flux/c/memory.h
- engine/apps/rmgr/include/flux/c/strings.h
- engine/apps/rmgr/include/flux/config.h
- engine/apps/rmgr/include/flux/debug.h
- engine/apps/rmgr/include/flux/machine/exec/elf.h
- engine/apps/rmgr/include/flux/machine/dos/vcpi.h
- engine/apps/rmgr/include/flux/machine/dos/dpmi.h
- engine/apps/rmgr/include/flux/machine/base_tss.h
- engine/apps/rmgr/include/flux/machine/page.h
- engine/apps/rmgr/include/flux/machine/pio.h
- engine/apps/rmgr/include/flux/machine/tss.h
- engine/apps/rmgr/include/flux/machine/pmode.h
- engine/apps/rmgr/include/flux/machine/far_ptr.h
- engine/apps/rmgr/include/flux/machine/pc/base_irq.h

### Task 5
- engine/apps/rmgr/include/flux/machine/pc/reset.h
- engine/apps/rmgr/include/flux/machine/pc/direct_cons.h
- engine/apps/rmgr/include/flux/machine/pc/rtc.h
- engine/apps/rmgr/include/flux/machine/pc/serial_gdb.h
- engine/apps/rmgr/include/flux/machine/pc/debug.h
- engine/apps/rmgr/include/flux/machine/pc/keyboard.h
- engine/apps/rmgr/include/flux/machine/pc/base_multiboot.h
- engine/apps/rmgr/include/flux/machine/pc/pic.h
- engine/apps/rmgr/include/flux/machine/pc/fdev.h
- engine/apps/rmgr/include/flux/machine/pc/irq_list.h
- engine/apps/rmgr/include/flux/machine/pc/com_cons.h
- engine/apps/rmgr/include/flux/machine/pc/phys_lmm.h
- engine/apps/rmgr/include/flux/machine/pc/pit.h
- engine/apps/rmgr/include/flux/machine/c/common.h
- engine/apps/rmgr/include/flux/machine/c/setjmp.h
- engine/apps/rmgr/include/flux/machine/c/stdarg.h
- engine/apps/rmgr/include/flux/machine/seg.h
- engine/apps/rmgr/include/flux/machine/asm.h
- engine/apps/rmgr/include/flux/machine/base_idt.h
- engine/apps/rmgr/include/flux/machine/types.h
- engine/apps/rmgr/include/flux/machine/fp_reg.h
- engine/apps/rmgr/include/flux/machine/debug.h
- engine/apps/rmgr/include/flux/machine/proc_reg.h
- engine/apps/rmgr/include/flux/machine/base_trap.h
- engine/apps/rmgr/include/flux/machine/cpuid.h

### Task 6
- engine/apps/rmgr/include/flux/machine/paging.h
- engine/apps/rmgr/include/flux/machine/fdev.h
- engine/apps/rmgr/include/flux/machine/multiboot.h
- engine/apps/rmgr/include/flux/machine/base_paging.h
- engine/apps/rmgr/include/flux/machine/base_cpu.h
- engine/apps/rmgr/include/flux/machine/anno.h
- engine/apps/rmgr/include/flux/machine/debug_reg.h
- engine/apps/rmgr/include/flux/machine/spin_lock.h
- engine/apps/rmgr/include/flux/machine/eflags.h
- engine/apps/rmgr/include/flux/machine/base_stack.h
- engine/apps/rmgr/include/flux/machine/trap.h
- engine/apps/rmgr/include/flux/machine/base_gdt.h
- engine/apps/rmgr/include/flux/machine/base_vm.h
- engine/apps/rmgr/include/flux/machine/gdb.h
- engine/apps/rmgr/include/flux/machine/gate_init.h
- engine/apps/rmgr/include/flux/machine/smp/smp.h
- engine/apps/rmgr/include/flux/machine/smp/asm-smp.h
- engine/apps/rmgr/include/flux/machine/smp/linux-smp.h
- engine/apps/rmgr/include/flux/machine/smp/bitops.h
- engine/apps/rmgr/include/flux/machine/smp/i82489.h
- engine/apps/rmgr/include/flux/gdb_serial.h
- engine/apps/rmgr/include/flux/fdev.h
- engine/apps/rmgr/include/flux/lmm.h
- engine/apps/rmgr/include/flux/anno.h
- engine/apps/rmgr/include/flux/diskpart/dec.h

### Task 7
- engine/apps/rmgr/include/flux/diskpart/omron.h
- engine/apps/rmgr/include/flux/diskpart/pcbios.h
- engine/apps/rmgr/include/flux/diskpart/disklabel.h
- engine/apps/rmgr/include/flux/diskpart/diskpart.h
- engine/apps/rmgr/include/flux/diskpart/vtoc.h
- engine/apps/rmgr/include/flux/memdebug.h
- engine/apps/rmgr/include/flux/boolean.h
- engine/apps/rmgr/include/flux/x86/exec/elf.h
- engine/apps/rmgr/include/flux/x86/dos/vcpi.h
- engine/apps/rmgr/include/flux/x86/dos/dpmi.h
- engine/apps/rmgr/include/flux/x86/base_tss.h
- engine/apps/rmgr/include/flux/x86/page.h
- engine/apps/rmgr/include/flux/x86/pio.h
- engine/apps/rmgr/include/flux/x86/tss.h
- engine/apps/rmgr/include/flux/x86/pmode.h
- engine/apps/rmgr/include/flux/x86/far_ptr.h
- engine/apps/rmgr/include/flux/x86/pc/base_irq.h
- engine/apps/rmgr/include/flux/x86/pc/reset.h
- engine/apps/rmgr/include/flux/x86/pc/direct_cons.h
- engine/apps/rmgr/include/flux/x86/pc/rtc.h
- engine/apps/rmgr/include/flux/x86/pc/serial_gdb.h
- engine/apps/rmgr/include/flux/x86/pc/debug.h
- engine/apps/rmgr/include/flux/x86/pc/keyboard.h
- engine/apps/rmgr/include/flux/x86/pc/base_multiboot.h
- engine/apps/rmgr/include/flux/x86/pc/pic.h

### Task 8
- engine/apps/rmgr/include/flux/x86/pc/fdev.h
- engine/apps/rmgr/include/flux/x86/pc/irq_list.h
- engine/apps/rmgr/include/flux/x86/pc/com_cons.h
- engine/apps/rmgr/include/flux/x86/pc/phys_lmm.h
- engine/apps/rmgr/include/flux/x86/pc/pit.h
- engine/apps/rmgr/include/flux/x86/c/common.h
- engine/apps/rmgr/include/flux/x86/c/setjmp.h
- engine/apps/rmgr/include/flux/x86/c/stdarg.h
- engine/apps/rmgr/include/flux/x86/seg.h
- engine/apps/rmgr/include/flux/x86/asm.h
- engine/apps/rmgr/include/flux/x86/base_idt.h
- engine/apps/rmgr/include/flux/x86/types.h
- engine/apps/rmgr/include/flux/x86/fp_reg.h
- engine/apps/rmgr/include/flux/x86/debug.h
- engine/apps/rmgr/include/flux/x86/proc_reg.h
- engine/apps/rmgr/include/flux/x86/base_trap.h
- engine/apps/rmgr/include/flux/x86/cpuid.h
- engine/apps/rmgr/include/flux/x86/paging.h
- engine/apps/rmgr/include/flux/x86/fdev.h
- engine/apps/rmgr/include/flux/x86/multiboot.h
- engine/apps/rmgr/include/flux/x86/base_paging.h
- engine/apps/rmgr/include/flux/x86/base_cpu.h
- engine/apps/rmgr/include/flux/x86/anno.h
- engine/apps/rmgr/include/flux/x86/debug_reg.h
- engine/apps/rmgr/include/flux/x86/spin_lock.h

### Task 9
- engine/apps/rmgr/include/flux/x86/eflags.h
- engine/apps/rmgr/include/flux/x86/base_stack.h
- engine/apps/rmgr/include/flux/x86/trap.h
- engine/apps/rmgr/include/flux/x86/base_gdt.h
- engine/apps/rmgr/include/flux/x86/base_vm.h
- engine/apps/rmgr/include/flux/x86/gdb.h
- engine/apps/rmgr/include/flux/x86/gate_init.h
- engine/apps/rmgr/include/flux/x86/smp/smp.h
- engine/apps/rmgr/include/flux/x86/smp/asm-smp.h
- engine/apps/rmgr/include/flux/x86/smp/linux-smp.h
- engine/apps/rmgr/include/flux/x86/smp/bitops.h
- engine/apps/rmgr/include/flux/x86/smp/i82489.h
- engine/apps/rmgr/include/flux/gdb.h
- engine/apps/rmgr/include/flux/base_critical.h
- engine/apps/rmgr/include/flux/queue.h
- engine/apps/rmgr/include/flux/smp/smp.h
- engine/apps/rmgr/src/task.h
- engine/apps/rmgr/src/rmgr.c
- engine/apps/rmgr/src/libpci-compat.c
- engine/apps/rmgr/src/cfg-mem.c
- engine/apps/rmgr/src/oskit_support.c
- engine/apps/rmgr/src/irq.c
- engine/apps/rmgr/src/memmap_lock.h
- engine/apps/rmgr/src/coff-i386.h
- engine/apps/rmgr/src/globals.h

### Task 10
- engine/apps/rmgr/src/irq.h
- engine/apps/rmgr/src/libpci.c
- engine/apps/rmgr/src/config.h
- engine/apps/rmgr/src/memmap.h
- engine/apps/rmgr/src/grub_mb_info.h
- engine/apps/rmgr/src/init.c
- engine/apps/rmgr/src/exec.h
- engine/apps/rmgr/src/grub_vbe_info.h
- engine/apps/rmgr/src/quota.h
- engine/apps/rmgr/src/libpci-compat.h
- engine/apps/rmgr/src/globals.c
- engine/apps/rmgr/src/rmgr.h
- engine/apps/rmgr/src/memmap.c
- engine/apps/rmgr/src/cfg.h
- engine/apps/rmgr/src/libpci.h
- engine/apps/rmgr/src/small.h
- engine/apps/rmgr/src/trampoline.c
- engine/apps/rmgr/src/pe.c
- engine/apps/rmgr/src/startup.c

Continue creating additional tasks of 25 files until all sources are migrated.
