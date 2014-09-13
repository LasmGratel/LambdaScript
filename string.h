#ifndef LS_STRING_H
#define LS_STRING_H

#define lsS_fix(x) ((void)0) //TODO fix!

LSI_EXTERN const char* lsS_sprintf(const char* str, ...);
LSI_EXTERN const char* lsS_vsprintf(const char* str, va_list args);

LSI_EXTERN ls_String* lsS_newstr_n(ls_State* L, const char* str, ls_MemSize n);//n=strlen + 1, including '\0'
LSI_EXTERN ls_String* lsS_newstr(ls_State* L, const char* str);
LSI_EXTERN ls_String* lsS_newstrf(ls_State* L, const char* str, ...);

LSI_EXTERN int lsS_equal(ls_String* a, ls_String* b);

LSI_EXTERN void lsS_resize(ls_State* L, ls_MemSize n);

#endif
