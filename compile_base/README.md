## Compile base

This is a sysroot for cross-compilation for `pcpu-unknown-pios` target (userspace only).

It should contain libc, crt objects and default linker script.

`compile_base` is separated from `base`, because OS is not able to link/compile under itself and those files would not be necessary.


### Setup

To setup sysroot structure with neccessary libraries run `./make_base.sh` script.

It will `make` all required libraries (libc.a, crt*.o) and copy them to appropiate places

### Compilation

To simply cross-compile a program with default arguments and linked to system libraries run:
```
clang -target pcpu-unknown-pios --sysroot=<path to compile_base/> <files to compile>
```
Add `-v` to see toolchain invocations.