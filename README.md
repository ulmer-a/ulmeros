# UlmerOS Distribution

This repository consists of custom utilities, ports and build scripts to build
a functional hard drive image that can boot and provide an environment for the
UlmerOS Operating System kernel.

If you're looking for the OS kernel, have a look at the 'ulmeros' repository.

## Features under development for version 1:

I consider the following features as a minimal enivironment
for the Operating System to be usable.

* Standard C library (e.g. `newlib` or `mlibc`)
* `/bin/init` program to initialize userspace
* `bash` shell 
* basic command line tools (e.g. `busybox`)
