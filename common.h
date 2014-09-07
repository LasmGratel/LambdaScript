//common.h
//For internal use only

#ifndef LS_COMMON_H
#define LS_COMMON_H

#include "config.h"

//Macros: values

#define ls_NULL ((void*)0)

//Macros: functions

#define G(L) (L->g)
#define CAST(t, v) ((t) v)

/* Error declaration */

#define THROW(L, e, m) ((void)0) //TODO

/* assert */
#include <assert.h>
#define ls_forceassert(e) assert(e)
#ifdef LS_USE_ASSERT
#define ls_assert(e) ls_forceassert(e)
#else
#define ls_assert(e) (void(0))
#endif
#ifdef LS_USE_APICHECK
#define ls_apicheck(e) ls_forceassert(e)
#else
#define ls_apicheck(e) (void(0))
#endif



//Structure declarations

typedef struct ls_State ls_State;
typedef struct ls_GlobalState ls_GlobalState;

//Typedefs


#endif
