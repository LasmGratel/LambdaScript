#include "ls.h"
#include "common.h"
#include "state.h"
#include "mem.h"

#define BLOCK_MAX_SIZE 0x10000
#define ALLOC_FAILED ls_throw(L, LS_ERRMEM, "memory allocation failed: allocator returned NULL")

void* lsM_alloc_(ls_State* L, void* block, ls_MemSize s_old, ls_MemSize s_new, int flag)
{
	ls_assert(s_old >= 0 && s_new >= 0);
	if (s_new > BLOCK_MAX_SIZE)
	{
		ls_throw(L, LS_ERRRUN, "memory allocation failed: block is too large");
	}
	ls_MemSize real_old = block ? s_old : 0;
	ls_GlobalState* g = G(L);
	void* ret = (*g->alloc)(g->alloc_ud, block, s_old, s_new);
	if (s_new > 0 && !ret)
	{
		ls_apicheck(s_new > real_old);
		ALLOC_FAILED;
	}
	ls_assert((ret == ls_NULL) != (s_new > 0));
	lsM_setmemstat_g(g, flag, s_new - real_old);
	return ret;
}

//Structure of an array:
/*
   [MemSize current_size] [element] [element] ...
                              x
                       returned pointer
*/
//ls_MemSize(ptrdiff_t) is usually well-aligned

typedef struct ArrayHeader
{
	ls_MemSize n; //Total size of the block allocated
} ArrayHeader;

#define HEADER_TO_ARRAY(x) (CAST(ArrayHeader*, x) + 1)
#define ARRAY_TO_HEADER(x) (CAST(ArrayHeader*, x) - 1)

void* lsM_allocarray_(ls_State* L, void* block, ls_MemSize newsize, ArrayInfo* info)
{
	ls_assert(newsize > 0); //free an array in lsM_freearray_
	if (newsize > info->limit)
	{
		ls_throw(L, LS_ERRRUN, "memory allocation failed: too many %s", info->what);
	}
	ArrayHeader* header;
	int real_size = sizeof(ArrayHeader) + newsize;
	if (!block)
	{
		//new block
		header = lsM_alloc_(L, ls_NULL, info->flag, real_size, info->flag);
		if (!header)
		{
			ALLOC_FAILED;
		}
	}
	else
	{
		//change size
		header = ARRAY_TO_HEADER(block);
		header = lsM_alloc_(L, header, header->n, real_size, info->flag);
		if (!header)
		{
			ls_apicheck(real_size > header->n);
			ALLOC_FAILED;
		}
	}
	header->n = real_size;
	return HEADER_TO_ARRAY(header);
}

void lsM_freearray_(ls_State* L, void* block, int flag)
{
	ls_GlobalState* g = G(L);
	ArrayHeader* header = ARRAY_TO_HEADER(block);
	ls_MemSize n = -header->n;
	void* ret = (*g->alloc)(g->alloc_ud, header, header->n, 0);
	lsM_setmemstat_g(g, flag, n);
	ls_assert(ret == ls_NULL);
}
