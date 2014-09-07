submodules
-----
lua?_xxxx

C: gc

I: internal common

M: memory

S: string


differences with lua
-----
Common
All types have name start with 'ls_', even internal types.
No `lock' and `unlock'.

Memory
Use a usage int to indicate allocation strategy and memory type (memory allocate type).
Use a ArrayInfo structure to store array allocating information.
`memused' is the total memory usage but not that for data storage only.

GC
`luaC_newobj' doesn't have a offset field (which in lua is only used in LX).

todos
-----
Do gc in memory allocation functions.
Finish `ls_close'.
