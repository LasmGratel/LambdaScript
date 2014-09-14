#ifndef LS_OBJECT_H
#define LS_OBJECT_H

#define ls_CommonHeader ls_Object *next; ls_byte tt; ls_byte marked

typedef union ls_String
{
	ls_AlignmentHeader;
	struct {
		ls_CommonHeader;
		ls_MemSize len;//including '\0'
		ls_Hash h;
		ls_byte extra;
	} s;
} ls_String;


/*
** Description of a local variable for function prototypes
** (used for debug information)
*/
typedef struct ls_LocVar
{
	ls_String *varname;
	ls_NInst startpc;  /* first point where variable is active */
	ls_NInst endpc;    /* first point where variable is dead */
} ls_LocVar;

typedef struct ls_Upvalue
{
	ls_String *name;  /* upvalue name (for debug information) */
	ls_Bool inlocal;  /* true if it's local in outer func, false if it's upval */
	ls_NLocal idx;  /* index of upvalue (in stack or in outer function's list) */
} ls_Upvalue;

typedef struct ls_Proto
{
	ls_CommonHeader;
	ls_LocVar* locvars;  /* information about local variables (debug information) */
	ls_NLocal sizelocvars;
	ls_Upvalue* upvalues;  /* upvalue information */
	ls_NLocal sizeupvalues;  /* size of 'upvalues' */
	//TValue *k;  /* constants used by the function */
	//Instruction *code;
	//struct ls_Proto **p;  /* functions defined inside the function */
	//int *lineinfo;  /* map from opcodes to source lines (debug information) */
	//union Closure *cache;  /* last created closure with this prototype */
	//TString  *source;  /* used for debug information */
	//int sizek;  /* size of `k' */
	//int sizecode;
	//int sizelineinfo;
	//int sizep;  /* size of `p' */
	//int linedefined;
	//int lastlinedefined;
	//GCObject *gclist;
	//lu_byte numparams;  /* number of fixed parameters */
	//lu_byte is_vararg;
	//lu_byte maxstacksize;  /* maximum stack used by this function */
} ls_Proto;

typedef struct ls_ObjectHeader
{
	ls_CommonHeader;
} ls_ObjectHeader;

typedef union ls_Object
{
	ls_ObjectHeader gch;
	ls_String s;
	ls_Proto p;
} ls_Object;

#define getstr(s) cast(const char *, cast(ls_String*, (s)) + 1)

#endif
