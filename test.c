#include "ls.h"
#include "common.h"
#include "state.h"
#include "mem.h"
#include "object.h"
#include "string.h"
#include "gc.h"

static void *l_alloc(void *ud, void *ptr, ls_MemSize osize, ls_MemSize nsize) {
	(void)ud;  (void)osize;  /* not used */
	if (nsize == 0) {
		free(ptr);
		return NULL;
	}
	else
		return realloc(ptr, nsize);
}

LS_API ls_noreturn ls_throw(ls_State* L, int code, const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
}

int main()
{
	ls_State* L = ls_newstate(l_alloc, NULL);

	/* Memory allocation test */
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
	obj = CAST(ls_Object*, lsS_newstrf(L, "Hello, %s!", "Lambda"));
	printf(lsS_tocstr(&obj->s));
	
	ls_close(L);
	return 0;
}
