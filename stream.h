#ifndef LS_STREAM_H
#define LS_STREAM_H

//use int to allow EOS(-1)
#define ls_Char int

#define ls_EOS (CAST(ls_Char, -1))

typedef struct ls_Stream
{
	const char* chunkname;

	ls_InputReader r;
	void* ud;

	const char* buf;
	ls_MemSize bl;

	ls_MemSize pos;
	ls_Char peek; //value at pos
} ls_Stream;

typedef struct ls_MemBuff
{
	ls_MemSize cap; //length of buffer
	ls_MemSize sz; //length of data
	char* buf;
	ls_State* L;
} ls_MemBuff;

/* stream bit manipulate */

//Peek a char and don't change pos
#define lsZ_peekb(s) ((s)->peek) 
//Step pos
#define lsZ_nextb(s) CAST(void, ( (s)->pos < (s)->bl - 1 ? \
	(s)->peek = (s)->buf[++((s)->pos)] \
	: lsZ_fills(s)))
//Peek and next
#define lsZ_readb(s, c) ((c) = lsZ_peekb(s), lsZ_nextb(s))
//Close a stream (do nothing)
#define lsZ_close(s) ((void)0)

/* stream reading */
//Create a new stream from ls_InputReader
LSI_EXTERN void lsZ_creates(ls_Stream* s, const char* chunkname, ls_InputReader r, void* ud);
//Refill the stream, set peek// and return peeked char
LSI_EXTERN void lsZ_fills(ls_Stream* s);

/* Buffer manipulate */
//Buffer block information
#define MEMBUFFER_BLOCK_STRATEGY (LSM_ALLOC_VARIABLE | LSM_ALLOC_MEDIUM | LSM_ALLOC_TEMPORARY)
//Initialize a new buffer
#define lsZ_newbuf(_L, b) ((void)((b)->cap = (b)->sz = 0, (b)->buf = ls_NULL, (b)->L = _L))
#define lsZ_clearbuf(b) ((void)((b)->sz = 0))
#define lsZ_appendbuf(b, c) ( (b)->cap > (b)->sz ? (void)((b)->buf[(b)->sz++] = c) : \
	(void)((b)->buf = lsM_resizeblock((b)->L, MEMBUFFER_BLOCK_STRATEGY, (b)->buf, (b)->cap, (b)->cap = ((b)->cap * 2 + 16)), (b)->buf[(b)->sz++] = c) \
	)
#define lsZ_closebuf(b) ((void) lsM_resizeblock((b)->L, MEMBUFFER_BLOCK_STRATEGY, (b)->buf, (b)->cap, 0), (b)->cap = (b)->sz = 0, (b)->buf = ls_NULL)
#define lsZ_bufsize(b) ((b)->sz)
#endif
