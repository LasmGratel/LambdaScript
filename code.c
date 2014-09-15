#include "ls.h"
#include "common.h"
#include "stream.h"
#include "string.h"
#include "object.h"
#include "lex.h"
#include "parser.h"
#include "code.h"
#include "mem.h"

/* code def */
#define OP_GETTABLE 1
#define OP_SETTABLE 2
#define OP_MOVE     3

#define not_supported_yet() ls_throw(pd->L, LS_ERRRUN, "parsing feature not supported yet")

static Instruction storedcode(ls_StoredExpr* v)
{
	Instruction ret = (v->id) & 127;
	switch (v->k)
	{
	case EXP_CONST:
		return ret;
	case EXP_LOCAL:
		return ret + 128;
	case EXP_UPVAL:
		return ret + 256;
	}
	ls_assert(0);
	return 0;
}

static Instruction gettable(ls_StoredExpr* tab, ls_StoredExpr* key, ls_StoredExpr* store_at)
{
	//Currently only support set to local
	ls_assert(store_at->k == EXP_LOCAL);
	int store = store_at->id & 127;
	Instruction t = storedcode(tab), k = storedcode(key), r = store;
	//t: 9, k: 9, r: 7
	Instruction opcode = OP_GETTABLE;
	return (((((opcode << 7) + r) << 9) + t) << 9) + k;
}

static Instruction settable(ls_StoredExpr* tab, ls_StoredExpr* key, ls_StoredExpr* new_val)
{
	//Currently only support set to local
	ls_assert(new_val->k == EXP_LOCAL);
	int nval = new_val->id & 127;
	Instruction t = storedcode(tab), k = storedcode(key), r = nval;
	//t: 9, k: 9, r: 7
	Instruction opcode = OP_SETTABLE;
	return (((((opcode << 7) + r) << 9) + t) << 9) + k;
}

static Instruction move(ls_StoredExpr* l, ls_StoredExpr* r)
{
	Instruction opcode = OP_MOVE;
	return (((((opcode << 7) + 0) << 9) + storedcode(l)) << 9) + storedcode(r);
}

/* helper */

static void write_code(ls_ParserData* pd, Instruction code)
{
	ls_Proto* f = pd->pf->f;
	lsM_growvector(pd->L, f->code, pd->pf->pc, f->sizecode, Instruction, 10000, "instructions");
	f->code[pd->pf->pc++] = code;
}

static void dogettable(ls_ParserData* pd, ls_Expr* v)
{
	if (v->u.k == EXP_INDEXED)
	{
		ls_StoredExpr stored_expr;
		stored_expr.k = EXP_LOCAL;
		if (v->u.i.tab.k == EXP_LOCAL && v->u.i.tab.id >= pd->pf->locals.n)
		{
			stored_expr.id = v->u.i.tab.id;//store at the position of table
		}
		else
		{
			stored_expr.id = pd->pf->freereg++;
		}
		write_code(pd, gettable(&v->u.i.tab, &v->u.i.key, &stored_expr));
		v->u.s = stored_expr;
	}
}

/* generator api */

void lsK_makestrk(ls_ParserData* pd, ls_String* str, ls_Expr* expr)
{
	//No reuse now
	ls_Proto* f = pd->pf->f;
	int old_size = f->sizek;
	lsM_growvector(pd->L, f->k, pd->pf->nk, f->sizek, ls_Value, 1000, "constants");
	while (f->sizek > old_size)
	{
		ls_Value* v = &f->k[old_size++];
		v->tt = LS_OBJ_NIL;
	}
	expr->u.k = EXP_CONST;//set type: const
	ls_Value* v = &f->k[expr->u.s.id = pd->pf->nk++];//set id
	v->tt = str->s.tt;
	v->v.gc = (ls_Object*) str;
}

void lsK_makestored(ls_ParserData* pd, ls_Expkind k, ls_NLocal id, ls_Expr* expr)
{
	expr->u.k = k;
	expr->u.s.id = id;
}

void lsK_makeindexed(ls_ParserData* pd, ls_Expr* v, ls_Expr* key)
{
	//store table and key
	lsK_storeexpr(pd, v);

	if (!(key->u.k == EXP_CONST || key->u.k == EXP_LOCAL || key->u.k == EXP_UPVAL))
	{
		not_supported_yet();
		return;
	}
	ls_Expr expr;
	expr.u.k = EXP_INDEXED;
	expr.u.i.tab = v->u.s;
	expr.u.i.key = key->u.s;
	//dogettable(pd, &expr);
	*v = expr;
}

