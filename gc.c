#include "ls.h"
#include "common.h"
#include "object.h"
#include "gc.h"
#include "mem.h"
#include "state.h"

ls_Object* lsC_newobj(ls_State* L, int tt, ls_MemSize sz, ls_Object **list)
{
	ls_Object* obj = lsM_newblock(L, sz);
	obj->gch.tt = tt;
	if (list == ls_NULL)
	{
		list = &G(L)->allgc;
	}
	obj->gch.next = *list;
	*list = obj;
	return obj;
}
