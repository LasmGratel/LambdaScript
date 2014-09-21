#include "ls.h"
#include "common.h"
#include "object.h"
#include "gc.h"
#include "func.h"

LSI_EXTERN ls_Proto* lsF_newproto(ls_State *L)
{
	ls_Proto* f = &lsC_newobj(L, LS_OBJ_PROTO, sizeof(ls_Proto), ls_NULL)->p;
	f->code = ls_NULL;
	f->sizecode = 0;
	f->locvars = ls_NULL;
	f->sizelocvars = 0;
	f->upvalues = ls_NULL;
	f->sizeupvalues = 0;
	f->k = ls_NULL;
	f->sizek = 0;
	f->p = ls_NULL;
	f->sizep = 0;
	f->numparams = 0;
	f->is_vararg = ls_FALSE;
	//f->k = NULL;
	//f->sizek = 0;
	//f->p = NULL;
	//f->sizep = 0;
	//f->code = NULL;
	//f->cache = NULL;
	//f->sizecode = 0;
	//f->lineinfo = NULL;
	//f->sizelineinfo = 0;
	//f->maxstacksize = 0;
	//f->linedefined = 0;
	//f->lastlinedefined = 0;
	//f->source = NULL;
	return f;
}
