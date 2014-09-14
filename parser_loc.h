//Only used in parser.c
#include "object.h"

//Get LocalVar information from Proto
//Most information is stored in Proto, not in ParseFunc
//index is that of ParseFunc.locals.nact, not that of ls_ParserData.actvarmap.n
//Note that in Proto there're not only active ones so a map is needed (actvarmap)
static ls_LocVar* getlocalfromproto(ls_ParserData* pd, ls_ParseFunc* pf, int index)
{
	ls_NLocal idx = pd->actvarmap.arr[index + pf->locals.offset].idx;
	ls_assert(idx < pf->locals.n);
	return &pf->f->locvars[idx];
}

//Internally used function, to add an local to Proto
//Return new index
static ls_NLocal registerlocalvar(ls_ParserData* pd, ls_String* varname)
{
	ls_ParseFunc* pf = pd->pf;
	ls_Proto* f = pf->f;
	int oldsize = f->sizelocvars;
	lsM_growvector(pd->L, f->locvars, pf->locals.n, f->sizelocvars,
		ls_LocVar, MAX_LOCAL_IN_PROTO, "local variables");
	//If successfully grew, make the name NULL
	while (oldsize < f->sizelocvars) f->locvars[oldsize++].varname = ls_NULL;
	f->locvars[pf->locals.n].varname = varname;
	return pf->locals.n++;
}

//Add information for a new local to Proto and ParseData
//ParseFunc.locals.nact is not changed (which is changed in lsYL_localvisiblestart)
static void lsYL_newlocal(ls_ParserData* pd, ls_String* varname)
{
	ls_NLocal reg = registerlocalvar(pd, varname);
	ls_ParseFunc* pf = pd->pf;
	checklimit(pd, pd->actvarmap.n + 1 - pf->locals.offset,
		MAX_ACTIVE_LOCAL_IN_FUNC, "local variables");
	lsM_growvector(pd->L, pd->actvarmap.arr, pd->actvarmap.n + 1,
		pd->actvarmap.size, ls_Vardesc, MAX_ACTIVE_LOCAL_IN_PARSER, "local variables");
	pd->actvarmap.arr[pd->actvarmap.n++].idx = reg;
}

static void lsYL_localvisiblestart(ls_ParserData* pd, ls_NLocal number)
{
	ls_ParseFunc* pf = pd->pf;
	while (number--)
	{
		getlocalfromproto(pd, pf, (pf->locals.nact)++)->startpc = pf->pc;
	}
}

//Remove some active locals to be not active
//Typically called when leaving (including jumping out of) a block
static void lsYL_localvisibleend(ls_ParserData* pd, ls_NLocal number)
{
	pd->actvarmap.n -= number;
	ls_NInst pc = pd->pf->pc;
	while (number--)
	{
		getlocalfromproto(pd, pd->pf, --(pd->pf->locals.nact))->endpc = pc;
	}
}

//Search varname in the given function (pf). Only search in locals
static int searchlocal(ls_ParserData* pd, ls_ParseFunc* pf, ls_String* varname)
{
	for (int i = pf->locals.nact - 1; i >= 0; --i)
	{
		if (lsS_equal(varname, getlocalfromproto(pd, pf, i)->varname))
		{
			return i;
		}
	}
	return -1;
}

//Create a new upval in Proto and increase the counter in pf
//Similar to lsYL_newlocal
static int newupvalue(ls_ParserData* pd, ls_ParseFunc* pf, ls_String* name, ls_Bool enislocal, ls_NLocal enid)
{
	ls_Proto* f = pf->f;
	int oldsize = f->sizeupvalues;
	checklimit(pd, pf->nupvals + 1, MAX_UPVAL_IN_PROTO, "upvalues");
	lsM_growvector(pd->L, f->upvalues, pf->nupvals, f->sizeupvalues,
		ls_Upvalue, MAX_UPVAL_IN_PROTO, "upvalues");
	while (oldsize < f->sizeupvalues) f->upvalues[oldsize++].name = NULL;
	f->upvalues[pf->nupvals].inlocal = enislocal;
	f->upvalues[pf->nupvals].idx = enid;
	f->upvalues[pf->nupvals].name = name;
	return pf->nupvals++;
}

static int searchvar(ls_ParserData* pd, ls_ParseFunc* pf, ls_String* varname, ls_Bool* islocal)
{
	int idx = searchlocal(pd, pf, varname);
	if (idx >= 0)
	{
		*islocal = ls_TRUE;
		return idx;
	}

	//Try existed upval
	ls_Upvalue* up = pf->f->upvalues;
	for (int i = 0; i < pf->nupvals; i++)
	{
		if (lsS_equal(up[i].name, varname)) return i;
	}

	//Find in enclosing function
	if (pf->prev)
	{
		ls_Bool enislocal = ls_FALSE;
		idx = searchvar(pd, pf->prev, varname, &enislocal);
		if (idx >= 0)
		{
			//Found in enclosing function. Need to add a new upval here in this function.
			idx = newupvalue(pd, pf, varname, enislocal, idx);
			//Return as upval
			return idx;
		}
	}

	//Really can't find
	return -1;
}

//Search a variable (local, upval)
//If it's a local, islocal is set to TRUE. Otherwise, it's NOT SET
//Return the index of local/upval, or -1 if not found.
static int lsYL_search(ls_ParserData* pd, ls_String* varname, ls_Bool* islocal)
{
	return searchvar(pd, pd->pf, varname, islocal);
}
