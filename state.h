//state.h
//Declaration of state struct and others related

#ifndef LS_STATE_H
#define LS_STATE_H


typedef struct ls_StringTable {
	ls_Object **hash;
	ls_MemSize nuse;  /* number of elements */
	ls_MemSize size;
} ls_StringTable;

typedef struct ls_GlobalState
{
	/* Memory management */
	ls_MemAllocator alloc;
	void* alloc_ud;
	ls_MemSize memused;//[LSM_ALLOC_USAGE_N + 1];

	/* String table */
	ls_Hash seed;
	ls_StringTable strt;

	/* GC */
	ls_Object* allgc;
} ls_GlobalState;

typedef struct ls_State
{
	ls_GlobalState* g;
} ls_State;

#define gch(o)          (&(o)->gch)
#define ttbasic(x)      ((x) & 0x0F)
#define ottbasic(o)     ttbasic((o)->gch.tt)
#define rawgco2str(o)   ls_check_exp(ottbasic(o) == LS_OBJ_STRING, &((o)->s))
#define gco2str(o)      (rawgco2str(o)->s)

#endif
