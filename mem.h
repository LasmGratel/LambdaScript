//mem.h
//memory manipulate functions

#ifndef LS_MEM_H
#define LS_MEM_H

/* information used in array allocation */
typedef struct ArrayInfo
{
	int flag;
	int limit;
	const char* what;
} ArrayInfo;

//Helpers to manipulate blocks, fixed-sized objects, arrays
//Blocks are raw memory blocks that can be resized, 
//objects are fixed blocks and arrays are blocks with a header structure

#define lsM_newblock(L, flag, n) lsM_alloc_(L, ls_NULL, 0, n, flag)
#define lsM_resizeblock(L, flag, p, o, n) lsM_alloc_(L, p, o, n, flag)
#define lsM_freeblock(L, flag, p, n) CAST(void, lsM_alloc_(L, p, n, 0, flag));

#define lsM_newobj(L, flag, t) lsM_alloc_(L, ls_NULL, 0, sizeof(t), flag)
#define lsM_freeobj(L, flag, p, t) CAST(void, lsM_alloc_(L, p, sizeof(t), 0, flag))

#define lsM_newarray(L, info, t, n) \
	lsM_allocarray_(L, ls_NULL, (n) * sizeof(t), info)
#define lsM_resizearray(L, info, p, t, n) \
	lsM_allocarray_(L, p, (n) * sizeof(t), info)
#define lsM_freearray(L, info, p) CAST(void, lsM_freearray_(L, p, (info)->flag))

//Helpers to set memory statistics in ls_State
//Note that memory flag includes those used by vm internally

#define lsM_setmemstat_g(g, u, changed) {\
		g->memused[0] += (changed); \
		if (u & LSM_ALLOC_USAGE) g->memused[lsM_allocusage(u)] += (changed); \
	}
#define lsM_setmemstat(L, u, changed) lsM_setmemstat_g(G(L), u, changed)

//lsM_alloc_: s_old is just 0 if its a new block
LSI_EXTERN void* lsM_alloc_(ls_State* L, void* block, ls_MemSize s_old, ls_MemSize s_new, int flag);
LSI_EXTERN void* lsM_allocarray_(ls_State* L, void* block, ls_MemSize newsize, ArrayInfo* info);
LSI_EXTERN void lsM_freearray_(ls_State* L, void* block, int flag);

#endif
