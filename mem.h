//mem.h
//memory manipulate functions

#ifndef LS_MEM_H
#define LS_MEM_H

/* information used in array allocation */
typedef struct ArrayInfo
{
	int usage;
	int limit;
	const char* what;
} ArrayInfo;

//Helpers to manipulate blocks, fixed-sized objects, arrays

#define lsM_newblock(L, usage, size) (ls_NULL)
#define lsM_freeblock(L, usage, p, size) (void(0))

#define lsM_newobj(L, usage, t) lsM_alloc_(L, ls_NULL, 0, sizeof(t), usage)
#define lsM_freeobj(L, usage, p, t) CAST(void, lsM_alloc_(L, p, sizeof(t), 0, usage));

#define lsM_newarray(L, info, t, n) \
	lsM_allocarray_(L, ls_NULL, (n) * sizeof(t), info)
#define lsM_resizearray(L, info, p, t, n) \
	lsM_allocarray_(L, p, (n) * sizeof(t), info)
#define lsM_freearray(L, info, p) CAST(void, lsM_freearray_(L, p, (info)->usage))

//Helpers to set memory statistics in ls_State
//Note that memory usage includes those used by vm internally

#define lsM_setmemstat_g(g, u, changed) {\
		g->memused[0] += (changed); \
		if (u & LSM_ALLOC_USAGE) g->memused[lsM_allocusage(u)] += (changed); \
	}
#define lsM_setmemstat(L, u, changed) lsM_setmemstat_g(G(L), u, changed)

LSI_EXTERN void* lsM_alloc_(ls_State* L, void* block, ls_MemSize s_old, ls_MemSize s_new, int usage);
LSI_EXTERN void* lsM_allocarray_(ls_State* L, void* block, ls_MemSize newsize, ArrayInfo* info);
LSI_EXTERN void lsM_freearray_(ls_State* L, void* block, int usage);

#endif
