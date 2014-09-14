#ifndef LS_TABLE_H
#define LS_TABLE_H

LSI_EXTERN ls_Table* lsH_new(ls_State* L);
LSI_EXTERN ls_Value* lsH_get(ls_Table* tab, ls_Value* key);
LSI_EXTERN ls_Value* lsH_getorcreate(ls_State* L, ls_Table* tab, ls_Value* key);

#endif
