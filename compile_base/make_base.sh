#!/bin/sh

set -e

mkdir -p bin
mkdir -p lib
mkdir -p usr/include

PWDC=$(pwd)

cd ../lib/crt && make && cp build/* ../../compile_base/lib/

cd ${PWDC}
cd ../lib/libc && make libc && cp build/libc.a ../../compile_base/lib/ && cp -r include/* ${PWDC}/usr/include

LLVM_PREFIX=~/llvm-project/rbuild

cd ${PWDC}
rm -f bin/clang bin/llvm-mc bin/ld.lld
ln -s ${LLVM_PREFIX}/bin/clang bin/
ln -s ${LLVM_PREFIX}/bin/llvm-mc bin/
ln -s ${LLVM_PREFIX}/bin/ld.lld bin/

