#ifndef LS_LS_H
#define LS_LS_H

#include <stdlib.h>
#include <string.h>
#include "config.h"

#define ls_noreturn void

/* memory allocate type */

#define LSM_ALLOC_UNKNOWN   0x00
//ls.h
//External header file

#define LSM_ALLOC_STRATEGY  0x0f
#define LSM_ALLOC_FIX       0x01
#define LSM_ALLOC_LARGE     0x02
#define LSM_ALLOC_VARIABLE  0x03
#define lsM_allocstrgy(n) (n & LSM_ALLOC_STRATEGY)

#define LSM_ALLOC_USAGE     0xf0
#define LSM_ALLOC_USAGE_N   0
#define lsM_allocusage(n) ((n & LSM_ALLOC_USAGE) << 2)

/* use signed to operate - when calculating memory usage */
typedef int ls_MemSize;

/* s_old is memory allocate type when block is NULL (different from lua) */
typedef void*(*ls_MemAllocator)(void* ud, void* block, ls_MemSize s_old, ls_MemSize s_new);

typedef struct ls_State ls_State;

LS_API ls_State* ls_newstate(ls_MemAllocator alloc, void* ud);
LS_API void ls_close(ls_State* L);

#endif
