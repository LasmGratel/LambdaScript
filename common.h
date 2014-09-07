//common.h
//For internal use only

#ifndef LS_LIMITS_H
#define LS_LIMITS_H

#include "config.h"

//Macros: values

#define ls_NULL ((void*)0)

//Macros: functions

#define G(L) (L->g)
#define CAST(t, v) ((t) v)

/* assert */
#include <assert.h>
#define ls_forceassert(e) assert(e)
#ifdef LS_O_USE_ASSERT
#define ls_assert(e) ls_forceassert(e)
#else
#define ls_assert(e) (void(0))
#endif
#ifdef LS_O_USE_APICHECK
#define ls_apicheck(e) ls_forceassert(e)
#else
#define ls_apicheck(e) (void(0))
#endif



//Structure declarations

struct ls_State;
struct ls_GlobalState;

//Typedefs


#endif
