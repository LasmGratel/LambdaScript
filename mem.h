//mem.h
//memory manipulate functions

#ifndef LS_MEM_H
#define LS_MEM_H

//Helpers to manipulate blocks, fixed-sized objects, arrays
//Blocks are raw memory blocks that can be resized, 
//objects are fixed blocks and arrays are blocks with a header structure

#define lsM_newblock(L, n) lsM_alloc_(L, ls_NULL, 0, n)
#define lsM_resizeblock(L, p, o, n) lsM_alloc_(L, p, o, n)
#define lsM_freeblock(L, p, n) lsM_alloc_(L, p, n, 0)

#define lsM_newobj(L, t) lsM_alloc_(L, ls_NULL, 0, sizeof(t))
#define lsM_freeobj(L, p, t) lsM_alloc_(L, p, sizeof(t), 0)

//Helpers to set memory statistics in ls_State
//Note that memory flag includes those used by vm internally
/*
#define lsM_setmemstat_g(g, u, changed) {\
		g->memused[0] += (changed); \
		if (u & LSM_ALLOC_USAGE) g->memused[lsM_allocusage(u)] += (changed); \
	}
#define lsM_setmemstat(L, u, changed) lsM_setmemstat_g(G(L), u, changed)
*/
#define lsM_setmemstat_g(g, changed) ((g)->memused += (changed))
#define lsM_setmemstat(L, changed) lsM_setmemstat_g(G(L), changed)

//lsM_alloc_: s_old is just 0 if its a new block
LSI_EXTERN void* lsM_alloc_(ls_State* L, void* block, ls_MemSize s_old, ls_MemSize s_new);

LSI_EXTERN void* lsM_doubleblock(ls_State* L, void* p, ls_MemSize* sz);

//vector (from lua)
LSI_EXTERN ls_noreturn lsM_toobig(ls_State *L);
LSI_EXTERN void *lsM_growaux_(ls_State *L, void *block, int *size,
	ls_MemSize size_elem, int limit, const char *what);


#define lsM_reallocv(L,b,on,n,e) \
	(cast(void, \
	(cast(ls_MemSize, (n)+1) > MAX_MEMSIZE / (e)) ? (lsM_toobig(L), 0) : 0), \
	lsM_alloc_(L, (b), (on)*(e), (n)*(e)))
#define lsM_newvector(L,n,t) \
	cast(t *, lsM_reallocv(L, ls_NULL, 0, n, (ls_MemSize)sizeof(t)))
#define lsM_growvector(L,v,nelems,size,t,limit,e) \
	if ((nelems)+1 > (size)) \
		((v) = cast(t *, lsM_growaux_(L, v, &(size), sizeof(t), limit, e)))
#define lsM_reallocvector(L, v,oldn,n,t) \
	((v) = cast(t *, lsM_reallocv(L, v, oldn, n, sizeof(t))))
#define lsM_freevector(L, b, n)   lsM_reallocv(L, (b), n, 0, (ls_MemSize)sizeof((b)[0]))

#endif
