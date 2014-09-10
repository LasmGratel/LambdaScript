#include <errno.h>

#include "ls.h"
#include "common.h"
#include "mem.h"
#include "object.h"
#include "stream.h"
#include "lex.h"
#include "ctype.h"

//print as string to enable debugging
#define PRINT_BUFFER_AS_STRING
//read comment as a single token to allow in-comment metadata
#define LS_COMMENT_AS_TOKEN

#define lex_error(...) ls_throw(ls->buf.L, LS_ERRRUN, __VA_ARGS__)

/* Helper functions. Only valid in the loop of lsX_next */

//peek a char
#define peek() lsZ_peekb(ls->s)
//read a char to local var c
#define next() lsZ_readb(ls->s, c)
//skip the char
#define skip() lsZ_nextb(ls->s)
//skip and then peek to c
#define skip_peek() c = (skip(), peek())
//skip several chars until 'cond' is true
#define skip_until(cond) { while(c = peek(), !(cond)){ next(); } }
//used when encontered '\n' or '\r' with next()
#define finish_new_line() ++ls->lno; skip_if(c == '\n' ? '\r' : '\n')
//clear buffer
#define clear_buf() lsZ_clearbuf(&ls->buf)
//append c to buffer (stream not changed)
#define append_buf() lsZ_appendbuf(&ls->buf, c)
//append a char to buffer if cond is true (already peeked to c)
#define append_buf_if(cond) if (cond) { append_buf(); next(); }
//append several chars as long as cond is true
#define append_buf_while(cond) { while(c = peek(), (cond)){ append_buf(); skip(); } }
//skip a char if it is 'what'
#define skip_if(what) (lsZ_peekb(ls->s) == (what) ? lsZ_nextb(ls->s) : (void)0)
//set data on buffer to result
#define set_as_buf() {buf_finished(); ls->current.d.str.l = lsZ_bufsize(&ls->buf); ls->current.d.str.s = ls->buf.buf;}
//set token type and return
#define return_type(type) ls->current.t = (type); return
//current char is not a token, try next
#define read_again() next(); break
//if cond is false, raise error
#define check(cond, ...) if (!(cond)) lex_error(__VA_ARGS__)

#ifdef PRINT_BUFFER_AS_STRING
#define buf_finished() lsZ_appendbuf(&ls->buf, 0)
#else
#define buf_finished() ((void)0)
#endif

static void multiline_comment(ls_LexState* ls)
{
#ifdef LS_COMMENT_AS_TOKEN
	clear_buf();
#endif
	char c, c2;
	lsZ_readb(ls->s, c);
	lsZ_readb(ls->s, c2);
	while (c != '*' || c2 != '/')
	{
#ifdef LS_COMMENT_AS_TOKEN
		append_buf();
#endif
		c = c2;
		lsZ_readb(ls->s, c2);
	}
#ifdef LS_COMMENT_AS_TOKEN
	set_as_buf();
#endif
}

static void number(ls_LexState* ls, char c)
{
	clear_buf();
	append_buf();
	append_buf_while(lisdigit(c));
	if (c == '.')
	{
		//We can continue
		append_buf();
		skip_peek();
		//Number should not end here!
		check(lisdigit(c), "bad number format");
		append_buf_while(lisdigit(c));
	}
	if (c == 'E' || c == 'e')
	{
		append_buf();
		skip_peek();
		append_buf_if(c == '+' || c == '-');
		//Number should not end here!
		check(lisdigit(c), "bad number format");
		append_buf_while(lisdigit(c));
	}
	lsZ_appendbuf(&ls->buf, '\0');
	char* e;
	ls->current.d.n = strtod(ls->buf.buf, &e);
	if (*e != '\0' || errno == ERANGE)
	{
		lex_error("bad number format");
	}
}

void lsX_next(ls_LexState* ls)
{
	//Read the first char to determine where to go
	char c;
	next();
	for (;;)
	{
		switch (c)
		{
		case '\n': case '\r':
			//new line
			finish_new_line();
			read_again();
		case ' ': case '\f': case '\t': case '\v':
			//space
			read_again();
		case '/':
			//'/' or comment
			switch (peek())
			{
			case '/': {
				//single line comment
#ifdef LS_COMMENT_AS_TOKEN
				//read this line into TOKEN_COMMENT
				skip();
				clear_buf();
				append_buf_while(c != '\n' && c != '\r');
				skip();
				finish_new_line();
				set_as_buf();
#else
				skip_until(c == '\n' || c == '\r');
#endif
				return_type(TOKEN_COMMENT_S);
			}
			case '*':
				//multiline comment
				skip();
				multiline_comment(ls);
				return_type(TOKEN_COMMENT_M);
			default:
				//'/'
				ls->current.d.sym = '/';
				return_type(TOKEN_SYMBOL);
			}
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			number(ls, c);
			return_type(TOKEN_NUMBER);
		case ls_EOS:
			return_type(TOKEN_EOS);
		default:
			if (lislalpha(c))
			{
				clear_buf();
				append_buf();
				append_buf_while(lislalnum(c) || c == '_');
				set_as_buf();
				return_type(TOKEN_IDENTIFIER);
			}
			else
			{
				ls->current.d.sym = c;
				return_type(TOKEN_SYMBOL);
			}
		}
	}
}
