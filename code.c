#include "ls.h"
#include "common.h"
#include "stream.h"
#include "string.h"
#include "object.h"
#include "lex.h"
#include "parser.h"
#include "code.h"
#include "mem.h"

#define not_supported_yet() ls_throw(pd->L, LS_ERRRUN, "parsing feature not supported yet")

#define expr_t(e) ((e)->u.k)
#define expr_key(e) (&(e)->u.i.key)
#define expr_keyid(e) (expr_key(e)->id)
#define expr_keyt(e) (expr_key(e)->k)
#define expr_tab(e) (&(e)->u.i.tab)
#define expr_tabid(e) (expr_tab(e)->id)

#define expr_s(e) (&(e)->u.s)
#define expr_c(e) (&(e)->u.c)
#define expr_ntf(e) (&(e)->u.n)

#define sexpr_t(s) ((s)->k)
#define sexpr_id(s)    ((s)->id)
#define cexpr_id(c) ((c)->p)

#define expr_is_stored(s)  (expr_t(s) == EXP_LOCAL || expr_t(s) == EXP_TEMP || expr_t(s) == EXP_UPVAL || expr_t(s) == EXP_CONST || expr_t(s) == EXP_NTF)
#define expr_is_stack(s)   (expr_t(s) == EXP_LOCAL || expr_t(s) == EXP_TEMP)
#define expr_is_temp(s)    (expr_t(s) == EXP_TEMP)

#define expr_is_unavailable(s) (expr_t(s) == EXP_UNAVAILABLE)
#define expr_is_indexed(s) (expr_t(s) == EXP_INDEXED)
#define expr_is_closure(s) (expr_t(s) == EXP_CLOSURE)


#define sexpr_is_stored(s) (sexpr_t(s) == EXP_LOCAL || sexpr_t(s) == EXP_TEMP || sexpr_t(s) == EXP_UPVAL || sexpr_t(s) == EXP_CONST)
#define sexpr_is_stack(s)  (sexpr_t(s) == EXP_LOCAL || sexpr_t(s) == EXP_TEMP)
#define sexpr_is_temp(s)   (sexpr_t(s) == EXP_TEMP)

/* code */
#define OP_GETTABLE      1 //799 to table key
#define OP_SETTABLE      2 //799 val table key
#define OP_MOVE          3 //799 0 to from
#define OP_JUMP          4 //799 closeup ? ?
#define OP_CLOSURE       5 //799 protoid resultto ?
#define OP_FILL          6 //799 tostack ? ?

#define make_op_7799(a, b, c, d) ((((((a << 7) + b) << 9) + c) << 9) + d)

#define new_temp_id() (pd->pf->freereg++)

#define storedcode7(v) ((v->id) & 127)
#define storedcode9_q(k) (((k)==EXP_CONST) ? 0 : ((k)==EXP_LOCAL || (k)==EXP_TEMP) ? 128 : (k)==EXP_UPVAL ? 256 : /* EXP_NTF */(128+256) )
#define storedcode9(v) (storedcode7(v) + storedcode9_q(sexpr_t(v)))


static Instruction gettable(ls_StoredExpr* tab, ls_StoredExpr* key, ls_StoredExpr* store_at)
{
	//Currently only support get to stack due to the length of an instrument
	ls_assert(sexpr_is_stack(store_at));
	return make_op_7799(OP_GETTABLE, storedcode7(store_at), storedcode9(tab), storedcode9(key));
}

static Instruction settable(ls_StoredExpr* tab, ls_StoredExpr* key, ls_StoredExpr* new_val)
{
	//Currently only support set to stack
	ls_assert(sexpr_is_stack(new_val));
	return make_op_7799(OP_SETTABLE, storedcode7(new_val), storedcode9(tab), storedcode9(key));
}

static Instruction move(ls_StoredExpr* l, ls_StoredExpr* r)
{
	return make_op_7799(OP_MOVE, 0, storedcode9(l), storedcode9(r));
}

static Instruction makeclosure(ls_ClosureExpr* cl, ls_StoredExpr* store_at)
{
	return make_op_7799(OP_CLOSURE, cexpr_id(cl), storedcode9(store_at), 0);
}

/* helper */

//Write an instruction to the next pc
static void write_code(ls_ParserData* pd, Instruction code)
{
	ls_Proto* f = pd->pf->f;
	lsM_growvector(pd->L, f->code, pd->pf->pc, f->sizecode, Instruction, MAX_INST_IN_PROTO, "instructions");
	f->code[pd->pf->pc++] = code;
}

