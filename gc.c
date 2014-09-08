#include "ls.h"
#include "common.h"
#include "object.h"
#include "gc.h"
#include "mem.h"

ls_Object* lsC_newobj(ls_State* L, int type, ls_MemSize sz, int usage, ls_Object **list)
{
	ls_Object* obj = lsM_newblock(L, usage, sz);
	//TODO initialization
	return obj;
}
