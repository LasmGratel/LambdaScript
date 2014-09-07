//state.h
//Declaration of state struct and others related

#ifndef LS_STATE_H
#define LS_STATE_H

typedef struct ls_GlobalState
{
	/* Memory management */
	ls_MemAllocator alloc;
	void* alloc_ud;
	ls_MemSize memused[LSM_ALLOC_USAGE_N + 1];

} ls_GlobalState;

typedef struct ls_State
{
	ls_GlobalState* g;
} ls_State;

#endif
