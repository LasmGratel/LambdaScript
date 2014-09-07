//mem.h
//memory manipulate functions

#ifndef LS_MEM_H
#define LS_MEM_H

#define lsM_newblock(L, w, s)
#define lsM_freeblock(L, s, p)

#define lsM_newobj(L, w, t)
#define lsM_freeobj(L, t, p)

#define lsM_newarray(L, w, t, n)
#define lsM_resizearray(L, w, t, p, nold, nnew)
#define lsM_freearray(L, t, p)

#define lsM_setmemstat_g(g, u, nold, nnew) {\
		g->memused[0] += (nnew) - (nold); \
		if (u & LSM_ALLOC_USAGE) g->memused[lsM_allocusage(u)] += (nnew) - (nold); \
	}
#define lsM_setmemstat(L, u, nold, nnew) lsM_setmemstat_g(G(L), u, nold, nnew)

LSI_EXTERN void* lsM_alloc_(ls_State* L, void* block, ls_MemSize s_old, ls_MemSize s_new);
LSI_EXTERN ls_noreturn lsM_toobig(ls_State* L, const char* what);

#endif
