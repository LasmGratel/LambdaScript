//Only used in parser.c

//Used in assignment
static void suffixedexp(ls_ParserData* pd, ls_Expr* v);

static void exprlist(ls_ParserData* pd, ls_MultiExprInfo* e)
{
	e->count = 0;
	e->firstmulti = -1;
	e->start = pd->pf->freereg;

	if (pd->ls->current.t == ')' || pd->ls->current.t == ';')
	{
		return;
	}

	ls_Expr ee;
	for (;;)
	{
		expr(pd, &ee);
		++e->count;

		//If no more expression, break.
		//The last expression is processed differently.
		if (current_token() != ',')
			break;
		next_token();//skip ','

		if (lsK_pushtostack(pd, &ee))
			e->firstmulti = pd->pf->freereg - 1;
	}
	lsK_pushtostackmulti(pd, &ee);
}

static void functioncall(ls_ParserData* pd, ls_Expr* v)
{
	lsK_pushtostack(pd, v);

	int last_fr = pd->pf->freereg;
	//Read arguments
	next_token(); //Skip '('
	
	ls_MultiExprInfo me;
	exprlist(pd, &me);
	if (me.firstmulti > -1)
		lsK_finishmultiexpr(pd, me.firstmulti, -1);
	next_token(); //Skip ')'

	ls_Bool multi = ls_FALSE;
	if (pd->ls->current.t == TK_TRIDOT)
	{
		multi = ls_TRUE;
		next_token();
	}
	lsK_makecall(pd, v, multi);
}

static void singlevar(ls_ParserData* pd, ls_Expr* v)
{
	ls_String* var;
	if (pd->ls->current.t == TK_TRIDOT)
		var = pd->namevp;
	else
		var = check_get_identifier();

	//Just let lsYL part to do the search (locals and upvals)
	ls_Bool islocal = ls_FALSE;
	int idx = lsYL_search(pd, var, &islocal);
	if (idx < 0)
	{
		//Not local or upval, so go back to global
		//Get global table
		idx = lsYL_search(pd, pd->nameg, &islocal);
		lsK_makestored(pd, EXP_UPVAL, idx, v);

		//Make indexed
		ls_Expr key;
		lsK_makestrk(pd, var, &key);
		lsK_makeindexed(pd, v, &key);
	}
	else
	{
		lsK_makestored(pd, islocal ? EXP_LOCAL : EXP_UPVAL, idx, v);
	}
}

static void assignment(ls_ParserData* pd, ls_Assignment* lh, int nvars)
{
	if (pd->ls->current.t == '=')
	{
		next_token();//skip '='
		if (nvars == 1)
		{
			//Normal assignment
			ls_Expr right;
			expr(pd, &right);

			//Don't store right to allow get table mode (local a,b;a=b.k;)
			lsK_assign(pd, &lh->v, &right);

			//we need to skip to the end of the statement
			//TODO check the grammar of this part?
			while (pd->ls->current.t != ';')
			{
				next_token();
				if (pd->ls->current.t == TK_EOS)
				{
					semantic_error(pd, "unexpected EOS");
				}
			}

			pd->pf->freereg = pd->pf->locals.nact;
			next_token();//skip ';'
		}
		else
		{
			/*
			ls_Expr right;
			ls_MultiAssignInfo mai;
			lsK_prepmultiassign(pd, &mai);

			//Read to the end, push all values
			do
			{
				expr(pd, &right);
				lsK_pushmultiassign(pd, &mai, &right);
			} while (next_when(',')); //Stop when it's no ','
			check_and_next(';');//So it should be ';'

			//Adjust result to nvars
			lsK_adjustmultiassign(pd, &mai, nvars);

			//Do the assignments
			do
			{
				lsK_getmultiassign(pd, &mai, --nvars, &right);
				lsK_assign(pd, &lh->v, &right);
				lh = lh->prev;
			} while (lh);

			//Reset regs
			pd->pf->freereg = pd->pf->locals.nact;
			*/
			int fr = pd->pf->freereg;
			ls_MultiExprInfo me;
			exprlist(pd, &me);
			check_and_next(';');
			lsK_finishmultiexpr(pd, me.firstmulti, fr + nvars);

			fr = fr + nvars;
			ls_Expr right;
			do
			{
				lsK_getlocalat(pd, --fr, &right);
				lsK_assign(pd, &lh->v, &right);
				lh = lh->prev;
			} while (lh);
		}
	}
	else
	{
		next_token();//skip','

		ls_Assignment lh2;
		lh2.prev = lh;
		suffixedexp(pd, &lh2.v);
		//TODO check conflict
		assignment(pd, &lh2, nvars + 1);
	}
}

static void primaryexp(ls_ParserData* pd, ls_Expr* v)
/* primaryexp -> NAME | '(' expr ')' */
{
	if (pd->ls->current.t == '(')
	{
		next_token();
		expr(pd, v);
		check_and_next(')');
	}
	else
	{
		singlevar(pd, v);
		next_token();
	}
}

