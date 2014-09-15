#ifndef LS_CODE_H
#define LS_CODE_H

/* Expression and assignment basic */
//Make a string const into expr.
LSI_EXTERN void lsK_makestrk(ls_ParserData* pd, ls_String* str, ls_Expr* expr);
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

LSI_EXTERN void lsK_reviewcode(ls_Proto* p);

#endif