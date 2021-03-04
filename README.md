# Ulmer OS

UlmerOS is a general purpose portable unix-like educational operating system. Please
note that this (incomplete) repository contains a rewrite of the original OS project
started in 2017. The rewrite was necessary due to lots of synchronization issues and
design flaws in the original code. After quite some years of experimenting and taking
the OS course at University of Technology in Graz, Austria (where we implemented
multithreading, advanced memory management and page swapping almost from scratch),
I eventually decided to do a full rewrite with the gained knowledge.

Since then, many parts of the OS have greatly improved, such as the ATA hard disk driver
that now uses DMA and the virtual memory management system with very stable copy-on-write
support that I just need to move over to this repository.

As a result of the reimplementation, I took down the old repository as it doesn't
contain anything that anyone could learn from anymore.

# Features

## Features currently supported
* Full x86\_64 architecture support
* Preemtive multitasking
* PCI ATA HDD Driver (DMA)
* ext2 read support
* Unix-like virtual filesystem

Features that will be supported in the feature:
* Ports of `bash` and `coreutils`/`busybox`
* Ports of `newlib` C library, `libstdc++`
* ext2 filesystem write support
* VBE Graphics console
* PS/2 Keyboard driver
* Network stack with ethernet driver
* `fork()` and `exec()` with `copy and write`
* Run userspace programs in 32bit compatibility mode on x86\_64

# Building Ulmer OS

## Build instructions

Configuration can be done by passing the configuration arguments to `cmake`. The
`menuconf` script in the project root directory automates this project and lets
you choose configuration options via a text GUI, similar to menuconfig in the linux
kernel.

```
$ mkdir build && cd build
$ ../menuconf
$ make
```

