//ls.h
//External header file

#ifndef LS_LS_H
#define LS_LS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "config.h"

#define ls_noreturn void

#define ls_Number double

/* memory allocate type */
/*
#define LSM_ALLOC_UNKNOWN   0x00

#define LSM_ALLOC_STRATEGY  0x0f
#define LSM_ALLOC_FIX       0x01
#define LSM_ALLOC_VARIABLE  0x00
#define LSM_ALLOC_LARGE     0x02
#define LSM_ALLOC_MEDIUM    0x00
#define LSM_ALLOC_PERMANENT 0x04
#define LSM_ALLOC_TEMPORARY 0x00
#define lsM_allocstrgy(n) (n & LSM_ALLOC_STRATEGY)

#define LSM_ALLOC_USAGE     0xf0
#define LSM_ALLOC_USAGE_N   0
#define lsM_allocusage(n) ((n & LSM_ALLOC_USAGE) << 2)
*/
/* error number */
#define LS_ERRRUN   1
#define LS_ERRMEM   2

/* use ptrdiff_t to operate minus when calculating memory usage */
typedef ptrdiff_t ls_MemSize;

/* s_old is memory allocate type when block is NULL (different from lua) */
typedef void*(*ls_MemAllocator)(void* ud, void* block, ls_MemSize s_old, ls_MemSize s_new);

typedef const char * (*ls_InputReader) (void* ud, ls_MemSize* sz);

typedef struct ls_State ls_State;

LS_API ls_State* ls_newstate(ls_MemAllocator alloc, void* ud);
LS_API void ls_close(ls_State* L);

LS_API ls_noreturn ls_throw(ls_State* L, int code, const char* msg, ...);

#endif
