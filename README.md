# Ulmer OS

UlmerOS is a general purpose portable unix-like educational operating system.

# Overview

## Boot
This is subject to change.

## Virtual memory
The `vspace_t` type represents a virtual address space in UlmerOS. The generic
header interface `vspace.h` is implemented by each architecture. The exact memory
layout is architecture-dependent.

## Scheduling
Currently, the scheduler implements a basic round-robin scheduling policy. The
scheduler calls `task_schedulable()` on a given task to determine whether it can be
scheduled or not. Don't modify the `state` field, but instead modify `task_schedulable()`.

## Syscalls
System calls provide a way for user space programs to call kernel routines.

## Debugging
The `debug()` macro provides an easy-to-use thread-safe logging mechanism which
prints to the QEMU or Bochs emulator debug output. Different log levels can be
individually switched on or off in `debug.h`.

## Interrupts
Proper interrupt handling has to be implemented.

## Virtual file system
Not yet implemented.

## Kernel C-library

### Linked list (list_t)
`list_t` is an important implementation of a linked list heavily used
in the entire kernel. It provides a lot of helper functions to make
dealing with linked lists as easy as possible.

### String library (kstring.h)
The kernel string library provides functionality similar to what can be found
in userspace `string.h` libraries. Beware that some less-frequently used
functions might not yet be implemented.

### FIFO (fifo_t)
Not implemented yet.

### Mutex (mutex_t)
Mutexes provide a way to synchronize access to shared resources. Before
scheduling is enabled, mutexes can be used too, although in that case they
behave like spinlocks.

### Condition variables (cond_t)
Condition variables provide a convenient way of signaling events to other
kernel threads. Using condition variables only makes sense after scheduling
was enabled.

## PCI bus
The initialization thread performs a PCI scan before drivers are loaded. This
will create a list of devices. The `bus/pci.h` header provides access to the PCI
configuration space and allows drivers to register themselves.

### Supported PCI devices
* `pc/ata.c`: Intel 82371AB/EB/MB PIIX4 IDE Controller
* `pc/ata.c`: Intel 82371SB PIIX3 IDE Controller (Natoma/Triton II)

## Supported platforms
The OS is implemented in a portable way to allow easy portability to new
processor architectures and hardware platforms. Currently, the following
architectures are supported:

### amd64-pc
Supports 64bit PC's. On this architecture, an additional 32bit boot stage is
required, which sets up boot page tables and jumps into 64bit mode.
