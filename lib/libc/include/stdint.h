#ifndef __STDINT_H
#define __STDINT_H

typedef signed char int8_t; 
typedef int int16_t; 
typedef long int32_t; 
typedef long long int64_t; 

typedef long long intmax_t;
typedef int intptr_t;


typedef unsigned char uint8_t; 
typedef unsigned int uint16_t; 
typedef unsigned long uint32_t; 
typedef unsigned long long uint64_t;

typedef unsigned long long uintmax_t;
typedef unsigned int uintptr_t;

#define INT8_MIN   (-1-0x7f)
#define INT16_MIN  (-1-0x7fff)
#define INT32_MIN  (-1-0x7fffffff)
#define INT64_MIN  (-1-0x7fffffffffffffff)

#define INT8_MAX   (0x7f)
#define INT16_MAX  (0x7fff)
#define INT32_MAX  (0x7fffffff)
#define INT64_MAX  (0x7fffffffffffffff)

#define UINT8_MAX  (0xff)
#define UINT16_MAX (0xffffu)
#define UINT32_MAX (0xffffffffu)
#define UINT64_MAX (0xffffffffffffffffu)

#endif
