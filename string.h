#ifndef LS_STRING_H
#define LS_STRING_H

LSI_EXTERN const char* lsS_tocstr(ls_String* s);
LSI_EXTERN ls_String* lsS_newstr(ls_State* L, const char* str);
LSI_EXTERN ls_String* lsS_newstrf(ls_State* L, const char* str, ...);
LSI_EXTERN int lsS_equal(ls_String* a, ls_String* b);

#endif
