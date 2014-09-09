#include "ls.h"
#include "common.h"
#include "stream.h"

void lsZ_creates(ls_Stream* s, const char* chunkname, ls_InputReader r, void* ud)
{
	s->chunkname = chunkname;
	s->r = r;
	s->ud = ud;

	lsZ_fills(s);
}

void lsZ_fills(ls_Stream* s)
{
	ls_MemSize len;
	if ((s->buf = (*s->r)(s->ud, &len)) == ls_NULL || len == 0)
	{
		s->bl = 0;
		s->pos = 0;
		s->peek = ls_EOS;
	}
	else
	{
		s->bl = len;
		s->pos = 0;
		s->peek = s->buf[0];
	}
}
