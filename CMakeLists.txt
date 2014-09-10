cmake_minimum_required (VERSION 3.0)

project (LambdaScript)

# TODO: Add config.h.in

# Library files

set(LIB_SOURCES
  ctype.c
  gc.c
  lex.c
  mem.c
  state.c
  stream.c
  string.c
)

set(LIB_HEADERS
  common.h
  config.h
  ctype.h
  gc.h
  lex.h
  ls.h
  mem.h
  object.h
  state.h
  stream.h
  string.h
)


# Now ls_throw is in test.c, so we can't use SHARED here.
# Change this when ls_throw is done!

add_library(lambdascript ${LIB_SOURCES} ${LIB_HEADERS})


# A simple test project.

add_executable(lstest test.c)
target_link_libraries(lstest lambdascript)
