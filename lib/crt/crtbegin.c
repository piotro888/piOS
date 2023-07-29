// crtbegin.o and crtend.o should be provided by compiler,
// these are simply copied from LLVM compiler-rt.
// TOOD: Remove when compiler-rt will be ported (will be needed soon).

// use .ctors and .dtors
//#define CRT_HAS_INITFINI_ARRAY

//===-- crtbegin.c - Start of constructors and destructors ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//-------------------------------------------------------------===//

#include <stddef.h>

unsigned int syscall_rawx(unsigned int p0, unsigned int p1, unsigned int p2, unsigned int p3, unsigned int p4) {
    unsigned int ret;
    asm volatile (
        " ldo r0, %1 \n"
        " ldo r1, %2 \n"
        " ldo r2, %3 \n"
        " ldo r3, %4 \n"
        " ldo r6, %5 \n"
        " sys \n"
        " mov %0, r0 \n"
        : "=r"(ret)
        : "m"(p0), "m"(p1), "m"(p2), "m"(p3), "m"(p4)
        : "r0", "r1", "r2", "r3", "r6");
    return ret;
}


__attribute__((visibility("hidden"))) void *__dso_handle = &__dso_handle;

#ifdef EH_USE_FRAME_REGISTRY
__extension__ static void *__EH_FRAME_LIST__[]
    __attribute__((section(".eh_frame"), aligned(sizeof(void *)))) = {};

extern void __register_frame_info(const void *, void *) __attribute__((weak));
extern void *__deregister_frame_info(const void *) __attribute__((weak));
#endif

#ifndef CRT_HAS_INITFINI_ARRAY
typedef void (*fp)(void);

static fp __CTOR_LIST__[]
    __attribute__((section(".ctors"), aligned(sizeof(fp)))) = {(fp)-1};
extern fp __CTOR_LIST_END__[];
#endif

extern void __cxa_finalize(void *) __attribute__((weak));

static void __attribute__((used)) __do_init(void) {
  static _Bool __initialized;
  if (__builtin_expect(__initialized, 0))
    return;
  __initialized = 1;

#ifdef EH_USE_FRAME_REGISTRY
  static struct { void *p[8]; } __object;
  if (__register_frame_info)
    __register_frame_info(__EH_FRAME_LIST__, &__object);
#endif
#ifndef CRT_HAS_INITFINI_ARRAY
  const size_t n = __CTOR_LIST_END__ - __CTOR_LIST__ - 1;
  syscall_rawx(0,3,n,0,0);
  for (size_t i = n; i >= 1; i--) {syscall_rawx(0,4,(unsigned int)__CTOR_LIST__[i],0,0);__CTOR_LIST__[i]();}
#endif
}

#ifdef CRT_HAS_INITFINI_ARRAY
__attribute__((section(".init_array"),
               used)) static void (*__init)(void) = __do_init;
#else
__asm__(".pushsection .init,\"ax\",@progbits\n\t"
    "jal r6, " __USER_LABEL_PREFIX__ "__do_init\n\t"
    ".popsection");
#endif // CRT_HAS_INITFINI_ARRAY

#ifndef CRT_HAS_INITFINI_ARRAY
static fp __DTOR_LIST__[]
    __attribute__((section(".dtors"), aligned(sizeof(fp)))) = {(fp)-1};
extern fp __DTOR_LIST_END__[];
#endif

static void __attribute__((used)) __do_fini(void) {
  static _Bool __finalized;
  if (__builtin_expect(__finalized, 0))
    return;
  __finalized = 1;

  if (__cxa_finalize)
    __cxa_finalize(__dso_handle);

#ifndef CRT_HAS_INITFINI_ARRAY
  const size_t n = __DTOR_LIST_END__ - __DTOR_LIST__ - 1;
  for (size_t i = 1; i <= n; i++) __DTOR_LIST__[i]();
#endif
#ifdef EH_USE_FRAME_REGISTRY
  if (__deregister_frame_info)
    __deregister_frame_info(__EH_FRAME_LIST__);
#endif
}

#ifdef CRT_HAS_INITFINI_ARRAY
__attribute__((section(".fini_array"),
               used)) static void (*__fini)(void) = __do_fini;
#else
__asm__(".pushsection .fini,\"ax\",@progbits\n\t"
    "jal r6, " __USER_LABEL_PREFIX__ "__do_init\n\t"
    ".popsection");
#endif  // CRT_HAS_INIT_FINI_ARRAY
