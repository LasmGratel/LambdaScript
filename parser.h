#ifndef LS_PARSER_H
#define LS_PARSER_H

typedef enum
{
	//nil, true and false
	EXP_NTF,

	/* Stored */
	//Local variable
	EXP_LOCAL,
	//Calculated temperary on stack
	EXP_TEMP,
	//Upvalue
	EXP_UPVAL,
	//Constant (k)
	EXP_CONST,

	/* Indexed */
	//Table and key
	EXP_INDEXED,

	/* Closure */
	EXP_CLOSURE,

	/* Function call */
	//Don't change order!
	EXP_CALL_SINGLERET,
	EXP_CALL_LISTRET,
	EXP_CALL_MULTIRET,

	/* Operator */
	EXP_UNOPR,
	EXP_BINOPR,

	//EXP_TEMP will become EXP_UNAVAILABLE after it's poped from stack
	EXP_UNAVAILABLE,
} ls_Expkind;

//Consistent with parse(@priority, @getbinopr) and review module(@print_code)
typedef enum
{
	OPR_ADD, OPR_SUB, OPR_MUL, OPR_DIV,
	OPR_EQ, OPR_LT, OPR_LE,
	OPR_NE, OPR_GT, OPR_GE,
	/*
	OPR_MOD, OPR_POW,
	OPR_CONCAT,
	OPR_AND, OPR_OR,
	*/
	OPR_NOBINOPR
} BinOpr;

typedef enum { OPR_MINUS, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;

enum
{
	EXP_NTF_NIL,
	EXP_NTF_TRUE,
	EXP_NTF_FALSE,
};

//Expressions whose value has been stored on stack, upvalue table or const table
//Including already calculated expressions and locals, upvals
typedef struct ls_StoredExpr
{
	ls_Expkind k;
	ls_NLocal id;
} ls_StoredExpr;

//Table expression. tab and key must be stored
typedef struct ls_IndexedExpr
{
	ls_Expkind k;
	ls_StoredExpr tab;
	ls_StoredExpr key;
} ls_IndexedExpr;

//Not calculated closure
typedef struct ls_ClosureExpr
{
	ls_Expkind k;
	ls_NLocal p;
} ls_ClosureExpr;

typedef ls_StoredExpr ls_NTFExpr;

typedef struct ls_CallExpr
{
	ls_Expkind k;
	ls_StoredExpr f;//Index of the function on stack. The params are pushed after.
} ls_CallExpr;

typedef struct ls_OprExpr
{
	ls_Expkind k;
	union {
		UnOpr u;
		BinOpr b;
	} opr;
	ls_StoredExpr l;
	ls_StoredExpr r;
} ls_OprExpr;

typedef struct ls_Expr {
	union
	{
		ls_Expkind k;//Common header, used to get expression type
		ls_StoredExpr s;
		ls_IndexedExpr i;
		ls_ClosureExpr c;
		ls_NTFExpr n;
		ls_CallExpr f;
		ls_OprExpr o;
	} u;
	//int t;  /* patch list of `exit when true' */
	//int f;  /* patch list of `exit when false' */
} ls_Expr;

//Helper functions are mostly used in code.c, so defined there.

//Just used to store stack position
//Is not directly used by parser
//Deprecated
typedef int ls_MultiAssignInfo;

typedef struct ls_MultiExprInfo
{
	//Where the first expression (result) is. It's a EXP_TEMP.
	int start;
	//Number of expression. One multi-returned is counted once.
	//If used in multi-assignment and left values are less than count, no FILL instrument is needed.
	int count;
	//Where the first multi-returned expression is. 
	//If it's -1, no expand is generated.
	int firstmulti;
} ls_MultiExprInfo;

typedef struct ls_Vardesc
{
	ls_NLocal idx;
} ls_Vardesc;

//typedef struct Labeldesc
//{
//	ls_String *name;  /* label identifier */
//	int pc;  /* position in code */
//	int line;  /* line where it appeared */
//	ls_byte nactvar;  /* local level where it appears in current block */
//} Labeldesc;
//
//typedef struct Labellist
//{
//	Labeldesc *arr;  /* array */
//	int n;  /* number of entries in use */
//	int size;  /* array size */
//} Labellist;

typedef struct ls_ParseFunc ls_ParseFunc;

typedef struct ls_ParserData
{
	ls_State* L;
	ls_ParseFunc* pf;
	ls_LexState* ls;

	//name of global (as an upval)
	ls_String* nameg;
	//name of varargs (as local table)
	ls_String* namevp;

	/* list of active local variables */
	//map from function's i-th local to locals in Proto (which contains not only active)
	struct
	{
		ls_Vardesc *arr;
		int n;
		int size;
	} actvarmap;
	//Labellist gt;  /* list of pending gotos */
	//Labellist label;   /* list of active labels */

} ls_ParserData;

typedef struct ls_Block
{
	struct ls_Block* previous;  /* chain */
	ls_NLocal nactvar;  /* # active locals outside the block */
	//short firstlabel;  /* index of first label in this block */
	//short firstgoto;  /* index of first pending goto in this block */
	ls_byte hasup;  /* true if some variable in the block is an upvalue */
	//ls_byte isloop;  /* true if `block' is a loop */
} ls_Block;

/* state needed to generate code for a given function */
typedef struct ls_ParseFunc
{
	ls_Proto* f;  /* current function header */
	
	//Parser chains
	ls_ParseFunc* prev;  /* enclosing function */
	ls_Block* bl;  /* chain of current blocks */

	//Local vars
	struct {
		//Count used to visit local array in Proto
		//In Proto there's no counter for that array
		ls_NLocal n;
		//Offset used for actvarmap in ls_ParserData
		//ParserData contains items for enclosing functions so an offset is need
		ls_NLocal offset;
		//Real number of active variabls
		//Different from that in ls_ParserData, see `offset`
		ls_NLocal nact;
	} locals;

	ls_NInst pc;  /* next position to code (equivalent to `ncode') */
	ls_NLocal nupvals;

	int nk;  /* number of elements in `k' */
	int np;  /* number of elements in `p' */

	//(by acaly) this table is pushed on stack used to prevent objects to be collected by gc
	//ls_Table *h;  /* table to find (and reuse) elements in `k' */
	//int lasttarget;   /* 'label' of last 'jump label' */
	//int jpc;  /* list of pending jumps to `pc' */
	//int firstlocal;  /* index of first local var (in Dyndata array) */
	//short nlocvars;  /* number of elements in 'f->locvars' */
	//ls_byte nactvar;  /* number of active local variables */
	//ls_byte nups;  /* number of upvalues */
	ls_byte freereg;  /* first free register */
} ls_ParseFunc;

typedef struct ls_Assignment
{
	struct ls_Assignment *prev;
	ls_Expr v;
} ls_Assignment;

LSI_EXTERN void lsY_rawparse(ls_State* L, ls_LexState* zin);

#endif