//If e is temp, try to pop it.
//Will raise error if it's a temp but not on top of the stack.
static void pop_temp(ls_ParserData* pd, ls_StoredExpr* e)
{
	if (sexpr_is_temp(e))
	{
		ls_assert(sexpr_id(e) == pd->pf->freereg - 1);
		--pd->pf->freereg;
		e->k = EXP_UNAVAILABLE;
	}
}

//Make an indexed expr stored
static void dogettable(ls_ParserData* pd, ls_Expr* v)
{
	ls_assert(expr_is_indexed(v));

	ls_StoredExpr stored_expr;
	sexpr_t(&stored_expr) = EXP_TEMP;
	if (sexpr_is_temp(expr_tab(v)))
	{
		//Table is temp, store at the position of table
		sexpr_id(&stored_expr) = expr_tabid(v);
	}
	else
	{
		//We must create a new temp
		sexpr_id(&stored_expr) = new_temp_id();
	}
	//Pop key if it's temp
	pop_temp(pd, expr_key(v));
	//Write code to calculate the value
	write_code(pd, gettable(expr_tab(v), expr_key(v), &stored_expr));
	*expr_s(v) = stored_expr;
}

static void domakeclosure(ls_ParserData* pd, ls_Expr* v)
{
	ls_assert(expr_is_closure(v));
	ls_StoredExpr e;
	sexpr_t(&e) = EXP_TEMP;
	sexpr_id(&e) = new_temp_id();
	
	write_code(pd, makeclosure(expr_c(v), &e));
	*expr_s(v) = e;
}

/* generator api */

void lsK_makenil(ls_ParserData* pd, ls_Expr* expr)
{
	expr_t(expr) = EXP_NTF;
	expr_ntf(expr)->id = EXP_NTF_NIL;
}

void lsK_makebool(ls_ParserData* pd, ls_Bool b, ls_Expr* expr)
{
	expr_t(expr) = EXP_NTF;
	expr_ntf(expr)->id = b ? EXP_NTF_TRUE : EXP_NTF_FALSE;
}

void lsK_makestrk(ls_ParserData* pd, ls_String* str, ls_Expr* expr)
{
	//TODO reuse string
	ls_Proto* f = pd->pf->f;
	int old_size = f->sizek;
	lsM_growvector(pd->L, f->k, pd->pf->nk, f->sizek, ls_Value, MAX_CONST_IN_PROTO, "constants");
	while (f->sizek > old_size)
	{
		f->k[old_size++].tt = LS_OBJ_NIL;
	}
	expr_t(expr) = EXP_CONST;
	sexpr_id(expr_s(expr)) = pd->pf->nk++;

	ls_Value* v = &f->k[sexpr_id(expr_s(expr))];
	v->tt = str->s.tt;
	v->v.gc = (ls_Object*) str;
}

void lsK_makenumk(ls_ParserData* pd, ls_Number num, ls_Expr* expr)
{
	ls_Proto* f = pd->pf->f;
	int old_size = f->sizek;
	lsM_growvector(pd->L, f->k, pd->pf->nk, f->sizek, ls_Value, MAX_CONST_IN_PROTO, "constants");
	while (f->sizek > old_size)
	{
		f->k[old_size++].tt = LS_OBJ_NIL;
	}
	expr_t(expr) = EXP_CONST;
	sexpr_id(expr_s(expr)) = pd->pf->nk++;

	ls_Value* v = &f->k[sexpr_id(expr_s(expr))];
	v->tt = LS_OBJ_NUMBER;
	v->v.n = num;
}

void lsK_makestored(ls_ParserData* pd, ls_Expkind k, ls_NLocal id, ls_Expr* expr)
{
	expr_t(expr) = k;
	sexpr_id(expr_s(expr)) = id;
}

void lsK_makeindexed(ls_ParserData* pd, ls_Expr* v, ls_Expr* key)
{
	//Table or key should be stored
	ls_assert(expr_is_stored(v) || expr_is_stored(key));

	//Calculate table and key first
	lsK_storeexpr(pd, v);
	lsK_storeexpr(pd, key);

	ls_Expr expr;
	expr_t(&expr) = EXP_INDEXED;
	*expr_tab(&expr) = *expr_s(v);
	*expr_key(&expr) = *expr_s(key);

	*v = expr;
	expr_t(key) = EXP_UNAVAILABLE;
}

