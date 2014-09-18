#include "ls.h"
#include "common.h"
#include "object.h"
#include "string.h"
#include "gc.h"
#include "state.h"
#include "mem.h"

#define STR_SEED 5381
#define STR_FORMATMAXLEN 1000
#define STR_SHORT_MAX 30
#define STR_LONG_MAX 1000
/*
** Use at most ~(2^LUAI_HASHLIMIT) bytes from a string to compute its hash
*/
#ifndef LSI_HASHLIMIT
#define LSI_HASHLIMIT 5
#endif

#define eqshrstr(a,b)	ls_check_exp((a)->s.tt == LS_OBJ_STRING_S, (a) == (b))

ls_Hash lsS_hash(const char* str, ls_MemSize l, ls_Hash seed)
{
	ls_Hash h = seed ^ cast(ls_Hash, l);
	ls_MemSize step = (l >> LSI_HASHLIMIT) + 1;
	ls_MemSize l1;
	for (l1 = l; l1 >= step; l1 -= step)
		h = h ^ ((h << 5) + (h >> 2) + cast_byte(str[l1 - 1]));
	return h;
}

static char buff[STR_FORMATMAXLEN + 1];

const char* lsS_sprintf(const char* str, ...)
{
	va_list args;
	va_start(args, str);
	vsnprintf(buff, STR_FORMATMAXLEN, str, args);
	va_end(args);
	return buff;
}

const char* lsS_vsprintf(const char* str, va_list args)
{
	vsnprintf(buff, STR_FORMATMAXLEN, str, args);
	return buff;
}

ls_String* lsS_newstrf(ls_State* L, const char* str, ...)
{
	va_list args;
	va_start(args, str);
	const char* result = lsS_vsprintf(str, args);
	va_end(args);
	return lsS_newstr(L, result);
}

ls_String* lsS_newstr(ls_State* L, const char* str)
{
	return lsS_newstr_n(L, str, strlen(str) + 1);
}

static ls_String* createstr(ls_State* L, const char* str, ls_MemSize len, int tt, ls_Hash h)
{
	ls_Object** list;  /* (pointer to) list where it will be inserted */
	ls_StringTable* tb = &G(L)->strt;
	if (tb->nuse >= tb->size && tb->size <= INT_MAX / 2)
		lsS_resize(L, tb->size * 2);  /* too crowded */
	list = &tb->hash[hashmod(h, tb->size)];

	ls_String* s = &lsC_newobj(L, len > STR_SHORT_MAX ? LS_OBJ_STRING_L : LS_OBJ_STRING_S, sizeof(ls_String)+len, list)->s;
	s->s.len = len;
	s->s.h = h;
	s->s.extra = 0;
	memcpy(s + 1, str, len);

	return s;
}

static ls_String* newshort(ls_State* L, const char* str, ls_MemSize len)
{
	ls_Object* o;
	ls_GlobalState* g = G(L);
	ls_Hash h = lsS_hash(str, len, g->seed);
	for (o = g->strt.hash[hashmod(h, g->strt.size)];
		o != ls_NULL;
		o = gch(o)->next) {
		ls_String* ts = rawgco2str(o);
		if (h == ts->s.h &&
			len == ts->s.len &&
			(memcmp(str, getstr(ts), len * sizeof(char)) == 0)) {
			//if (isdead(G(L), o))  /* string is dead (but was not collected yet)? */
			//	changewhite(o);  /* resurrect it */
			return ts;
		}
	}
	return createstr(L, str, len, LS_OBJ_STRING_S, h);
}

static ls_String* newlong(ls_State* L, const char* str, ls_MemSize len)
{
	if (len > STR_LONG_MAX)
	{
		ls_throw(L, LS_ERRRUN, "string is too big");
	}
	return createstr(L, str, len, LS_OBJ_STRING_L, G(L)->seed);
}

ls_String* lsS_newstr_n(ls_State* L, const char* str, ls_MemSize len)
{
	return len > STR_SHORT_MAX ? newlong(L, str, len) : newshort(L, str, len);
}

int eqlngstr(ls_String *a, ls_String *b) {
	ls_MemSize len = a->s.len;
	ls_assert(a->s.tt == LS_OBJ_STRING_L && b->s.tt == LS_OBJ_STRING_L);
	return (a == b) ||  /* same instance or... */
		((len == b->s.len) &&  /* equal length and ... */
		(memcmp(getstr(a), getstr(b), len) == 0));  /* equal contents */
}

int lsS_equal(ls_String* a, ls_String* b)
{
	return (a->s.tt == b->s.tt) && 
		(a->s.tt == LS_OBJ_STRING_S ? eqshrstr(a, b) : 0);
}

void lsS_resize(ls_State* L, ls_MemSize n)
{
	int i;
	ls_StringTable* tb = &G(L)->strt;
	//ensure string gc status
	if (n > tb->size) {
		lsM_reallocvector(L, tb->hash, tb->size, n, ls_Object*);
		for (i = tb->size; i < n; i++) tb->hash[i] = ls_NULL;
	}
	for (i = 0; i<tb->size; i++) {
		ls_Object* p = tb->hash[i];
		tb->hash[i] = ls_NULL;
		while (p) {  /* for each node in the list */
			ls_Object* next = gch(p)->next;  /* save next */
			ls_Hash h = hashmod(gco2str(p).h, n);  /* new position */
			gch(p)->next = tb->hash[h];  /* chain it */
			tb->hash[h] = p;
			//TODO reset old bit
			p = next;
		}
	}
	if (n < tb->size) {
		/* shrinking slice must be empty */
		ls_assert(tb->hash[n] == ls_NULL && tb->hash[tb->size - 1] == ls_NULL);
		lsM_reallocvector(L, tb->hash, tb->size, n, ls_Object*);
	}
	tb->size = n;
}
