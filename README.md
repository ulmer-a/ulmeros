# Ulmer OS

UlmerOS is a general purpose portable unix-like educational operating system. Please
note that this (incomplete) repository contains a rewrite of the original OS project
started in 2017. The rewrite was necessary due to lots of synchronization issues and
design flaws in the original code. After quite some years of experimenting and taking
the OS course at University of Technology in Graz, Austria (where we implemented
multithreading, advanced memory management and page swapping almost from scratch),
I eventually decided to do a full rewrite with the gained knowledge.

# Building Ulmer OS

Building UlmerOS is rather easy. Make sure you've got your latest binutils and
gcc installed from the repositories for the architecture you're building. You'll
also need `cmake`.

## Build instructions for amd64-pc platform

As for now, this is the only supported platform. First clone the repository into
any directory on your machine. We're going to assume you cloned into `~/ulmeros`.

```
$ mkdir /tmp/ulmeros
$ cd /tmp/ulmeros
$ cmake ~/ulmeros
$ make
```

After that you can copy the files to a hard drive, install grub and boot it. You
can also start the OS in an emulator:

```
$ make qemu
```

# Overview

## Programming on UlmerOS

As of December 2020, the public domain C library (pdclibc) has been added to the project.
That means, it is now possible to develop applications for the OS in C with a
little bit more comfort than just on bare metal. Currently, the following features of the C standard
library are supported:

* stdio
* stdlib
* ctype
* string
* time
* unistd (incomplete)

However, due to PDClib not being a complete standard C library, this is not enough to port
entire UNIX applications over. It will be necessary to port another C library (newlib?) in the future.

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
