#ifndef LS_CODE_H
#define LS_CODE_H

/* code */
#define OP_GETTABLE      1 //799 to table key
#define OP_SETTABLE      2 //799 val table key
#define OP_MOVE          3 //799 0 to from
#define OP_JUMP          4 //799 closeup ? ?
#define OP_CLOSURE       5 //799 protoid resultto ?
#define OP_EXPANDFILL    6 //799 expand_from+1 fill_to+1
#define OP_CALL          7 //799 calltype function

#define OP_CALL_SUBTYPE(et) ((et) - EXP_CALL_SINGLERET)

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
//Make an expr a stored. If it's not calculated it is calculated and stored on stack.
//Note: this function should not be directly called in parser. Let lsK_assign do it automatically.
//lsK will also take care of the stack when doing these.
LSI_EXTERN void lsK_storeexpr(ls_ParserData* pd, ls_Expr* v);
//Make a stored expr onto the stack.
LSI_EXTERN void lsK_stackexpr(ls_ParserData* pd, ls_Expr* v);
//Close n locals and their upvalues.
LSI_EXTERN void lsK_closeupvalue(ls_ParserData* pd, int n);
//Generate the closure instrument to make a closure from proto `p` and return it to `ret`.
LSI_EXTERN void lsK_makeclosure(ls_ParserData* pd, int p, ls_Expr* ret);
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
//Generate ADJUST instruction to expand list-returned values or to fill some nil on stack. Used in expr list.
LSI_EXTERN void lsK_finishmultiexpr(ls_ParserData* pd, int expand_from, int fill_to);
//Get the temp var on stack.
LSI_EXTERN void lsK_getlocalat(ls_ParserData* pd, int pos, ls_Expr* expr);

#endif
