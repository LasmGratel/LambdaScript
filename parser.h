#ifndef LS_PARSER_H
#define LS_PARSER_H

typedef enum {
	VVOID,	/* no value */
	VNIL,
	VTRUE,
	VFALSE,
	VK,		/* info = index of constant in `k' */
	VKNUM,	/* nval = numerical value */
	VNONRELOC,	/* info = result register */
	VLOCAL,	/* info = local register */
	VUPVAL,       /* info = index of upvalue in 'upvalues' */
	VINDEXED,	/* t = table register/upvalue; idx = index R/K */
	VJMP,		/* info = instruction pc */
	VRELOCABLE,	/* info = instruction pc */
	VCALL,	/* info = instruction pc */
	VVARARG	/* info = instruction pc */
} ls_Expkind;

typedef struct expdesc {
	ls_Expkind k;
	union {
		struct {  /* for indexed variables (VINDEXED) */
			short idx;  /* index (R/K) */
			ls_byte t;  /* table (register or upvalue) */
			ls_byte vt;  /* whether 't' is register (VLOCAL) or upvalue (VUPVAL) */
		} ind;
		int info;  /* for generic use */
		ls_Number nval;  /* for VKNUM */
	} u;
	//int t;  /* patch list of `exit when true' */
	//int f;  /* patch list of `exit when false' */
} expdesc;

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
	//ls_byte upval;  /* true if some variable in the block is an upvalue */
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

	//(by acaly) this table is pushed on stack used to prevent objects to be collected by gc
	//Table *h;  /* table to find (and reuse) elements in `k' */
	//int lasttarget;   /* 'label' of last 'jump label' */
	//int jpc;  /* list of pending jumps to `pc' */
	//int nk;  /* number of elements in `k' */
	//int np;  /* number of elements in `p' */
	//int firstlocal;  /* index of first local var (in Dyndata array) */
	//short nlocvars;  /* number of elements in 'f->locvars' */
	//ls_byte nactvar;  /* number of active local variables */
	//ls_byte nups;  /* number of upvalues */
	//ls_byte freereg;  /* first free register */
} ls_ParseFunc;

typedef struct ls_Assignment
{
	struct ls_Assignment *prev;
	expdesc v;
} ls_Assignment;

LSI_EXTERN void lsY_rawparse(ls_State* L, ls_LexState* zin);

#endif
