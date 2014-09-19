#include "ls.h"
#include "common.h"
#include "mem.h"
#include "stream.h"
#include "lex.h"
#include "object.h"
#include "parser.h"
#include "code.h"
#include "string.h"
#include "func.h"
#include "review.h"

static void expr(ls_ParserData* pd, ls_Expr* v);
static void statlist(ls_ParserData* pd, ls_uint16 f);
static void enterblock(ls_ParserData* pd, ls_Block* bl, ls_byte isloop);
static void leaveblock(ls_ParserData* pd);
static void enterfunc(ls_ParserData* pd, ls_ParseFunc* pf);
static ls_Proto* leavefunc(ls_ParserData* pd);

#include "parser_util.h"
#include "parser_loc.h"
#include "parser_expr.h"
#include "parser_ctrl.h"

static void funcbody(ls_ParserData* pd, ls_Expr* v)
{
	//Stack objects
	ls_ParseFunc pf;
	ls_Block bl;

	//Enter func to declare parameters
	enterfunc(pd, &pf);

	//Read parlist
	check_and_next('(');
	//Currently don't support parameters
	check_and_next(')');

	check_and_next('{');
	//Now enter block
	enterblock(pd, &bl, ls_FALSE);

	statlist(pd, '}');

	leaveblock(pd);
	check_and_next('}');
	
	ls_Proto* fn = leavefunc(pd);

	//Always end with ';'
	check_and_next(';');

	//Add f to Proto list
	ls_Proto* fo = pd->pf->f;

	int old_size = fo->sizep;
	lsM_growvector(pd->L, fo->p, pd->pf->np, fo->sizep, ls_Proto*, MAX_PROTO_IN_PROTO, "local functions");
	while (old_size < fo->sizep)
	{
		fo->p[old_size++] = ls_NULL;
	}

	int p = pd->pf->np++;
	fo->p[p] = fn;
	lsK_makeclosure(pd, p, v);
}

static void localfunc(ls_ParserData* pd)
{
	next_token();//skip 'func'
	ls_String* name = check_get_identifier();

	//Create local var
	ls_Expr var;
	lsYL_newlocal(pd, name, &var);
	lsYL_localvisiblestart(pd, 1);

	next_token();//skip name

	ls_Expr func;
	
	//Read the body
	funcbody(pd, &func);

	//Assign
	lsK_assign(pd, &var, &func);
}

static void localvarassign(ls_ParserData* pd, ls_Assignment* assign, int n)
{
	ls_Assignment newvar;
	switch (pd->ls->current.t)
	{
	case ';':
		lsYL_localvisiblestart(pd, n);
		//No assignment
		break;
	case ',':
		//New var
		next_token();
		newvar.prev = assign;
		lsYL_newlocal(pd, check_get_identifier(), &newvar.v);
		next_token();
		localvarassign(pd, &newvar, n + 1);
		break;
	case '=':
		if (n == 1)
		{
			//Simple assignment
			next_token();
			ls_Expr r;
			expr(pd, &r);
			lsK_assign(pd, &assign->v, &r);

			//The end of the statement
			check_and_next(';');
			return;
		}
		else
		{
			/*
			//Do multiassignment
			next_token();
			ls_Expr r;
			ls_MultiAssignInfo info;

			lsK_prepmultiassign(pd, &info);
			do
			{
				expr(pd, &r);
				lsK_pushmultiassign(pd, &info, &r);
			} while (next_when(','));
			check_and_next(';');

			lsK_adjustmultiassign(pd, &info, n);

			int index = n;
			do
			{
				lsK_getmultiassign(pd, &info, --index, &r);
				lsK_assign(pd, &assign->v, &r);
				assign = assign->prev;
			} while (assign);

			pd->pf->freereg = pd->pf->locals.nact;

			//Set local visible
			lsYL_localvisiblestart(pd, n);
			return;
			*/
			assignment(pd, assign, n);
			lsYL_localvisiblestart(pd, n);

			return;
		}
	}
}

static void localvar(ls_ParserData* pd)
{
	ls_LexState* ls = pd->ls;
	next_token();

	ls_assert(ls->current.t == TK_IDENTIFIER);

	//ls_Expr var;
	ls_Assignment assign;
	assign.prev = ls_NULL;
	lsYL_newlocal(pd, check_get_identifier(), &assign.v);

	next_token();
	localvarassign(pd, &assign, 1);
	/*
	ls_assert(ls->current.t == ';');
	lsX_next(ls);

	ls_Expr nil;
	lsK_makenil(pd, &nil);
	lsK_assign(pd, &var, &nil);

	//Visible after initialization
	lsYL_localvisiblestart(pd, 1);
	*/
}

