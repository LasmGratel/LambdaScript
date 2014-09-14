#include "ls.h"
#include "common.h"
#include "object.h"
#include "gc.h"
#include "table.h"
#include "mem.h"

ls_Table* lsH_new(ls_State* L)
{
	ls_Table* ret = lsC_newobj(L, LS_OBJ_TABLE, sizeof(ls_Table), ls_NULL);
	ret->n = ret->size = 0;
	ret->nodes = ls_NULL;
}

ls_Value* lsH_get(ls_Table* tab, ls_Value* key)
{
	for (int i = 0; i < tab->n; ++i)
	{
		if (lsO_valequal(key, &tab->nodes[i].key))
		{
			return &tab->nodes[i].val;
		}
	}
	return ls_NULL;
}

static void grownodevector(ls_State* L, ls_Table* tab)
{
	int old_size = tab->size;
	lsM_growvector(L, tab->nodes, tab->n, tab->size, ls_Node, 1000, "nodes in table");
	while (old_size < tab->size)
	{
		ls_Node* node = &tab->nodes[old_size++];
		node->key.next = ls_NULL;
		node->key.v.tt = LS_OBJ_NIL;
		node->val.tt = LS_OBJ_NIL;
	}
}

ls_Value* lsH_getorcreate(ls_State* L, ls_Table* tab, ls_Value* key)
{
	ls_Value* ret = lsH_get(tab, key);
	if (ret == ls_NULL)
	{
		ls_Node* node = &tab->nodes[tab->n++];
		node->key.v = *key;
		return &node->val;
	}
}
