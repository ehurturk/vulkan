# Memory Subsystem

To decrease memory fragmentation and calls to `new/malloc` and `delete/free`, a custom memory allocator system is used.

This ensures no memory fragmentation and no context switches from user to kernel mode. In addition, it is possible to
allocate in aligned sizes, thus spatial cache locality is satisfied.

It is intended to make these allocators thread safe, using lock free and `thread_local` approaches wherever possible.

## Allocators

### Stack Allocator

### Destack Allocator

### Pool Allocator

### Arena Allocator

### Linear Allocator

### Freelist Allocator (with RedBlack Trees)