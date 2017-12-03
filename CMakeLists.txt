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
try_flag(CMAKE_C_FLAGS "-Wno-unused-command-line-argument")
try_flag(CMAKE_C_FLAGS "-Wno-keyword-macro")
try_flag(CMAKE_C_FLAGS "-Wno-reserved-id-macro")
try_flag(CMAKE_C_FLAGS "-Wno-vla")
try_flag(CMAKE_C_FLAGS "-Wno-disabled-macro-expansion")
try_flag(CMAKE_C_FLAGS "-Wno-missing-braces")
try_flag(CMAKE_C_FLAGS "-Wno-missing-field-initializers")

set(SOURCE_FILES
        src/system.h

        src/lib/macro.h
        src/lib/vector.c
        src/lib/buffer.c
        src/lib/string.c

        src/ctx.c
        src/main.c

        src/phases/parse.c src/phases/parse.inc.h
        src/phases/flatten.c
        src/phases/print.c
        src/phases/eval.c
        src/phases/compile/compile.c

        src/intrinsics/debug/puti.c
        src/intrinsics/debug/puts.c

        src/intrinsics/_.h
        src/intrinsics/cond.c
        src/intrinsics/define.c
        src/intrinsics/do.c
        src/intrinsics/extern.c
        src/intrinsics/func.c

        src/intrinsics/minus.c
        src/intrinsics/plus.c

        src/intrinsics/types/func.c
        )
add_executable(silis ${SOURCE_FILES})
