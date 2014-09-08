#ifndef LS_LEX_H
#define LS_LEX_H

typedef struct ls_Token
{
	int t;
	union {
		int kw;
		char sym;
		ls_String* id;
		ls_Number n;
	} d;
} ls_Token;

typedef struct ls_LexState
{
	ls_Stream* s;

} ls_LexState;

#endif