static void suffixedexp(ls_ParserData* pd, ls_Expr* v)
/* suffixedexp -> primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
{
	primaryexp(pd, v);
	ls_Expr n;
	while (1)
		{
		switch (pd->ls->current.t)
		{
		case '.':
			next_token();//skip '.'
			lsK_makestrk(pd, check_get_identifier(), &n);
			lsK_makeindexed(pd, v, &n);
			next_token();//skip key
			break;
		case '(':
			functioncall(pd, v);
			break;
		default:
			return;
		}
	}
}

static void simpleexp(ls_ParserData* pd, ls_Expr* v)
/* simpleexp -> NUMBER | STRING | NIL | TRUE | FALSE | ... |
constructor | FUNCTION body | suffixedexp */
{
	switch (pd->ls->current.t)
	{
	case TK_NIL:
		lsK_makenil(pd, v);
		next_token();
		break;
	case TK_TRUE:
		lsK_makebool(pd, ls_TRUE, v);
		next_token();
		break;
	case TK_FALSE:
		lsK_makebool(pd, ls_FALSE, v);
		next_token();
		break;
	case TK_NUMBER:
		lsK_makenumk(pd, pd->ls->current.d.n, v);
		next_token();
		break;
	case TK_STRING:
		lsK_makestrk(pd, pd->ls->current.d.objs, v);
		next_token();
		break;
	default:
		suffixedexp(pd, v);
	}
}

static UnOpr getunopr(int op)
{
	switch (op)
	{
	case '-': return OPR_MINUS;
	case '!': return OPR_NOT;
	case '#': return OPR_LEN;
	default: return OPR_NOUNOPR;
	}
}

static BinOpr getbinopr(int op)
{
	switch (op)
	{
	case '+': return OPR_ADD;
	case '-': return OPR_SUB;
	case '*': return OPR_MUL;
	case '/': return OPR_DIV;
	case '<': return OPR_LT;
	case '>': return OPR_GT;
	case TK_LE: return OPR_LE;
	case TK_GE: return OPR_GE;
	case TK_EQ: return OPR_EQ;
	case TK_NE: return OPR_NE;
	/*
	case TK_NE: return OPR_NE;
	case TK_EQ: return OPR_EQ;
	case '<': return OPR_LT;
	case TK_LE: return OPR_LE;
	case '>': return OPR_GT;
	case TK_GE: return OPR_GE;
	case TK_AND: return OPR_AND;
	case TK_OR: return OPR_OR;
	*/
	default: return OPR_NOBINOPR;
	}
}

static const struct
{
	ls_byte left;  /* left priority for each binary operator */
	ls_byte right; /* right priority */
} priority[] =
{  /* ORDER OPR */
	{ 6, 6 }, { 6, 6 }, { 7, 7 }, { 7, 7 },  /* `+' `-' `*' `/' */

	{ 3, 3 }, { 3, 3 }, { 3, 3 },          /* ==, <, <= */
	{ 3, 3 }, { 3, 3 }, { 3, 3 },          /* !=, >, >= */

//	{ 7, 7 },                            /* '%' *.
//	{ 10, 9 }, { 5, 4 },                 /* ^, .. (right associative) */
//	{ 2, 2 }, { 1, 1 }                   /* and, or */
};

#define UNARY_PRIORITY	8  /* priority for unary operators */


static BinOpr subexp(ls_ParserData* pd, ls_Expr* v, int limit)
/*
** subexpr -> (simpleexp | unop subexpr) { binop subexpr }
** where `binop' is any binary operator with a priority higher than `limit'
*/
{
	UnOpr pre = getunopr(pd->ls->current.t);
	if (pre == OPR_NOUNOPR)
	{
		simpleexp(pd, v);
	}
	else
	{
		next_token();
		subexp(pd, v, UNARY_PRIORITY);
		lsK_makeunopr(pd, pre, v);
	}
	BinOpr opr;
	opr = getbinopr(pd->ls->current.t);
	while (opr != OPR_NOBINOPR && priority[opr].left > limit)
	{
		ls_Expr r;
		next_token();
		BinOpr next = subexp(pd, &r, priority[opr].right);
		lsK_makebinopr(pd, opr, v, &r);
		opr = next;
	}
	return opr;
}

static void expr(ls_ParserData* pd, ls_Expr* v)
{
	subexp(pd, v, 0);
}

static void exprstat(ls_ParserData* pd)
/* exprstat -> func | assignment */
{
	int fr = pd->pf->freereg;
	//only assignment now
	ls_Assignment v;
	suffixedexp(pd, &v.v);
	if (pd->ls->current.t == '=' || pd->ls->current.t == ',')
	{
		v.prev = ls_NULL;
		assignment(pd, &v, 1);
	}
	else if (pd->ls->current.t == ';')
	{
		if (!lsK_issimplecall(pd, &v.v)) //The call instrument is generated in lsK_issimplecall
		{
			semantic_error(pd, "statement should be assignment or function call");
		}
		check_and_next(';');
	}
	ls_assert(pd->pf->freereg == fr);
}
