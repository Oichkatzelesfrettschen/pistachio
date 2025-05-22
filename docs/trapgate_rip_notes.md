# Trap Gate RIP-Relative Updates

The x86-64 trap gate wrappers now load the handler addresses and trap
reasons using RIP-relative addressing.  Older assemblers required
absolute references which caused relocation issues with modern tool
chains.  The wrappers allocate small constant symbols next to the
handlers and reference them with `leaq` and `pushq` instructions.
This keeps the code position independent and avoids large immediates.
