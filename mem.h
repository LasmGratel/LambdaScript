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

#define lsM_newblock(L, info, size) (ls_NULL)
#define lsM_freeblock(L, p, size) (void(0))

#define lsM_newobj(L, info, t) lsM_alloc_(L, ls_NULL, info.f, sizeof(t))
#define lsM_freeobj(L, p, t) lsM_alloc_(L, p, sizeof(t), 0);

#define lsM_newarray(L, info, t, n) \
	lsM_allocarray_(L, ls_NULL, (n) * sizeof(t), info)
#define lsM_resizearray(L, info, p, t, nnew, limit) \
	lsM_allocarray_(L, p, (nnew) * sizeof(t), info)
#define lsM_freearray(L, info, p, t) \
	lsM_allocarray_(L, p, 0, info)

//Helpers to set memory statistics in ls_State
//Note that memory usage includes those used by vm internally

#define lsM_setmemstat_g(g, u, changed) {\
		g->memused[0] += (changed); \
		if (u & LSM_ALLOC_USAGE) g->memused[lsM_allocusage(u)] += (changed); \
	}
#define lsM_setmemstat(L, u, changed) lsM_setmemstat_g(G(L), u, changed)

LSI_EXTERN void* lsM_alloc_(ls_State* L, void* block, ls_MemSize s_old, ls_MemSize s_new);
LSI_EXTERN void* lsM_allocarray_(ls_State* L, void* block, ls_MemSize newsize, ArrayInfo* info);

#endif
