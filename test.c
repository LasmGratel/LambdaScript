#include "ls.h"
#include "common.h"
#include "state.h"
#include "mem.h"
#include "object.h"
#include "string.h"
#include "gc.h"
#include "stream.h"
#include "lex.h"

#define set_string_stream(s, str) { \
LoadS ss = { str, strlen(str) }; \
lsZ_creates((s), "<string>", getS, &ss); }

static void *l_alloc(void *ud, void *ptr, ls_MemSize osize, ls_MemSize nsize) {
	(void)ud;  (void)osize;  /* not used */
	if (nsize == 0) {
		free(ptr);
		return NULL;
	}
	else
		return realloc(ptr, nsize);
}

ls_noreturn ls_throw(ls_State* L, int code, const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
}

typedef struct LoadS
{
	const char* str;
	ls_MemSize len;
} LoadS;

static const char* getS(void* ud, ls_MemSize* sz)
{
	LoadS* s = CAST(LoadS*, ud);
	if (s->len)
	{
		*sz = s->len;
		s->len = 0;
		return s->str;
	}
	return ls_NULL;
}

int main()
{
	ls_State* L = ls_newstate(l_alloc, NULL);

	/* Memory allocation test */ {
		void* p;

		p = lsM_newblock(L, 0, 200);
		p = lsM_resizeblock(L, 0, p, 200, 10);
		lsM_freeblock(L, 0, p, 10);

		p = lsM_newobj(L, 0, int[100]);
		lsM_freeobj(L, 0, p, int[100]);

		ArrayInfo info = {0, INT_MAX, "test"};
		p = lsM_newarray(L, &info, int, 10);
		p = lsM_resizearray(L, &info, p, int, 100);
		lsM_freearray(L, &info, p);

		/* String object test */
		ls_Object* obj;
		obj = CAST(ls_Object*, lsS_newstrf(L, "Hello, %s!\n", "Lambda"));
		printf(lsS_tocstr(&obj->s));
		//ls_Object can not be freed
	}

	/* String stream test */ {
		ls_Stream stream;
		set_string_stream(&stream, "This is a string.\n")
		ls_Char c;
		while (lsZ_readb(&stream, c), c != ls_EOS)
		{
			putchar(c);
		}
		lsZ_closes(&stream);
	}

	/* Memory buffer test */ {
		ls_MemBuff buf;
		lsZ_newbuf(L, &buf);
		lsZ_appendbuf(&buf, 'O');
		lsZ_appendbuf(&buf, 'K');
		lsZ_appendbuf(&buf, '\n');
		lsZ_appendbuf(&buf, 0);
		printf(buf.buf);
		lsZ_closebuf(&buf);
	}

	/* Lexical analyzer */ {
		const char* name[] = {
			"EOS",
			"Symbol",
			"Identifier",
			"Number",
			"Keyword",
			"Single line comment",
			"Multiline comment"
		};
		ls_Stream stream;
		ls_LexState lex;
		set_string_stream(&stream, 
			"int main()\n"
			"{\n"
			"	ls_State* L = ls_newstate(l_alloc, NULL);\n"
			"\n"
			"	/* Memory allocation test */ {\n"
			"		void* p;\n"
			"\n"
			"		p = lsM_newblock(L, 0, 200);\n"
			"		p = lsM_resizeblock(L, 0, p, 200, 10);\n"
			"		lsM_freeblock(L, 0, p, 10);\n"
			);
		lsX_initlex(L, &lex, &stream);
		do
		{
			lsX_next(&lex);
			printf("%s: ", name[lex.current.t]);
			switch (lex.current.t)
			{
			case TOKEN_SYMBOL:
				printf("%c\n", lex.current.d.sym);
				break;
			case TOKEN_NUMBER:
				printf("%f\n", lex.current.d.n);
				break;
			case TOKEN_COMMENT_S:
			case TOKEN_COMMENT_M:
			case TOKEN_IDENTIFIER:
				printf("%s\n", lex.current.d.str);
				break;
			default:
				printf("\n");
				break;
			}
		} while (lex.current.t != TOKEN_EOS);
		lsX_freelex(&lex);
		lsZ_closes(&stream);
	}

	ls_close(L);
	return 0;
}
