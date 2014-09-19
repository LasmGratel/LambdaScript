cmake_minimum_required (VERSION 2.8)

project (LambdaScript)

# TODO: Add config.h.in

# Library files

set(LIB_SOURCES
  code.c
  ctype.c
  func.c
  gc.c
  lex.c
  mem.c
  parser.c
  review.c
  state.c
  stream.c
  string.c
  table.c
)

set(LIB_HEADERS
  code.h
  common.h
  config.h
  ctype.h
  func.h
  gc.h
  lex.h
  ls.h
  mem.h
  object.h
  parser.h
  parser_ctrl.h
  parser_expr.h
  parser_loc.h
  parser_util.h
  review.h
  state.h
  stream.h
  string.h
  table.h
)


# Now ls_throw is in test.c, so we can't use SHARED here.
# Change this when ls_throw is done!

add_library(lambdascript ${LIB_SOURCES} ${LIB_HEADERS})


# A simple test project.

add_executable(lstest test.c)
target_link_libraries(lstest lambdascript)
