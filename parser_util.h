//Only used in parser.c

//TODO finish these functions

static void checklimit(ls_ParserData* pd, int v, int l, const char* what)
{
}

static void semantic_error(ls_ParserData* pd, const char* str)
{
}

#define next_token() (lsX_next(pd->ls))
#define check_current_token(tk) ((void)0)
#define check_get_identifier() (pd->ls->current.d.objs)
