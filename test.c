#include <stdio.h>

#include "ls.h"
#include "common.h"
#include "state.h"
#include "mem.h"

static void *l_alloc(void *ud, void *ptr, ls_MemSize osize, ls_MemSize nsize) {
	(void)ud;  (void)osize;  /* not used */
	if (nsize == 0) {
		free(ptr);
		return NULL;
	}
	else
		return realloc(ptr, nsize);
}

int main()
{
	ls_State* L = ls_newstate(l_alloc, NULL);
	ls_close(L);
	return 0;
}
