cmake_minimum_required(VERSION 2.8)
project(silis C)

set(CMAKE_C_STANDARD 11)

include(CheckCCompilerFlag)
function(try_flag LIST FLAG)
    string(REGEX REPLACE "[/=-]" "_" TEST ${FLAG})
    set(CMAKE_REQUIRED_FLAGS "-Werror")
    check_c_compiler_flag("${FLAG}" ${TEST})
    set(CMAKE_REQUIRED_FLAGS "")
    if (${TEST})
        set(${LIST} "${${LIST}} ${FLAG}" PARENT_SCOPE)
    endif ()
endfunction()

try_flag(CMAKE_C_FLAGS "-Werror")
try_flag(CMAKE_C_FLAGS "-pedantic")
try_flag(CMAKE_C_FLAGS "-Wall")
try_flag(CMAKE_C_FLAGS "-Wextra")
try_flag(CMAKE_C_FLAGS "-Weverything")

# keep
try_flag(CMAKE_C_FLAGS "-Wno-type-limits")
try_flag(CMAKE_C_FLAGS "-Wno-unused-command-line-argument")
try_flag(CMAKE_C_FLAGS "-Wno-keyword-macro")
try_flag(CMAKE_C_FLAGS "-Wno-reserved-id-macro")
try_flag(CMAKE_C_FLAGS "-Wno-vla")
try_flag(CMAKE_C_FLAGS "-Wno-disabled-macro-expansion")
try_flag(CMAKE_C_FLAGS "-Wno-missing-braces")
try_flag(CMAKE_C_FLAGS "-Wno-missing-field-initializers")

include_directories(src)

set(SOURCE_FILES
        src/system.h

        src/lib/encoding/ascii.c
        src/lib/buffer.c
        src/lib/macro.h
        src/lib/slice.h
        src/lib/stdio.c
        src/lib/string.c
        src/lib/trie.c
        src/lib/vector.c

        src/compiler/ctx.c
        src/compiler/main.c

        src/compiler/phases/parse.c src/compiler/phases/parse.inc.h
        src/compiler/phases/flatten.c
        src/compiler/phases/print.c
        src/compiler/phases/eval.c
        src/compiler/phases/compile/compile.c

        src/compiler/intrinsics/_.h
        src/compiler/intrinsics/debug/puti.c
        src/compiler/intrinsics/debug/puts.c
        src/compiler/intrinsics/cond.c
        src/compiler/intrinsics/define.c
        src/compiler/intrinsics/do.c
        src/compiler/intrinsics/extern.c
        src/compiler/intrinsics/func.c
        src/compiler/intrinsics/minus.c
        src/compiler/intrinsics/plus.c

        src/compiler/intrinsics/types/func.c
        )
add_executable(silis ${SOURCE_FILES})
