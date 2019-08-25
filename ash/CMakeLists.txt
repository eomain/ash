
find_library(LIBEDIT edit)

set(
	SRC_TYPE
	"type/obj.c" "type/int.c" "type/bool.c"
	"type/str.c" "type/range.c" "type/func.c"
	"type/tuple.c"
)

set(
	SRC_LANG
	"lang/lang.c" "lang/ast.c" "lang/parser.c"
	"lang/runtime.c" "lang/lex.c" "lang/main.c"
)

set(
	SRC_TERM
	"term/term.c"
)

add_executable("ash" ash.c io.c env.c var.c builtin.c
	   mem.c script.c exec.c echo.c
	   cd.c sleep.c unset.c ops.c source.c
	   read.c exit.c export.c typeof.c
	   iter.c defined.c command.c rand.c util.c
	   signal.c help.c session.c unit.c history.c
	   ${SRC_LANG} ${SRC_TYPE} ${SRC_TERM})

target_compile_options("ash" PRIVATE "-Wreturn-type" "-g")

target_link_libraries("ash" ${LIBEDIT})
