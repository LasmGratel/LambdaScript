//common.h
//For internal use only

#ifndef LS_COMMON_H
#define LS_COMMON_H

#include "config.h"
#include <stdint.h>

//Macros: values

#define ls_NULL  ((void*)0)
#define ls_Bool int
#define ls_TRUE  (1)
#define ls_FALSE (0)

#define MAX_MEMSIZE	((ls_MemSize)(PTRDIFF_MAX-2))

#if !defined(MIN_STRTAB_SIZE)
#define MIN_STRTAB_SIZE	32
#endif

//internal object type
#define LS_OBJ_NIL         0
#define LS_OBJ_BOOL        1
#define LS_OBJ_LUSERDATA   2
#define LS_OBJ_USERDATA    3
#define LS_OBJ_NUMBER      4
#define LS_OBJ_TABLE       5
#define LS_OBJ_STRING      6
#define LS_OBJ_FUNCTION    7
#define LS_OBJ_THREAD      8
#define LS_OBJ_NUMTAGS     9

/*
** Extra tags for non-values
*/
#define LS_OBJ_PROTO     LS_OBJ_NUMTAGS
#define LS_OBJ_UPVAL    (LS_OBJ_NUMTAGS+1)
#define LS_OBJ_DEADKEY  (LS_OBJ_NUMTAGS+2)

/*
** number of all possible tags (including LUA_TNONE but excluding DEADKEY)
*/
#define LUA_TOTALTAGS	(LUA_TUPVAL+2)


#define LS_OBJ_FUNC_L	(LS_OBJ_FUNCTION | (0 << 4))  /* Ls closure */
#define LS_OBJ_FUNC_C	(LS_OBJ_FUNCTION | (1 << 4))  /* light C function */
#define LS_OBJ_FUNC_CC	(LS_OBJ_FUNCTION | (2 << 4))  /* C closure */

#define LS_OBJ_STRING_S	(LS_OBJ_STRING | (0 << 4))  /* short strings */
#define LS_OBJ_STRING_L	(LS_OBJ_STRING | (1 << 4))  /* long strings */


//Macros: functions

#define G(L) (L->g)

#define cast(t, v) ((t)(v))
#define cast_byte(v) cast(ls_byte, v)
#define cast_int(v) cast(int, v)

#define hashmod(s,size) \
	(ls_check_exp((size&(size - 1)) == 0, (cast(int, (s)& ((size)-1)))))
#define isreserved(obj) ((obj)->s.extra > 0)

//Macros: others

//Alignment
#ifdef LS_USE_ALIGN_T
#define LSI_ALIGNMENT_T	union { double u; void *s; long l; }
#else
#define LSI_ALIGNMENT_T	char
#endif
#define ls_AlignmentHeader LSI_ALIGNMENT_T _align

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
#define ls_check_exp(c,e) (ls_assert(c), (e))



//Type declarations

typedef struct ls_State ls_State;
typedef struct ls_GlobalState ls_GlobalState;
typedef union ls_Object ls_Object;
typedef union ls_String ls_String;
typedef struct ls_Proto ls_Proto;
typedef struct ls_Stream ls_Stream;
typedef struct ls_MemBuff ls_MemBuff;
typedef struct ls_ParserData ls_ParserData;
typedef unsigned int ls_Hash;

typedef char ls_byte;
typedef LS_UINT16 ls_uint16;
typedef LS_UINT32 ls_uint32;
typedef LS_INT16 ls_int16;
typedef LS_INT32 ls_int32;

typedef ls_uint32 Instruction;


//Used by parser
//size types
typedef ls_int32 ls_NParserG; //suitable for all following types
typedef ls_int32 ls_NLocal; //locals
typedef int ls_NInst; //instruments
#define MAX_LOCAL_IN_PROTO  SHRT_MAX
#define MAX_UPVAL_IN_PROTO	UCHAR_MAX
#define MAX_ACTIVE_LOCAL_IN_PARSER  (INT_MAX - 2)
#define MAX_ACTIVE_LOCAL_IN_FUNC    200


#endif