void lsK_assign(ls_ParserData* pd, ls_Expr* l, ls_Expr* r)
{
	if (r->u.k == EXP_INDEXED && (l->u.k == EXP_LOCAL || l->u.k == EXP_UPVAL))
	{
		//get table mode
		write_code(pd, gettable(&r->u.i.tab, &r->u.i.key, &l->u.s));
		return;
	}
	else if (r->u.k == EXP_INDEXED)
	{
		//must store it first
		lsK_storeexpr(pd, r);
	}
	if (l->u.k == EXP_INDEXED)
	{
		write_code(pd, settable(&l->u.i.tab, &l->u.i.key, &r->u.s));
	}
	else if (l->u.k == EXP_LOCAL | l->u.k == EXP_UPVAL)
	{
		write_code(pd, move(&l->u.s, &r->u.s));
	}
	else
	{
		not_supported_yet();
	}
}
void lsK_prepmultiassign(ls_ParserData* pd, ls_MultiAssignInfo* info)
{
}
void lsK_pushmultiassign(ls_ParserData* pd, ls_MultiAssignInfo* info, ls_Expr* value)
{
}
void lsK_adjustmultiassign(ls_ParserData* pd, ls_MultiAssignInfo* info, int n)
{
}
void lsK_getmultiassign(ls_ParserData* pd, ls_MultiAssignInfo* info, int i, ls_Expr* expr)
{
}

void lsK_storeexpr(ls_ParserData* pd, ls_Expr* v)
{
	dogettable(pd, v);
}

/* review */
#define HLINE "----------------\n"
#define P_TAB "    "

static void print9(Instruction code)
{
	switch (code >> 7)
	{
	case 0:
		printf(" CONST(%d)", code & ((1 << 7) - 1));
		break;
	case 1:
		printf(" LOCAL(%d)", code & ((1 << 7) - 1));
		break;
	case 2:
		printf(" UPVAL(%d)", code & ((1 << 7) - 1));
		break;
	}
}
static void print7(Instruction code)
{
	printf(" LOCAL(%d)", code);
}
static void print_code(Instruction code)
{
	printf("0x%x ", code);
	Instruction opcode = code >> (7 + 9 + 9);
	Instruction a = (code >> (9 + 9)) & ((1 << 7) - 1);
	Instruction b = (code >> (9)) & ((1 << 9) - 1);
	Instruction c = (code) & ((1 << 9) - 1);
	switch (opcode)
	{
	case OP_MOVE:
		printf("MOVE     ");
		print9(b);
		printf(" :=");
		print9(c);
		break;
	case OP_GETTABLE:
		printf("GET TABLE");
		print7(a);
		printf(" :=");
		print9(b);
		printf(" [");
		print9(c);
		printf(" ]");
		break;
	case OP_SETTABLE:
		printf("SET TABLE");
		print9(b);
		printf(" [");
		print9(c);
		printf(" ] :=");
		print7(a);
		break;
	}
}

void lsK_reviewcode(ls_Proto* p)
{
	printf("\nCode Review\n");
	printf(HLINE);

	//constants
	printf("Constants:\n");
	for (int i = 0; i < p->sizek; ++i)
	{
		switch (p->k[i].tt)
		{
		case LS_OBJ_STRING:
			printf(P_TAB "(%d) %s\n", i, getstr(&p->k[i].v.gc->s));
			break;
		default:
			printf(P_TAB "(%d) <unknown>\n", i);
		}
	}
	printf(HLINE);
	
	//local variables
	printf("Local variables:\n");
	for (int i = 0; i < p->sizelocvars; ++i)
	{
		printf(P_TAB "(%d) %s\n", i, getstr(p->locvars[i].varname));
	}
	printf(HLINE);

	//upvals
	printf("Upvalues:\n");
	for (int i = 0; i < p->sizeupvalues; ++i)
	{
		printf(P_TAB "(%d) %s\n", i, getstr(p->upvalues[i].name));
	}
	printf(HLINE);

	printf("Instructions:\n");
	for (int i = 0; i < p->sizecode; ++i)
	{
		printf(P_TAB);
		print_code(p->code[i]);
		printf("\n");
	}
	printf(HLINE);
}
