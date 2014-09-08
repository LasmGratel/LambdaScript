#include "ls.h"
#include "common.h"
#include "state.h"
#include "mem.h"
#include "object.h"
#include "string.h"
#include "gc.h"
#include "stream.h"
#include "lex.h"

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

static const char* getS(void* ud, size_t* sz)
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
	}

	/* String stream test */ {
		LoadS ss = { "This is a string.\n", 18 };
		ls_Stream stream;
		ls_Char c;
		lsZ_creates(&stream, getS, &ss);
		while (lsZ_readb(&stream, &c), c != ls_EOS)
		{
			putchar(c);
		}
		lsZ_close(&stream);
	}

	/* Memory buffer test */ {
		ls_MemBuff buf;
		lsZ_newbuf(L, &buf);
		lsZ_appendbuf(&buf, 'O');
		lsZ_appendbuf(&buf, 'K');
		lsZ_appendbuf(&buf, '\n');
		lsZ_appendbuf(&buf, 0);
		printf(buf.buf);
	}

	ls_close(L);
	return 0;
}
