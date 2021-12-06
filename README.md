# piOS
Operating system for my custom processor - [pcpu](https://github.com/piotro888/pcpu)

This is my self-educational project. Homemade  OS created because I wanted to make something fun for my processor, and got interested in operating systems.

**In development**, don't expect too much from it ***yet***.

## Features 
* Runs on 16 bit pcpu
* VGA driver
* PS/2 Keyboard
* Kernel library: printf, malloc, strings
* TAR filesystem support (without device driver)

Not much yet, but the os is in early development stage now. I will keep the list this updated.

System is currently kernel-mode only (even without processes).  
Note that all devices are specific to my dev board.

## Build
Build piOS kernel
```bash
cd kernel
# build .hex
make
# or build and upload to dev board
make upload
# or build and run in emulator
make emu
```

## OS goals
* Multiprocessing and networking - `run simple web server in background`
* UNIX-like shell
----
* Virtual file system with UNIX-like devices
* Virtual memory menagment

## Author
piOS (C) 2021 by Piotr Wegrzyn