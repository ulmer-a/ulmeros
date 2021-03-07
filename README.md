# Ulmer OS

UlmerOS is a general purpose, portable unix-like operating system. Please
note that this repository contains a rewrite of the original OS project
started in 2017. The rewrite was necessary due to lots of design flaws and
synchronization issues in the original code. After quite some years of
experimenting and taking the OS course at University of Technology in Graz,
Austria, I eventually decided to do a full rewrite with the gained knowledge.

Since then, many parts of the OS have greatly improved, such as the ATA hard
disk driver that now is capable of doing DMA transfers, the virtual memory
management system with copy-on-write and full x86\_64 architecture support.

## Project status

Development is aiming at shipping version 1.0 of the OS, which requires the
following features to be implemented and stable:
* Architecture support for `x86\_64` (and maybe `i386`)
* Preemptive multitasking & virtual memory
* Basic set of supported system calls (`read()`, `write()`, `fork()`, `execve()`, `sbrk()`, ...)
* Virtual file system and `ext2` filesystem read (maybe also write) support
* ATA disk driver
* PC console and keyboard drivers
* C standard library
* Working ports of `bash` and `coreutils`

### Kernel
Currently the kernel can successfully boot and mount an ext2 root filesystem,
and then run a binary passed on the kernel command line (such as `/bin/init`).
Full support for virtual memory as well as a few system calls (`exit()`, `sbrk()`,
`read()`, `write()`) and lazy allocation of code, data and stack pages is
implemented and working. 
 
### Userspace
For developing userspace programs, a full C Standard library is available
(RedHat Newlib) by a port and programs can be statically linked against it.
However, a dedicated toolchain is still missing and not many programs have
been developed yet.

## Building Ulmer OS

### Build instructions

Configuration can be done by passing the configuration arguments to `cmake`. The
`menuconf` script in the project root directory automates this project and lets
you choose configuration options via a text GUI, similar to menuconfig in the linux
kernel.

```
$ mkdir build && cd build
$ ../menuconf
$ make
```
