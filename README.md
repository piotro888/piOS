# piOS
Operating system for my custom processor - [ppcpu](https://github.com/piotro888/ppcpu)

Stared as homemade OS created because I wanted to make something fun for my processor, and got interested in operating systems.

Now it is a fully featured operating system that demostrates technical possibilities of PCPU and can be used to easily create useful and complex apps for it.

You can get the toolchain to run it here: [llvm-pcpu](https://github.com/piotro888/llvm-project-pcpu) (full LLVM toolchain) and [pcsn](https://github.com/piotro888/pcsn) (emulator).

## Features 
* Runs on 16 bit custom procesoor **pcpu**!
* Fully isolated userspace processes support
* IPC mechanisms
* Hybrid kernel design
* Virtual filesystem with UNIX-like devices and mounting other FS
* Basic libc implementation
* UNIX-like syscalls
* Async I/O
* Synchronization primitives
* Kernel library
* TAR filesystem support from SD card
* Multiple peripheral drivers

*This is a real OS now, running preemptively multiple isolated processes in userspace, loaded from ELF file on SD card mounted on VFS!*

Note that all devices are currently specific to my dev board / emulator.

## Build

Firstly, build C libraries for userspace and kernel
```bash
# compile kerel libc
cd lib/libc/ && make libc_k
# compile libc, libcrt and all dependencies for userspace
cd compile_base
./make_base.sh
```

Then build piOS kernel
```bash
cd kernel
# build build/kernel.elf, build/kernel_data.bin, build/kernel_text.bin
make
# or build and upload to dev board
make upload
```
You can use `build/kernel_text.bin` and `build/kerenl_data.bin` for emulating piOS.

Then build your userspace apps (see example in: `userspace/simple_build_example/`).

Move your executable to `base/` directory and then generate tar archive from it using `pack_sd.sh`.
Flash `test.tar` archive on SD card using `dd` or provide path to it if using emulator.

Don't foreget to rebuild libraries/kernel after changes!

## OS goals
* ~Multiprocessing~ DONE and networking - `run simple web server in background`
* UNIX-like shell
* ~~Virtual file system with UNIX-like devices~~ DONE
* ~~Virtual memory managment~~ DONE
* ~~Custom user process managment, without kernel thread for each process~~

## Author
Licensed under BSD-2-Clause license
`piOS (C) 2021-2023 by Piotr Wegrzyn`
