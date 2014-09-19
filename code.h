#ifndef LS_CODE_H
#define LS_CODE_H

/* Expression and assignment basic */
LSI_EXTERN void lsK_makenil(ls_ParserData* pd, ls_Expr* expr);
LSI_EXTERN void lsK_makebool(ls_ParserData* pd, ls_Bool b, ls_Expr* expr);
//Make a string const into expr.
LSI_EXTERN void lsK_makestrk(ls_ParserData* pd, ls_String* str, ls_Expr* expr);
LSI_EXTERN void lsK_makenumk(ls_ParserData* pd, ls_Number str, ls_Expr* expr);
//Make a StoredExpr. `k` can only be `EXP_LOCAL`, `EXP_UPVAL`, or `EXP_CONST`.
LSI_EXTERN void lsK_makestored(ls_ParserData* pd, ls_Expkind k, ls_NLocal id, ls_Expr* expr);
//Make a indexed expression into expr. v:=v[key].
//Note that `key` may no longer be valid after this.
LSI_EXTERN void lsK_makeindexed(ls_ParserData* pd, ls_Expr* v, ls_Expr* key);
//Do assignment.
LSI_EXTERN void lsK_assign(ls_ParserData* pd, ls_Expr* l, ls_Expr* r);
//Prepare for a multi-assignment. Called before pushing any values with `lsK_pushmultiassign`.
LSI_EXTERN void lsK_prepmultiassign(ls_ParserData* pd, ls_MultiAssignInfo* info);
//Push value on to a multi-assignment stack.
LSI_EXTERN void lsK_pushmultiassign(ls_ParserData* pd, ls_MultiAssignInfo* info, ls_Expr* value);
//Adjust the number of value needed for multi-assignent. Called after pushing all values.
LSI_EXTERN void lsK_adjustmultiassign(ls_ParserData* pd, ls_MultiAssignInfo* info, int n);
//Get the i-th pushed value in a multi-assignment. Return a StoredExpr and can be passed to lsK_assign.
//i starts from 0
LSI_EXTERN void lsK_getmultiassign(ls_ParserData* pd, ls_MultiAssignInfo* info, int i, ls_Expr* expr);
//Make a expr a stored. If it's not calculated it is calculated and stored on stack.
//Note: this function should not be directly called in parser. Let lsK_assign do it automatically.
//lsK will also take care of the stack when doing these.
LSI_EXTERN void lsK_storeexpr(ls_ParserData* pd, ls_Expr* v);

LSI_EXTERN void lsK_stackexpr(ls_ParserData* pd, ls_Expr* v);
//Close n locals and their upvalues
LSI_EXTERN void lsK_closeupvalue(ls_ParserData* pd, int n);

LSI_EXTERN void lsK_makeclosure(ls_ParserData* pd, int p, ls_Expr* ret);
//Return true if the result of `e` may contain multiple values
LSI_EXTERN ls_Bool lsK_isexprmvalue(ls_ParserData* pd, ls_Expr* e);
//Check if e is a call. If it is, call it and clear all return value.
//Used when the statement finishes without '=' (then the expression must be a call).
LSI_EXTERN ls_Bool lsK_issimplecall(ls_ParserData* pd, ls_Expr* e);
//Push the expresssion to THE TOP OF the stack. The temp used by it will be poped
//Return true if e is needed to expand (it's a list-returned function call)
LSI_EXTERN ls_Bool lsK_pushtostack(ls_ParserData* pd, ls_Expr* e);
//Like lsK_pushtostack, but function with multiple return will be called as MULTI instead of LIST
//Always return false
LSI_EXTERN ls_Bool lsK_pushtostackmulti(ls_ParserData* pd, ls_Expr* e);
//func is the function pushed on the stack, and all temps on stack after func are used as arguments
//Must be stacked (and the arguments are poped) before using the stack for other temps
LSI_EXTERN void lsK_makecall(ls_ParserData* pd, ls_Expr* func, ls_Bool is_multi);

LSI_EXTERN void lsK_finishmultiexpr(ls_ParserData* pd, int expand_from, int fill_to);

LSI_EXTERN void lsK_reviewcode(ls_Proto* p);

LSI_EXTERN void lsK_getlocalat(ls_ParserData* pd, int pos, ls_Expr* expr);

#endif
