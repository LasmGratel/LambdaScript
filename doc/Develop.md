submodules
-----
ls?_xxxx
* C: gc
* I: internal common
* M: memory
* S: string
* Z: stream


differences with lua
-----
Common
* All types have name start with 'ls_', even internal types.
* No `lock' and `unlock'.

Memory
* Use a usage int to indicate allocation strategy and memory type (memory allocate type).
* Use a ArrayInfo structure to store array allocating information.
* `memused' is the total memory usage but not that for data storage only.

GC
* `luaC_newobj' doesn't have a offset field (which in lua is only used in LX).

Stream
* Reader function doesn't receive ls_State pointer (which is in most cases useless).
* Data is used directly within memory block returned by reader function instead of copied into strem.

todos
-----
* Use a simple test system for test.c.
* Do gc in memory allocation functions.
* Finish `ls_close'.
* THROW, ls_throw.
* String pool for short string reuse.
* Add filter to stream.
* Specify usage in internal allocation.
