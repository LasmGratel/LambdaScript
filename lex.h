#ifndef LS_LEX_H
#define LS_LEX_H

#define FIRST_RESERVED 256

enum RESERVED {
	/* terminal symbols denoted by reserved words */
	TK_NOT_USED = FIRST_RESERVED,

	/* first reserved words */
	TK_VAR, TK_FUNC,
	TK_NIL, TK_TRUE, TK_FALSE,
	RESERED_END,

	/* other types */
	TK_EOS,
	TK_COMMENT_S,
	TK_COMMENT_M,
	TK_NUMBER,
	TK_IDENTIFIER,
	TK_STRING,

	/* not implemented */
	TK_BREAK,
	TK_DO, TK_ELSE, TK_ELSEIF, TK_END, _TK_FALSE, TK_FOR, TK_FUNCTION,
	TK_GOTO, TK_IF, TK_IN, TK_LOCAL, _TK_NIL, TK_NOT, TK_OR, TK_REPEAT,
	TK_RETURN, TK_THEN, _TK_TRUE, TK_UNTIL, TK_WHILE,
	/* other terminal symbols */
	TK_CONCAT, TK_DOTS, TK_EQ, TK_GE, TK_LE, TK_NE, TK_DBCOLON, _TK_EOS,
	_TK_NUMBER, _TK_NAME, _TK_STRING
};

#define NUM_RESERVED (cast(int, RESERED_END - FIRST_RESERVED - 1))

/*
#define TOKEN_EOS         0
#define TOKEN_SYMBOL      1
#define TOKEN_IDENTIFIER  2
#define TOKEN_NUMBER      3
#define TOKEN_KEYWORD     4
#define TOKEN_COMMENT_S   5
#define TOKEN_COMMENT_M   6
#define TOKEN_STRING      7
*/
typedef struct ls_Token
{
	ls_uint16 t;
	union {
		int kw;
		//char sym;//is set to 0 if not sym to make it easier
		struct {
			const char* s;
			ls_MemSize l;
		} str;
		ls_String* objs;
		ls_Number n;
	} d;
} ls_Token;

typedef struct ls_LexState
{
	ls_Stream* s;     //Input
	ls_MemBuff buf;   //Used to store when reading
	ls_MemSize lno;   //Line number
	ls_Token current; //Current token

	//ls_ParserData* pd;
} ls_LexState;

#define lsX_initlex(L, ls, _s) cast(void, (lsZ_newbuf(L, &(ls)->buf),(ls)->s = _s, (ls)->lno = 0))
#define lsX_freelex(ls) cast(void, (lsZ_freebuf(&(ls)->buf)))

LSI_EXTERN void lsX_init(ls_State* L);
LSI_EXTERN void lsX_next(ls_LexState* ls);

LSI_EXTERN ls_noreturn luaX_syntaxerror(ls_LexState* ls, const char *s);

#endif
