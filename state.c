#include "ls.h"
#include "common.h"
#include "state.h"
#include "mem.h"

typedef struct LG
{
	ls_GlobalState g;
	ls_State l;
} LG;

LS_API ls_State* ls_newstate(ls_MemAllocator alloc, void* ud)
{
	LG* lg = CAST(LG*, alloc(ud, ls_NULL, LSM_ALLOC_FIX, sizeof(LG)));
	if (!lg)
	{
		return ls_NULL;
	}

	/* setup global state */
	ls_GlobalState* g = &lg->g;
	g->alloc = alloc;
	g->alloc_ud = ud;
	memset(g->memused, 0, sizeof(g->memused));
	lsM_setmemstat_g(g, LSM_ALLOC_FIX, sizeof(LG));

	/* setup local state */
	ls_State* L = &lg->l;
	L->g = g;

	return L;
}

LS_API void ls_close(ls_State* L)
{
	//currently do nothing
}