void lsK_assign(ls_ParserData* pd, ls_Expr* l, ls_Expr* r)
{
	if (expr_is_stack(l))
	{
		//Currently assign to stack is much more easier. (get table, make closure)
		switch (expr_t(r))
		{
		case EXP_INDEXED:
			write_code(pd, gettable(expr_tab(r), expr_key(r), expr_s(l)));
			return;
		case EXP_CLOSURE:
			write_code(pd, makeclosure(expr_c(r), expr_s(l)));
			return;
		}
	}
	
	//Or must store it first
	lsK_storeexpr(pd, r);
	
	switch (expr_t(l))
	{
	case EXP_INDEXED:
		//settable use 7 bit for r, so need to stack it first
		lsK_stackexpr(pd, r);
		write_code(pd, settable(expr_tab(l), expr_key(l), expr_s(r)));
		break;
	case EXP_LOCAL:
	case EXP_UPVAL:
		write_code(pd, move(expr_s(l), expr_s(r)));
		break;
	case EXP_NTF:
		//error
	default:
		not_supported_yet();
	}

	//Pop right if it's temp
	pop_temp(pd, expr_s(r));
}

void lsK_prepmultiassign(ls_ParserData* pd, ls_MultiAssignInfo* info)
{
	*info = pd->pf->freereg;
}

void lsK_pushmultiassign(ls_ParserData* pd, ls_MultiAssignInfo* info, ls_Expr* value)
{
	ls_StoredExpr temp;
	sexpr_t(&temp) = EXP_LOCAL;//make it local to be assigned
	sexpr_id(&temp) = new_temp_id();
	lsK_assign(pd, &temp, value);
	sexpr_t(&temp) = EXP_TEMP;
}

void lsK_adjustmultiassign(ls_ParserData* pd, ls_MultiAssignInfo* info, int n)
{
	write_code(pd, make_op_7799(OP_FILL, *info + n, 0, 0));
	pd->pf->freereg = *info + n;
}

void lsK_getmultiassign(ls_ParserData* pd, ls_MultiAssignInfo* info, int i, ls_Expr* expr)
{
	expr_t(expr) = EXP_TEMP;
	sexpr_id(expr_s(expr)) = *info + i;
}

void lsK_storeexpr(ls_ParserData* pd, ls_Expr* v)
{
	switch (expr_t(v))
	{
	case EXP_INDEXED:
		dogettable(pd, v);
		break;
	case EXP_CLOSURE:
		domakeclosure(pd, v);
		break;
	case EXP_NTF:
		//do nothing
		break;
	}
}

void lsK_stackexpr(ls_ParserData* pd, ls_Expr* v)
{
	ls_assert(expr_is_stored(v));
	ls_StoredExpr e;
	sexpr_t(&e) = EXP_TEMP;
	if (!expr_is_stack(v))
	{
		sexpr_id(&e) = new_temp_id();
		write_code(pd, move(&e, expr_s(v)));
		*expr_s(v) = e;
	}
}

void lsK_closeupvalue(ls_ParserData* pd, int n)
{
	write_code(pd, make_op_7799(OP_JUMP, n, 0, 0));
}

void lsK_makeclosure(ls_ParserData* pd, int p, ls_Expr* ret)
{
	expr_t(ret) = EXP_CLOSURE;
	cexpr_id(expr_c(ret)) = p;
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
		printf(" STACK(%d)", code & ((1 << 7) - 1));
		break;
	case 2:
		printf(" UPVAL(%d)", code & ((1 << 7) - 1));
		break;
	case 3:
		switch (code & ((1 << 7) - 1))
		{
		case EXP_NTF_NIL:
			printf(" NIL     ");
			break;
		case EXP_NTF_TRUE:
			printf(" TRUE    ");
			break;
		case EXP_NTF_FALSE:
			printf(" FALSE   ");
			break;
		}
		break;
	}
}
static void print7(Instruction code)
{
	printf(" STACK(%d)", code);
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
	case OP_JUMP:
		if (b == 0 && c == 0)
		{
			printf("CLOSE     %d", a);
		}
		break;
	case OP_CLOSURE:
		printf("CLOSURE  ");
		print9(b);
		printf(" :=");
		printf(" PROTO(%d) ", a);
		break;
	case OP_FILL:
		printf("FILL      %d", a);
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
		case LS_OBJ_NUMBER:
			printf(P_TAB "(%d) %f\n", i, p->k[i].v.n);
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

	printf("Subfunctions:\n");
	for (int i = 0; i < p->sizep; ++i)
	{
		printf(P_TAB "(%d) START_SUBFUNCION\n", i);
		lsK_reviewcode(p->p[i]);
		printf(P_TAB "(%d) END_SUBFUNCTION\n", i);
	}
}
