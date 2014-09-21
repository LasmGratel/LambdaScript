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

typedef struct ls_Value
{
	struct
	{
		ls_Object* gc;    /* collectable objects */
		void* p;         /* light userdata */
		ls_Number n;         /* numbers */
	} v;
	ls_byte tt;
} ls_Value;

typedef struct ls_Proto
{
	ls_CommonHeader;
	ls_LocVar* locvars;  /* information about local variables (debug information) */
	ls_NLocal sizelocvars;
	ls_Upvalue* upvalues;  /* upvalue information */
	ls_NLocal sizeupvalues;  /* size of 'upvalues' */
	ls_Value *k;  /* constants used by the function */
	Instruction *code;
	struct ls_Proto **p;  /* functions defined inside the function */
	int sizek;  /* size of `k' */
	int sizecode;
	int sizep;  /* size of `p' */
	int numparams;  /* number of fixed parameters */
	int is_vararg;
	//int *lineinfo;  /* map from opcodes to source lines (debug information) */
	//union Closure *cache;  /* last created closure with this prototype */
	//TString  *source;  /* used for debug information */
	//int sizelineinfo;
	//int linedefined;
	//int lastlinedefined;
	//GCObject *gclist;
	//lu_byte maxstacksize;  /* maximum stack used by this function */
} ls_Proto;

typedef struct ls_ObjectHeader
{
	ls_CommonHeader;
} ls_ObjectHeader;

typedef union ls_Key
{
	ls_Value v;
	struct ls_Node* next;
} ls_Key;

typedef struct ls_Node
{
	ls_Value val;
	ls_Key key;
} ls_Node;

typedef struct ls_Table
{
	ls_CommonHeader;
	ls_Node* nodes;
	int size;
	int n;
} ls_Table;

typedef union ls_Object
{
	ls_ObjectHeader gch;
	ls_String s;
	ls_Proto p;
	ls_Table t;
} ls_Object;

#define getstr(s) cast(const char *, cast(ls_String*, (s)) + 1)

LSI_EXTERN int lsO_objequal(ls_Object* a, ls_Object* b);
LSI_EXTERN int lsO_valequal(ls_Value* a, ls_Value* b);

#endif
