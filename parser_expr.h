//Only used in parser.c

static void singlevar(ls_ParserData* pd, expdesc* v)
{
	ls_String* var = check_get_identifier();

	//Just let lsYL part to do the search (locals and upvals)
	ls_Bool islocal = ls_FALSE;
	int idx = lsYL_search(pd, var, &islocal);
	if (idx < 0)
	{
		//Not local or upval, so go back to global
		//Get global table
		idx = lsYL_search(pd, pd->nameg, &islocal);

		//TODO store name

		//TODO make v as index
	}
	else
	{
		//TODO make v as local/up (see isup)
	}
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
