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

#define MAXVARS		200

typedef struct ls_Assignment
{
	struct ls_Assignment *prev;
	expdesc v;
} ls_Assignment;

typedef struct ls_Block
{
	struct ls_Block* previous;  /* chain */
	short firstlabel;  /* index of first label in this block */
	short firstgoto;  /* index of first pending goto in this block */
	ls_byte nactvar;  /* # active locals outside the block */
	ls_byte upval;  /* true if some variable in the block is an upvalue */
	ls_byte isloop;  /* true if `block' is a loop */
} ls_Block;


static void init_exp(expdesc *e, ls_Expkind k, int i)
{
	//e->f = e->t = NO_JUMP;//TODO patch list
	e->k = k;
	e->u.info = i;
}

static void expr(ls_ParserData* pd, expdesc* v);

#include "parser_util.h"
#include "parser_loc.h"
#include "parser_expr.h"

static void stat(ls_ParserData* pd)
{
	ls_LexState* ls = pd->ls;
	switch (ls->current.t)
	{
	case TK_VAR:
		lsX_next(ls);
		if (ls->current.t == TK_IDENTIFIER)
		{
			newlocalvar(pd, ls->current.d.objs);
		}
		lsX_next(ls);
		if (ls->current.t == ';')
		{
			lsX_next(ls);
		}
		adjustlocalvars(pd, 1);//add 1 new local
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

static void open_func(ls_ParserData* pd, ls_ParseFunc* pf, ls_Block* bl)
{
	pf->prev = pd->pf;
	pd->pf = pf;

	pf->pc = 0;
	pf->firstlocal = 0;
	pf->nlocvars = 0;
	pf->nactvar = 0;
	ls_Proto* f = pf->f;
	//f->source = ls->source;
	//f->maxstacksize = 2;  /* registers 0/1 are always valid */
	//anchor table
	//enter block
}

static void close_func(ls_ParserData* pd)
{
	//ls_LexState* ls;
	//final ret
	//leave block
	ls_ParseFunc* pf = pd->pf;
	pd->pf = pf->prev;
}

static void enterblock(ls_ParseFunc* pf, ls_Block* bl, ls_byte isloop)
{
	bl->isloop = isloop;
	bl->nactvar = pf->nactvar;
	bl->firstlabel = pf->pd->label.n;
	bl->firstgoto = pf->pd->gt.n;
	bl->upval = 0;
	bl->previous = pf->bl;
	pf->bl = bl;
	ls_assert(pf->freereg == pf->nactvar);
}

static void leaveblock(ls_ParseFunc* pf) {
	ls_Block* bl = pf->bl;
	if (bl->previous && bl->upval) {
		/* create a 'jump to here' to close upvalues */
		//int j = luaK_jump(pf);
		//luaK_patchclose(pf, j, bl->nactvar);
		//luaK_patchtohere(pf, j);
	}
	//if (bl->isloop)
	//	breaklabel(pf);  /* close pending breaks */
	pf->bl = bl->previous;
	//removevars(pf, bl->nactvar);
	ls_assert(bl->nactvar == pf->nactvar);
	pf->freereg = pf->nactvar;  /* free registers */
	pf->pd->label.n = bl->firstlabel;  /* remove local labels */
	//if (bl->previous)  /* inner block? */
	//	movegotosout(pf, bl);  /* update pending gotos to outer block */
	//else if (bl->firstgoto < pf->pd->gt.n)  /* pending gotos in outer block? */
	//	undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);  /* error */
}

void lsY_rawparse(ls_State* L, ls_LexState* zin)
{
	ls_ParserData pd;
	ls_ParseFunc pf;

	pd.actvar.arr = ls_NULL;
	pd.actvar.n = pd.actvar.size = 0;
	pd.pf = &pf;
	pd.L = L;
	pd.ls = zin;

	ls_Block bl;
	pf.f = lsF_newproto(L);
	pf.ls = zin;

	//main func
	open_func(&pd, &pf, &bl);
	//TODO setup var arg
	//TODO setup global (as upval)
	lsX_next(zin);
	statlist(&pd);
	check_current_token(TK_EOS);
	close_func(&pd);
}
