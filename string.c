#include "ls.h"
#include "common.h"
#include "object.h"
#include "string.h"
#include "gc.h"

#define STR_SEED 5381
#define STR_FORMATMAXLEN 1000
#define USAGE_STRING LSM_ALLOC_FIX

#define TO_CSTR(x) CAST(char*, CAST(ls_String*, x) + 1)

const char* lsS_tocstr(ls_String* s)
{
	return TO_CSTR(s);
}

ls_Hash lsS_hash(const char* str, ls_MemSize l, ls_Hash seed)
{
	if (!str)
	{
		return 0;
	}
	ls_Hash hash = seed;
	while (l--)
		hash = ((hash << 5) + hash) + (*str++); /* hash * 33 + c */
	return hash;
}

ls_String* lsS_newstr(ls_State* L, const char* str)
{
	ls_MemSize len = strlen(str) + 1; //including \0
	ls_Hash h = lsS_hash(str, len, STR_SEED);
	//TODO reuse strings
	ls_String* s = &lsC_newobj(L, LS_OBJ_STRING, sizeof(ls_String) + len, USAGE_STRING, ls_NULL)->s;
	s->s.len = len;
	s->s.h = h;
	memcpy(TO_CSTR(s), str, len);
	return s;
}

ls_String* lsS_newstrf(ls_State* L, const char* str, ...)
{
	static char buff[STR_FORMATMAXLEN + 1];
	va_list args;
	va_start(args, str);
	vsnprintf(buff, STR_FORMATMAXLEN, str, args);
	va_end(args);
	return lsS_newstr(L, buff);
}

int lsS_equal(ls_String* a, ls_String* b)
{
	return a->s.h == b->s.h && strcmp(TO_CSTR(a), TO_CSTR(b));
}
