# piOS
Operating system for my custom processor - [pcpu](https://github.com/piotro888/pcpu)

This is my self-educational project. Homemade OS created because I wanted to make something fun for my processor, and got interested in operating systems.

You can get tools to build and emulate it from here: [pcpu-toolchain](https://github.com/piotro888/pcpu-toolchain)

**In development**, don't expect too much from it ***yet***.

## Features 
* Runs on 16 bit *pcpu*
* VGA driver
* PS/2 Keyboard
* First userspace process which can be interrupted and resumed
* Kernel library: printf, malloc, strings, data structures, logs
* SD card driver over SPI bus
* TAR filesystem support

Not much yet, but the os is in early development stage now. I will keep the list this updated.

Runing processes in userspace is supported, so this is a real OS now!

Note that all devices are specific to my dev board.

### Incoming features
* Kernel threads
* Kernel synchronization primitives
* *Main syscall queue and dispatcher*
* System design documentation

## Build
Build piOS kernel
```bash
cd kernel
# build .hex
make
# or build and upload to dev board
make upload
# or build and run in emulator
make pce
```
If using pce add `test.tar` archive in main directory for sd card emulation.
## 

## OS goals
* Multiprocessing and networking - `run simple web server in background`
* UNIX-like shell
* Virtual file system with UNIX-like devices
* Virtual memory managment
* Custom user process managment, without kernel thread for each process

## Author
`piOS (C) 2021-2022 by Piotr Wegrzyn`