#include "ls.h"
#include "common.h"
#include "state.h"
#include "mem.h"

#define BLOCK_MAX_SIZE 0x10000
#define ALLOC_FAILED ls_throw(L, LS_ERRMEM, "memory allocation failed: allocator returned NULL")

void* lsM_alloc_(ls_State* L, void* block, ls_MemSize s_old, ls_MemSize s_new)
{
	if (s_new == 0 && block == ls_NULL) return ls_NULL; //To avoid undefined behavior on realloc(NULL, 0)
	ls_assert(s_old >= 0 && s_new >= 0);
	if (s_new > BLOCK_MAX_SIZE)
	{
		lsM_toobig(L);
	}
	ls_GlobalState* g = G(L);
	void* ret = (*g->alloc)(g->alloc_ud, block, s_old, s_new);
	if (s_new > 0 && !ret)
	{
		ls_apicheck(s_new > s_old);
		ALLOC_FAILED;
	}
	ls_assert((ret == ls_NULL) != (s_new > 0));
	lsM_setmemstat_g(g, s_new - s_old);
	return ret;
}

void* lsM_doubleblock(ls_State* L, void* p, ls_MemSize* sz)
{
	ls_MemSize s = *sz;
	ls_MemSize ss;
	*sz = ss = s == 0 ? 256 : s * 2;
	return lsM_alloc_(L, p, s, ss);
}

#define MINSIZEARRAY	4

void *lsM_growaux_(ls_State *L, void *block, int *size, ls_MemSize size_elems,
	int limit, const char *what) {
	void *newblock;
	int newsize;
	if (*size >= limit / 2) {  /* cannot double it? */
		if (*size >= limit)  /* cannot grow even a little? */
			ls_throw(L, LS_ERRMEM, "too many %s (limit is %d)", what, limit);
		newsize = limit;  /* still have at least one free place */
	}
	else {
		newsize = (*size) * 2;
		if (newsize < MINSIZEARRAY)
			newsize = MINSIZEARRAY;  /* minimum size */
	}
	newblock = lsM_reallocv(L, block, *size, newsize, size_elems);
	*size = newsize;  /* update only when everything else is OK */
	return newblock;
}

ls_noreturn lsM_toobig(ls_State *L)
{
	ls_throw(L, LS_ERRMEM, "memory allocation error: block too big");
}
