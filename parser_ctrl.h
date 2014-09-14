//Only used in parser.c

//A simple block (no control)
static void block(ls_ParserData* pd)
{
	next_token();//skip '{'
	ls_Block bl;
	enterblock(pd, &bl, ls_FALSE);
	statlist(pd);
	leaveblock(pd);
	check_and_next('}');
}
