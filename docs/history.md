# Kernel Architecture History

Pistachio began as a classic microkernel providing rich in-kernel services. Over time, the project embraced an exokernel approach, stripping the kernel down to the bare essentials and moving policy decisions into user space. This transition allows specialised servers and libraries to implement features such as scheduling and memory management while keeping the trusted computing base minimal.
