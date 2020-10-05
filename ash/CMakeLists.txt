
find_library(LIBEDIT edit)

set(
	SRC_CORE
	"core/ash.c" "core/io.c" "core/unit.c"
	"core/path.c" "core/exec.c" "core/var.c"
	"core/module.c" "core/session.c" "core/command.c"
	"core/env.c" "core/signal.c"
)

set(
	SRC_FS
	"fs/script.c"
)

set(
	SRC_FFI
	"ffi/ffi.c"
)

set(
	SRC_UTIL
	"util/map.c" "util/hash.c" "util/vec.c"
	"util/strbuf.c" "util/queue.c" "util/rc.c"
)

set(
	SRC_TYPE
	"type/obj.c" "type/int.c" "type/bool.c"
	"type/str.c" "type/range.c" "type/func.c"
	"type/tuple.c" "type/array.c" "type/map.c"
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

set(
	SRC_COMMAND
	"exec.c" "echo.c" "cd.c"
	"sleep.c" "unset.c" "source.c"
	"read.c" "exit.c" "export.c"
	"typeof.c" "defined.c" "rand.c"
	"help.c" "history.c" "alias.c"
	"list.c"
)

add_executable("ash"
	   mem.c ops.c
	   iter.c
	   ${SRC_LANG} ${SRC_TYPE} ${SRC_TERM} ${SRC_FFI}
	   ${SRC_FS} ${SRC_UTIL} ${SRC_CORE} ${SRC_COMMAND})

target_compile_options("ash" PRIVATE "-Wreturn-type" "-g")

target_link_libraries("ash" ${LIBEDIT})
