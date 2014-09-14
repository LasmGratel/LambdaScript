#include "ls.h"
#include "common.h"
#include "mem.h"
#include "stream.h"
#include "lex.h"
#include "table.h"
#include "object.h"
#include "parser.h"
#include "code.h"
#include "string.h"
#include "func.h"

static void expr(ls_ParserData* pd, ls_Expr* v);
static void statlist(ls_ParserData* pd);
static void enterblock(ls_ParserData* pd, ls_Block* bl, ls_byte isloop);
static void leaveblock(ls_ParserData* pd);
static void enterfunc(ls_ParserData* pd, ls_ParseFunc* pf);
static void leavefunc(ls_ParserData* pd);

#include "parser_util.h"
#include "parser_loc.h"
#include "parser_expr.h"
#include "parser_ctrl.h"

static void funcbody(ls_ParserData* pd)
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

	statlist(pd);

	leaveblock(pd);
	check_and_next('}');
	leavefunc(pd);

	//Always end with ';'
	check_and_next(';');
}

static void localfunc(ls_ParserData* pd)
{
	next_token();//skip 'func'
	ls_String* name = check_get_identifier();
	next_token();//skip name
	//Just read the body
	funcbody(pd);
}

static void stat(ls_ParserData* pd)
{
	ls_LexState* ls = pd->ls;
	switch (ls->current.t)
	{
	case TK_VAR:
		lsX_next(ls);
		if (ls->current.t == TK_IDENTIFIER)
		{
			//newlocalvar(pd, ls->current.d.objs);
			lsYL_newlocal(pd, check_get_identifier());
		}
		lsX_next(ls);
		if (ls->current.t == ';')
		{
			lsX_next(ls);
		}
		//adjustlocalvars(pd, 1);//add 1 new local
		lsYL_localvisiblestart(pd, 1);
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

static void statlist(ls_ParserData* pd)
{
	while (pd->ls->current.t != TK_EOS && pd->ls->current.t != '}')
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

	//Other tasks
	//Locals
	pf->locals.n = 0;
	pf->locals.nact = 0;
	pf->locals.offset = pd->actvarmap.n;

	//Upvals
	pf->nupvals = 0;
}

static void leavefunc(ls_ParserData* pd)
{
	//Function chain
	ls_ParseFunc* pf = pd->pf;
	pd->pf = pf->prev;

	//Other tasks
	//Locals
	//Don't need to change actvarmap in ParserData, which are already
	//released in leaveblock (by lsYL_localvisibleend)
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
}

static void leaveblock(ls_ParserData* pd)
{
	ls_ParseFunc* pf = pd->pf;
	ls_Block* bl = pf->bl;

	//Block chain
	pf->bl = bl->previous;

	//Other tasks
	//Locals
	lsYL_localvisibleend(pd, pf->locals.nact - bl->nactvar); //Remove some locals
	//TODO parse close local
}

static void mainfunc(ls_ParserData* pd)
{
	//Stack objects
	ls_ParseFunc pf;
	ls_Block bl;

	//Parse function and main block
	enterfunc(pd, &pf);
	enterblock(pd, &bl, ls_FALSE);

	statlist(pd);

	leaveblock(pd);
	leavefunc(pd);
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
	mainfunc(&pd);

	//Stream should end
	check_current_token(TK_EOS);
}
