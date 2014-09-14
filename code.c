#include "ls.h"
#include "common.h"
#include "stream.h"
#include "string.h"
#include "object.h"
#include "lex.h"
#include "parser.h"
#include "code.h"

void lsK_makestrk(ls_ParserData* pd, ls_String* str, ls_Expr* expr)
{
	printf("STR CONST %s\n", getstr(str));
}
void lsK_makestored(ls_ParserData* pd, ls_Expkind k, ls_NLocal id, ls_Expr* expr)
{
	printf("STORE %d %d\n", k, id);
}
void lsK_makeindexed(ls_ParserData* pd, ls_Expr* v, ls_Expr* key)
{
	printf("GET TABLE\n");
}
void lsK_assign(ls_ParserData* pd, ls_Expr* l, ls_Expr* r)
{
	printf("ASSIGN\n");
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