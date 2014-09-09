#ifndef LS_LEX_H
#define LS_LEX_H

#define TOKEN_EOS         0
#define TOKEN_SYMBOL      1
#define TOKEN_IDENTIFIER  2
#define TOKEN_NUMBER      3
#define TOKEN_KEYWORD     4
#define TOKEN_COMMENT_S   5
#define TOKEN_COMMENT_M   6

typedef struct ls_Token
{
	int t;
	union {
		int kw;
		char sym;
		struct {
			const char* s;
			ls_MemSize l;
		} str;
		ls_Number n;
	} d;
} ls_Token;

typedef struct ls_LexState
{
	ls_Stream* s;     //Input
	ls_MemBuff buf;   //Used to store when reading
	ls_MemSize lno;   //Line number
	ls_Token current; //Current token
} ls_LexState;

#define lsX_initlex(L, ls, _s) CAST(void, (lsZ_newbuf(L, &(ls)->buf),(ls)->s = _s, (ls)->lno = 0))
#define lsX_freelex(ls) CAST(void, (lsZ_closebuf(&(ls)->buf)))
LSI_EXTERN void lsX_next(ls_LexState* ls);

#endif
