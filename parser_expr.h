//Only used in parser.c

#define UNARY_PRIORITY	8  /* priority for unary operators */

static UnOpr getunopr(int op)
{
	switch (op) {
	case '!': return OPR_NOT;
	case '-': return OPR_MINUS;
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
		case '%': return OPR_MOD;
		case '^': return OPR_POW;
		//case TK_CONCAT: return OPR_CONCAT;
		//case TK_NE: return OPR_NE;
		//case TK_EQ: return OPR_EQ;
		case '<': return OPR_LT;
		//case TK_LE: return OPR_LE;
		case '>': return OPR_GT;
		//case TK_GE: return OPR_GE;
		//case TK_AND: return OPR_AND;
		//case TK_OR: return OPR_OR;
		default: return OPR_NOBINOPR;
	}
}

static const struct
{
	ls_byte left;  /* left priority for each binary operator */
	ls_byte right; /* right priority */
} priority[] = 
{  /* ORDER OPR */
	{ 6, 6 }, { 6, 6 }, { 7, 7 }, { 7, 7 }, { 7, 7 },  /* `+' `-' `*' `/' `%' */
	{ 10, 9 }, { 5, 4 },                 /* ^, .. (right associative) */
	{ 3, 3 }, { 3, 3 }, { 3, 3 },          /* ==, <, <= */
	{ 3, 3 }, { 3, 3 }, { 3, 3 },          /* ~=, >, >= */
	{ 2, 2 }, { 1, 1 }                   /* and, or */
};

static void singlevar(ls_ParserData* pd, expdesc* v)
{
	ls_String* var = check_get_identifier();
	lsYL_search(pd, var);
}

static void assignment(ls_ParserData* pd, ls_Assignment* lh, int nvars)
{
	//skip '='
	next_token();
	expdesc v;
	expr(pd, &v);
	next_token();//skip ';'
	//store!
}

static void primaryexp(ls_ParserData* pd, expdesc* v)
/* primaryexp -> NAME | '(' expr ')' */
{
	if (pd->ls->current.t == '(')
	{
		next_token();
		expr(pd, v);

		//check_match
		ls_assert(pd->ls->current.t == ')');
		next_token();

		//TODO discharge
	}
	else
	{
		singlevar(pd, v);
	}
}

static void suffixedexp(ls_ParserData* pd, expdesc* v)
/* suffixedexp ->
primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */
{
	primaryexp(pd, v);
	next_token();
	//other forms are not supported yet
}

static void simpleexp(ls_ParserData* pd, expdesc* v)
/* simpleexp -> NUMBER | STRING | NIL | TRUE | FALSE | ... |
constructor | FUNCTION body | suffixedexp */
{
	switch (pd->ls->current.t)
	{
	default:
		suffixedexp(pd, v);
	}
}

static void subexp(ls_ParserData* pd, expdesc* v, int limit)
/*
** subexpr -> (simpleexp | unop subexpr) { binop subexpr }
** where `binop' is any binary operator with a priority higher than `limit'
*/
{
	//no op supported
	simpleexp(pd, v);
}

static void expr(ls_ParserData* pd, expdesc* v)
{
	subexp(pd, v, 0);
}

//stat part

static void exprstat(ls_ParserData* pd)
{
	//only assignment now
	ls_Assignment v;
	suffixedexp(pd, &v.v);
	if (pd->ls->current.t == '=')//no multiple assignment supported yet, so no ','
	{
		v.prev = ls_NULL;
		assignment(pd, &v, 1);
	}
	else
	{
		//check if its a VCALL
		//call func
	}
}