static void stat(ls_ParserData* pd)
{
	ls_LexState* ls = pd->ls;
	switch (ls->current.t)
	{
	case TK_VAR:
		localvar(pd);
		break;

	case TK_FUNC:
		//Function defination
		//Currently treat all func as local
		localfunc(pd);
		break;

	case '{':
		//New block
		block(pd);
		break;

	default:
		exprstat(pd);
	}
}

static void statlist(ls_ParserData* pd, ls_uint16 f)
{
	while (pd->ls->current.t != f)
	{
		stat(pd);
	}
}

static void enterfunc(ls_ParserData* pd, ls_ParseFunc* pf)
{
	//Function chain
	pf->prev = pd->pf;
	pd->pf = pf;
	
	//Setup function
	pf->bl = ls_NULL;
	pf->f = lsF_newproto(pd->L);
	pf->pc = 0;
	pf->freereg = 0;

	//Other tasks
	//Locals
	pf->locals.n = 0;
	pf->locals.nact = 0;
	pf->locals.offset = pd->actvarmap.n;

	//Upvals
	pf->nupvals = 0;

	//Consts
	pf->nk = 0;

	//Protos
	pf->np = 0;
}

static ls_Proto* leavefunc(ls_ParserData* pd)
{
	//Function chain
	ls_ParseFunc* pf = pd->pf;
	pd->pf = pf->prev;

	//Other tasks

	//Resize vectors
	ls_Proto* f = pf->f;
	ls_State* L = pd->L;
	lsM_reallocvector(L, f->code, f->sizecode, pf->pc, Instruction);
	f->sizecode = pf->pc;
	lsM_reallocvector(L, f->k, f->sizek, pf->nk, ls_Value);
	f->sizek = pf->nk;
	lsM_reallocvector(L, f->p, f->sizep, pf->np, ls_Proto*);
	f->sizep = pf->np;
	lsM_reallocvector(L, f->locvars, f->sizelocvars, pf->locals.n, ls_LocVar);
	f->sizelocvars = pf->locals.n;
	lsM_reallocvector(L, f->upvalues, f->sizeupvalues, pf->nupvals, ls_Upvalue);
	f->sizeupvalues = pf->nupvals;

	return f;
}

//Setup everything when entering a block
//Only need an uninitialized stack object ls_Block
static void enterblock(ls_ParserData* pd, ls_Block* bl, ls_byte isloop)
{
	ls_ParseFunc* pf = pd->pf;

	//Block chain
	bl->previous = pf->bl;
	pf->bl = bl;

	//Other tasks
	//Locals
	bl->nactvar = pf->locals.nact;

	//Upvalue mark
	bl->hasup = 0;
}

static void leaveblock(ls_ParserData* pd)
{
	ls_ParseFunc* pf = pd->pf;
	ls_Block* bl = pf->bl;

	//Block chain
	pf->bl = bl->previous;

	//Other tasks
	//Locals
	int c = pf->locals.nact - bl->nactvar;
	lsYL_localvisibleend(pd, c); //Remove some locals
	pd->pf->freereg -= c;

	//Close upvalues
	if (bl->hasup)//TODO in lua it also check bl->previous?
		lsK_closeupvalue(pd, c);
}

static ls_Proto* mainfunc(ls_ParserData* pd)
{
	//Stack objects
	ls_ParseFunc pf;
	ls_Block bl;

	//Init function and main block
	enterfunc(pd, &pf);
	enterblock(pd, &bl, ls_FALSE);

	//Add global env
	newupvalue(pd, &pf, pd->nameg, ls_TRUE, 0);

	//Start parsing
	statlist(pd, TK_EOS);

	leaveblock(pd);

	return leavefunc(pd);
}

void lsY_rawparse(ls_State* L, ls_LexState* zin)
{
	//Setup parser data
	ls_ParserData pd;
	pd.actvarmap.arr = ls_NULL;
	pd.actvarmap.n = pd.actvarmap.size = 0;
	pd.pf = ls_NULL;
	pd.L = L;
	pd.ls = zin;

	pd.nameg = lsS_newstr(L, "_G");
	//TODO not attached

	//Start input
	lsX_next(zin);
	//Parse mainfunc
	ls_Proto* ret = mainfunc(&pd);

	//Stream should end
	check_current_token(TK_EOS);

	lsR_reviewcode(ret);
}
