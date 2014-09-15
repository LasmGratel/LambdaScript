//Only used in parser.c

//Used in assignment
static void suffixedexp(ls_ParserData* pd, ls_Expr* v);

static void singlevar(ls_ParserData* pd, ls_Expr* v)
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
			//TODO should we check the grammar of this part?
			while (pd->ls->current.t != ';')
			{
				next_token();
				if (pd->ls->current.t == TK_EOS)
				{
					semantic_error(pd, "unexpected EOS");
				}
			}
			next_token();//skip ';'
		}
		else
		{
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
		not_supported_yet();
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
	default:
		suffixedexp(pd, v);
	}
}

static void subexp(ls_ParserData* pd, ls_Expr* v, int limit)
/*
** subexpr -> (simpleexp | unop subexpr) { binop subexpr }
** where `binop' is any binary operator with a priority higher than `limit'
*/
{
	//no op supported
	simpleexp(pd, v);
}

static void expr(ls_ParserData* pd, ls_Expr* v)
{
	subexp(pd, v, 0);
}

static void exprstat(ls_ParserData* pd)
/* exprstat -> func | assignment */
{
	//only assignment now
	ls_Assignment v;
	suffixedexp(pd, &v.v);
	if (pd->ls->current.t == '=' || pd->ls->current.t == ',')
	{
		v.prev = ls_NULL;
		assignment(pd, &v, 1);
	}
	else
	{
		not_supported_yet();
	}
	//TODO balance stack
}
