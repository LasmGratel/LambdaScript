#include "ls.h"
#include "common.h"
#include "state.h"
#include "mem.h"
#include "object.h"
#include "string.h"
#include "gc.h"
#include "stream.h"
#include "lex.h"
#include "table.h"
#include "parser.h"
#include "func.h"

#define set_string_stream(s, str) { \
LoadS ss = { str, strlen(str) }; \
lsZ_creates((s), "<string>", getS, &ss); }

//#define printf

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
	getchar();
}

typedef struct LoadS
{
	const char* str;
	ls_MemSize len;
} LoadS;

static const char* getS(void* ud, ls_MemSize* sz)
{
	LoadS* s = cast(LoadS*, ud);
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

		p = lsM_newblock(L, 200);
		p = lsM_resizeblock(L, p, 200, 10);
		lsM_freeblock(L, p, 10);

		p = lsM_newobj(L, int[100]);
		lsM_freeobj(L, p, int[100]);

		int* vec;
		int n = 0, cap = 10;
		int max = MAX_MEMSIZE;
		vec = lsM_newvector(L, cap, int);
		for (int i = 0; i < 20; ++i)
		{
			lsM_growvector(L, vec, n, cap, int, 30, "test");
			vec[n++] = n;
		}
		lsM_freevector(L, vec, cap);

		/* String object test */
		ls_Object* obj;
		obj = cast(ls_Object*, lsS_newstrf(L, "Hello, %s!\n", "Lambda"));
		printf(getstr(&obj->s));
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
		void* pos = buf.buf + buf.sz;
		lsZ_rawappendbuf_t_(&buf, int, 1000);
		assert(*(int*)pos == 1000);
		printf(buf.buf);
		lsZ_freebuf(&buf);
	}

	/* Lexical analyzer */ for (int i = 0; i < 1; ++i){
		const char* name[] = {
			"EOS",
			"Symbol",
			"Identifier",
			"Number",
			"Keyword",
			"Single line comment",
			"Multiline comment",
			"String literal"
		};
		ls_Stream stream;
		ls_LexState lex;
		set_string_stream(&stream,
			"var b; a,b=b,c;"
			);
		lsX_initlex(L, &lex, &stream);
		lsY_rawparse(L, &lex);
		/*
		do
		{
			lsX_next(&lex);
			//printf("%s: ", name[lex.current.t]);
			switch (lex.current.t)
			{
			case TK_NUMBER:
				printf("%f\n", lex.current.d.n);
				break;
			case TK_COMMENT_S:
			case TK_COMMENT_M:
			case TK_IDENTIFIER:
			case TK_STRING:
				printf("%s\n", getstr(lex.current.d.objs));
				break;
			default:
				if (lex.current.t < TK_NOT_USED) printf("%c\n", cast_byte(lex.current.t));
				break;
			}
		} while (lex.current.t != TK_EOS);
		*/
		lsX_freelex(&lex);
		lsZ_closes(&stream);
	}

	ls_close(L);
	return 0;
}
