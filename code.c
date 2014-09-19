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
#define expr_is_funcioncall(s) (expr_t(s) == EXP_CALL_SINGLERET || expr_t(s) == EXP_CALL_LISTRET || expr_t(s) == EXP_CALL_MULTIRET)

#define expr_f(e) (&(e)->u.f.f)


#define sexpr_is_stored(s) (sexpr_t(s) == EXP_LOCAL || sexpr_t(s) == EXP_TEMP || sexpr_t(s) == EXP_UPVAL || sexpr_t(s) == EXP_CONST)
#define sexpr_is_stack(s)  (sexpr_t(s) == EXP_LOCAL || sexpr_t(s) == EXP_TEMP)
#define sexpr_is_temp(s)   (sexpr_t(s) == EXP_TEMP)


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

static Instruction callfunction(ls_StoredExpr* func, int subtype)
{
	ls_assert(sexpr_t(func) == EXP_TEMP);
	return make_op_7799(OP_CALL, subtype, storedcode9(func), 0);
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

static void docallfunction(ls_ParserData* pd, ls_Expr* v)
{
	ls_assert(expr_is_funcioncall(v) && sexpr_t(expr_f(v)) == EXP_TEMP);
	write_code(pd, callfunction(expr_f(v), OP_CALL_SUBTYPE(expr_t(v))));
	//Set v as EXP_TEMP
	ls_StoredExpr func = *expr_f(v);
	*expr_s(v) = func;
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
	//Note that table or key maybe functioncall (can only be single returned).
	//In this case, storeexpr here will push the only value to stack.
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
	//If list call is used in assignment, only the first return value is used.
	//So we must convert it first
	if (expr_t(r) == EXP_CALL_LISTRET)
		expr_t(r) = EXP_CALL_SINGLERET;

	if (expr_is_stack(l))
	{
		//Currently assign to stack is much more easier. (get table, make closure)
		switch (expr_t(r))
		{
		case EXP_INDEXED:
			write_code(pd, gettable(expr_tab(r), expr_key(r), expr_s(l)));
			//TODO pop temp?
			return;
		case EXP_CLOSURE:
			write_code(pd, makeclosure(expr_c(r), expr_s(l)));
			//TODO pop temp?
			return;
		//No functioncall here.
		//Functioncall can't be directly assigned to a certain stack (need to be called first). Store and move.
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
	case EXP_CALL_LISTRET:
		//Convert to single first
		//list is only supported in lsK_pushtostack
		expr_t(v) = EXP_CALL_SINGLERET;
	case EXP_CALL_SINGLERET:
		docallfunction(pd, v);
		break;
	case EXP_CALL_MULTIRET:
		//Can not be stored
	case EXP_UNAVAILABLE:
		ls_assert(0);
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

ls_Bool lsK_issimplecall(ls_ParserData* pd, ls_Expr* e)
{
	if (expr_is_funcioncall(e))
	{
		//The return is not even used. So use the easiest SINGLERET 
		expr_t(e) = EXP_CALL_SINGLERET;
		lsK_pushtostack(pd, e);
		pop_temp(pd, expr_s(e));
		return ls_TRUE;
	}
	return ls_FALSE;
}

ls_Bool lsK_pushtostack(ls_ParserData* pd, ls_Expr* e)
{
	if (expr_is_funcioncall(e))
	{
		ls_assert(sexpr_t(expr_f(e)) == EXP_TEMP);
		//Function call is processed separatedly
		write_code(pd, callfunction(expr_f(e), OP_CALL_SUBTYPE(expr_t(e))));
		//Set e to temp
		ls_Bool ret = expr_t(e) == EXP_CALL_LISTRET;
		ls_StoredExpr stack;
		stack = *expr_f(e);
		*expr_s(e) = stack;
		return ret;
	}

	if (expr_t(e) != EXP_TEMP || sexpr_id(expr_s(e)) != pd->pf->freereg)
	{
		ls_Expr stack;
		expr_t(&stack) = EXP_LOCAL; //Set to local to allow assignment
		sexpr_id(expr_s(&stack)) = new_temp_id();

		lsK_assign(pd, &stack, e);
		sexpr_id(expr_s(e)) = sexpr_id(expr_s(&stack));
		expr_t(e) = EXP_TEMP;
	}
	return ls_FALSE;
}

ls_Bool lsK_pushtostackmulti(ls_ParserData* pd, ls_Expr* e)
{
	if (expr_t(e) == EXP_CALL_LISTRET)
		expr_t(e) = EXP_CALL_MULTIRET;
	lsK_pushtostack(pd, e);
	return ls_FALSE;
}

void lsK_finishmultiexpr(ls_ParserData* pd, int expand_from, int fill_to)
{
	int real_fill_to = fill_to > pd->pf->freereg ? fill_to : -1;
	if (expand_from > -1 || real_fill_to > -1)
		write_code(pd, make_op_7799(OP_EXPANDFILL, expand_from + 1, real_fill_to + 1, 0));
	if (fill_to > -1)
		pd->pf->freereg = fill_to;
}

void lsK_getlocalat(ls_ParserData* pd, int pos, ls_Expr* expr)
{
	expr_t(expr) = EXP_TEMP;
	sexpr_id(expr_s(expr)) = pos;
}

void lsK_makecall(ls_ParserData* pd, ls_Expr* func, ls_Bool is_multi)
{
	ls_Expr c;
	expr_t(&c) = is_multi ? EXP_CALL_LISTRET : EXP_CALL_SINGLERET;
	ls_assert(expr_is_temp(func));
	*expr_f(&c) = *expr_s(func);
	*func = c;
	//Now the arguments are already poped
	pd->pf->freereg = sexpr_id(expr_f(func)) + 1;
}
