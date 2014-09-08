#ifndef LS_STREAM_H
#define LS_STREAM_H

//use int to allow EOF(-1)
#define ls_Char int

#define ls_EOS (CAST(ls_Char, -1))

typedef struct ls_Stream
{
	ls_InputReader r;
	void* ud;

	const char* buf;
	ls_MemSize bl;

	ls_MemSize pos;
	ls_Char peek; //value at pos
} ls_Stream;

/* stream bit manipulate */

//Peek a char and don't change pos
#define lsZ_peekb(s) ((s)->peek) 
//Step pos
#define lsZ_nextb(s) CAST(void, ( (s)->pos < (s)->bl - 1 ? \
	(s)->peek = (s)->buf[++((s)->pos)] \
	: lsZ_fills(s)))
//Peek and next
#define lsZ_readb(s, c) (*(c) = lsZ_peekb(s), lsZ_nextb(s))
//Close a stream (do nothing)
#define lsZ_close(s) ((void)0)

/* stream reading */
//Create a new stream from ls_InputReader
LSI_EXTERN void lsZ_createstream(ls_Stream* s, ls_InputReader r, void* ud);
//Refill the stream, set peek// and return peeked char
LSI_EXTERN void lsZ_fills(ls_Stream* s);


#endif
