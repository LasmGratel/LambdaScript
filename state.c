#include "ls.h"
#include "common.h"
#include "state.h"
#include "mem.h"
#include "string.h"

LSI_EXTERN void lsX_init(ls_State* L);

typedef struct LG
{
	ls_GlobalState g;
	ls_State l;
} LG;

static void open_state(ls_State* L)
{
	lsX_init(L);
}

static void close_state(ls_State* L)
{

}

LS_API ls_State* ls_newstate(ls_MemAllocator alloc, void* ud)
{
	LG* lg = cast(LG*, alloc(ud, ls_NULL, 0, sizeof(LG)));
	if (!lg)
	{
		return ls_NULL;
	}

	ls_GlobalState* g = &lg->g;
	ls_State* L = &lg->l;
	L->g = g;

	/* setup global state */
	/* memory related */
	g->alloc = alloc;
	g->alloc_ud = ud;
	g->memused = 0;
	lsM_setmemstat_g(g, sizeof(LG));

	/* string table */
	g->strt.nuse = g->strt.size = 0;
	g->strt.hash = ls_NULL;
	lsS_resize(L, MIN_STRTAB_SIZE);

	/* GC */
	g->allgc = ls_NULL;

	/* setup local state */

	/* memory allocating */
	open_state(L);
	return L;
}

LS_API void ls_close(ls_State* L)
{
	//currently do nothing
}
